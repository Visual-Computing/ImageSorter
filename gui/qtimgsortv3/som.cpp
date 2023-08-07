#include "som.h"
#include "imageerror.h"
#include <assert.h>
#include <cstdlib>
#include <ctime>
#include <limits>
#include "leak.h"
template <typename T,unsigned dim> somNode<T,dim>::somNode(const somNode& rOther)
{
	m_uX=rOther.m_uX;
	m_uY=rOther.m_uY;
	m_Data=rOther.m_Data;
}

template <typename T,unsigned dim> somNode<T,dim>& somNode<T,dim>::operator=(const somNode& rOther)
{
	if(&rOther==this)
		return *this;

	m_uX=rOther.m_uX;
	m_uY=rOther.m_uY;
	m_Data=rOther.m_Data;

	return *this;
}

template <typename T,typename TMutex,unsigned dim> som<T,TMutex,dim>::som() :
m_bInitialized(false),
m_uXDim(0),
m_uYDim(0),
m_uSize(0),
m_uNumNodes(0),
m_pTaken(NULL),
m_pNodes(NULL),
m_pNet(NULL),
m_pNetUpdate(NULL),
m_pNetUpdateHor(NULL),
m_pWeight(NULL),
m_pSum(NULL),
m_pWeightHor(NULL),
m_dDimFactor(1.3),
m_dAspectRatio(1.0),
m_uIters(25),  // 25
m_pCallBack(NULL),
m_pUserData(NULL),
m_uInterval(0),
m_pPauseCallBack(NULL),
m_pPauseUserData(NULL),
m_pMutex(NULL),
m_pAbort(NULL),
m_pPause(NULL)
{
	// test if T is a numeric type
	if(!numeric_limits<T>::is_specialized)
		throw fqImageError(imageError::badTemplateType);
}

template <typename T,typename TMutex,unsigned dim> void som<T,TMutex,dim>::registerCallback(somCallBack pCallBack,void* pUserData,unsigned interval)
{
	m_pCallBack=pCallBack;
	m_pUserData=pUserData;
	m_uInterval=interval?interval:1;
}
template <typename T,typename TMutex,unsigned dim> void som<T,TMutex,dim>::registerPauseCallback(somPauseCallBack pCallBack,void* pUserData)
{
	m_pPauseCallBack=pCallBack;
	m_pPauseUserData=pUserData;
}

template <typename T,typename TMutex,unsigned dim> void som<T,TMutex,dim>::init(array<somNode<T,dim> >& rNodes,TMutex* pMutex,bool *pAbort,bool *pPause,bool bRandom)
{
	if(m_bInitialized)
		_destroy();

	m_uNumNodes=rNodes.size();
	if(!m_uNumNodes)
		throw fqImageError(imageError::badParameter);

	unsigned mapXDim,mapYDim;

	if(m_dAspectRatio==1.0)
		mapXDim=mapYDim=(unsigned)ceil(sqrt(m_uNumNodes*m_dDimFactor));
	else{
		// we have:
		// mapXDim*mapYDim==m_uNumNodes*m_dDimFactor
		// mapXDim/mapYDim==m_dAspectRatio 
		// => mapYDim=mapXDim/m_dAspectRatio
		// => mapXDim=sqrt(m_uNumNodes*m_dDimFactor*m_dAspectRatio);
		mapXDim=(unsigned)ceil(sqrt(m_uNumNodes*m_dDimFactor*m_dAspectRatio));
		// => mapXDim=mapYDim*m_dAspectRatio
		// => mapYDim=sqrt(m_uNumNodes*m_dDimFactor/m_dAspectRatio);
		mapYDim=(unsigned)ceil(sqrt(m_uNumNodes*m_dDimFactor/m_dAspectRatio));
	}

	m_uXDim=mapXDim;
	m_uYDim=mapYDim;
	m_uSize=mapXDim*mapYDim;

	m_pTaken=NULL;
	m_pTaken=new bool[m_uSize];
	if(!m_pTaken){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	/*
	m_pNodes=NULL;
	m_pNodes=new somNode<T,dim>[m_uSize];
	if(!m_pNodes){
	_destroy();
	throw fqImageError(imageError::badAlloc);
	}
	*/
	// new nodes passed as array<> from outside
	m_pNodes=rNodes.pVals();

	m_pNet=NULL;
	m_pNet=new fixedArray<T,dim>[m_uSize];
	if(!m_pNet){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	m_pNetUpdate=NULL;
	m_pNetUpdate=new fixedArray<T,dim>*[m_uSize];
	if(!m_pNetUpdate){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}


	m_pNetUpdateHor=NULL;
	m_pNetUpdateHor=new fixedArray<T,dim>[m_uSize];
	if(!m_pNetUpdateHor){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	m_pWeight=NULL;
	m_pWeight=new unsigned[m_uSize];
	if(!m_pWeight){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	m_pWeightHor=NULL;
	m_pWeightHor=new unsigned[m_uSize];
	if(!m_pWeightHor){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	m_pSum=NULL;
	m_pSum=new T[dim];
	if(!m_pSum){
		_destroy();
		throw fqImageError(imageError::badAlloc);
	}

	// TODO: take care if the divisions are what we want
	// maybe x/(T)2 is better...
	m_r0=(T)(m_uXDim/2.);

	m_bTakenState=true;

	const unsigned s=size();
	unsigned i,d;
	if(bRandom){
		// set random values to the net
		// note this is from kai's java code
		// note this is propably something which is bound to the color layout feature
		// so, if we use another, this might be wrong
		clock_t clk=::clock();
		//::srand((unsigned int)clk);
		::srand((unsigned int)17);
		T tRndMax=(T)(RAND_MAX);
		for (int y=0; y<m_uYDim; y++) {
			for (int x=0; x<m_uXDim; x++) {
				int i = x+y*m_uXDim;
				// d==0
				m_pNet[i][2]=128;
				// d==1
				m_pNet[i][1]= 0.5 - ((T)(rand())/tRndMax); 
				// d==2
				m_pNet[i][0]= 0.5 - ((T)(rand())/tRndMax); 
				for(d=3;d<dim;++d)
					m_pNet[i][d]=(T)(0);

				// init the pTaken array to *not* taken
				m_pTaken[i]=!m_bTakenState;
			}
		}
	}
	else{
		// init to zero
		for(i=0;i<s;++i){
			// d==0
			m_pNet[i][0]=(T)0.0;
			// d==1
			m_pNet[i][1]=(T)0.0;
			// d==2
			m_pNet[i][2]=(T)0.0;
			for(d=3;d<dim;++d)
				m_pNet[i][d]=(T)0.0;
			// init the pTaken array to *not* taken
			m_pTaken[i]=!m_bTakenState;
		}
	}

	//if(pMutex&&pAbort){
	//	m_pMutex=pMutex;
	//	m_pAbort=pAbort;
	//}
	//else{
	//	m_pMutex=NULL;
	//	m_pAbort=NULL;
	//}
	m_pMutex=pMutex;
	m_pAbort=pAbort;
	m_pPause=pPause;
	m_bInitialized=true;
}



template <typename T,typename TMutex,unsigned dim> void som<T,TMutex,dim>::_destroy()
{
	if(!m_bInitialized){
		assert(m_pTaken==NULL);
		assert(m_pNodes==NULL);
		assert(m_pNet==NULL);
		return;
	}

	if(m_pTaken){
		delete[] m_pTaken;
		m_pTaken=NULL;
	}
	if(m_pNodes){
		// note nodes are passed from outside
		m_pNodes=NULL;
	}
	if(m_pNet){
		delete[] m_pNet;
		m_pNet=NULL;
	}
	if(m_pNetUpdate){
		delete[] m_pNetUpdate;
		m_pNetUpdate=NULL;
	}

	if(m_pNetUpdateHor){
		delete[] m_pNetUpdateHor;
		m_pNetUpdateHor=NULL;
	}
	if(m_pWeight){
		delete[] m_pWeight;
		m_pWeight=NULL;
	}
	if(m_pWeightHor){
		delete[] m_pWeightHor;
		m_pWeightHor=NULL;
	}
	if(m_pSum){
		delete[] m_pSum;
		m_pSum=NULL;
	}

	m_uXDim=m_uYDim=m_uSize=m_uNumNodes=0;

	m_pMutex=NULL;
	m_pAbort=NULL;
	m_pPause=NULL;

	m_bInitialized=false;
}

//#define USE_OLD_SOM

#if !defined(USE_OLD_SOM)

template <typename T,typename TMutex,unsigned dim> bool som<T,TMutex,dim>::train()
{
	if(!m_bInitialized){
		assert(false);
		// this is not really true, it's an error
		// we'd better throw
		return false;
	}

	unsigned loop,node,d,pos;
	const unsigned iters=m_uIters;
	const unsigned nNodes=numNodes();
	T err,bestError,tTemp;
	int iUpdateRadius,iSearchRadius, ix,iy;
	unsigned changed,bestX,bestY;

	const bool bCheckAbort=m_pMutex&&m_pAbort;
	const bool bCheckPause=m_pMutex&&m_pPause&&m_pPauseCallBack;
	const bool bCheckPauseOrAbort=bCheckAbort||bCheckPause;
	int dim_t[] = {3,3,3,6,6,6,9,9,12,12,15,15,18,18,21,21,24,24,24,24,24,24,24,24,24};

	// this loop will not be interrupted it will loop 35 times
	for(loop=0;loop<iters;++loop){
		int dimT = dim_t[loop];

		// clear all taken positions and initialize update buffers
		for(int i=0;i<m_uYDim*m_uXDim;++i){
			m_pTaken[i]= !m_bTakenState;
			m_pNetUpdate[i] = &m_pNet[i]; // initialize the new som with the old values
			m_pWeight[i] = 1;             // give them a weight of 1
			m_pWeightHor[i] = 0;		  // horizontal filtered weights	

			for (int d=0; d<dimT; d++)
				m_pNetUpdateHor[i][d] = 0; // horizontal filtered feature vectors of the new som
		}

		
		// neighbourhood radius
		iUpdateRadius=(int)(m_r0  * (1-loop/23.)) - 3;  // /23 do a linear decrease of the radius 
		if (iUpdateRadius < 1)						  // do not allow smaller radii than 1	
			iUpdateRadius = 1;

		for(node=0;node<nNodes;++node){
			// check if to abort
			// note the prior version did the abort check 
			// after the node loop. although this is more correct, since
			// we have a complete new iteration of the sort process after abort
			// in that case, it is bad on big soms, since it takes considerable time
			// for one iteration.

			// we now check for abort on each node
			// if we abort here, we'll have a sort result which isn't that good
			bool bAbort=false;
			bool bPause=false;
			if(bCheckPauseOrAbort){
				m_pMutex->lock();
				if(bCheckAbort)
					bAbort=*m_pAbort;
				if(bCheckPause)
					bPause=*m_pPause;
				m_pMutex->unlock();
			}
			if(bAbort)
				return false;

			// check if to pause
			// note m_pCauseCallBack() either returns immediately
			// or pauses the thread
			// thus, m_pPauseCallBack() has to run in this thread...
			if(bPause&&m_pPauseCallBack)
				m_pPauseCallBack(m_pPauseUserData);

			////////////////////////////////////////////////////////
			// Matching
			//
			
			bestError=numeric_limits<T>::max();

			// find best position for each node
			somNode<T,dim> &actNode=m_pNodes[node];
			unsigned i;

			// try to initialize bestError with the previous best position
			int bestXold = m_pNodes[node].m_uX;
			int bestYold = m_pNodes[node].m_uY;

			if (loop==0)
				bestXold = bestYold = 0;

			unsigned bestI = bestYold*m_uXDim + bestXold;

			if (m_pTaken[bestI]!=m_bTakenState) { 
				bestError = (T)0;
				fixedArray<T,dim> &actNet=m_pNet[bestI];
				for (int d=0; d<dimT; d++){
					tTemp=actNode[d]-actNet[d];
					bestError += tTemp*tTemp;
				}
			}

			
			// find best position for every node
			for(i=0;i<m_uYDim*m_uXDim;++i){
				// check if position is already taken
				if(m_pTaken[i]==m_bTakenState)
					continue;
				// calculate distance between current node vector and current net vector
				err=(T)0;
				fixedArray<T,dim> &actNet=m_pNet[i];
				for(d=0;d<dimT;++d){
					T no = actNode[d];
					T ne = actNet[d];
					tTemp=no-ne;
					err+=tTemp*tTemp;
					if(err>bestError)
						break;
				}
				if(err<bestError){
					bestError=err;
					bestI=i;
				}
			}
			

			m_pTaken[bestI]=m_bTakenState;
			m_pNetUpdate[bestI] = &m_pNodes[node].m_Data;  // set the reference of this feature vector for the new som 
			m_pWeight[bestI] = 4;	                       // giving it a weight of 4

			// remember the best position for this node
			m_pNodes[node].m_uX=bestX=bestI%m_uXDim;
			m_pNodes[node].m_uY=bestY=bestI/m_uXDim;
		}

		// SOM Update ///////////////////////////////////
		//
		// perform a box filtering of the new som 
		// filter is inplemented separable with incremental kernel calculations

		int wk = 0;

		// Horizontal filtering.
		for (int indexDest = 0, y = 0; y < m_uYDim ; y++) {

			int yOffset = y*m_uXDim;

			// do first line block (x = 0)
			for (int ix = -iUpdateRadius; ix <= iUpdateRadius; ix++) {

				int indexSrc = yOffset + ((ix + m_uXDim)%m_uXDim);

				int w =  m_pWeight[indexSrc];
				wk += w;
				m_pWeightHor[indexDest] += w;

				for (int d=0; d<dimT; d++) {
					T value = (*m_pNetUpdate[indexSrc])[d]*w;
					m_pNetUpdateHor[indexDest][d] += value;	
				}
			}
			indexDest++;

			for (int x = 1; x < m_uXDim; x++, indexDest++){

				// copy sum of vectors and weights from previous line block 
				for (int d=0; d<dimT; d++)
					m_pNetUpdateHor[indexDest][d] = m_pNetUpdateHor[indexDest-1][d];
				m_pWeightHor[indexDest] = m_pWeightHor[indexDest-1];

				// remove left position
				int indexSrc = yOffset + (x -iUpdateRadius-1 + m_uXDim)%m_uXDim;
				int w =  m_pWeight[indexSrc];
				m_pWeightHor[indexDest] -= w;
				for (int d=0; d<dimT; d++)
					m_pNetUpdateHor[indexDest][d] -= w*(*m_pNetUpdate[indexSrc])[d];

				// add right position
				indexSrc = yOffset + (x + iUpdateRadius)%m_uXDim;
				w =  m_pWeight[indexSrc];
				m_pWeightHor[indexDest] += w;
				for (int d=0; d<dimT; d++)
					m_pNetUpdateHor[indexDest][d] += w*(*m_pNetUpdate[indexSrc])[d];	
			}
		}

		// Vertical filtering
		for (int x = 0; x < m_uXDim; x++){
			int indexDest = x;

			for (int d=0; d<dimT; d++)
				m_pSum[d] = 0;

			int sumWeight = 0;

			// upper coloumn block of pixels (y = 0)
			for (int iy = -iUpdateRadius; iy <= iUpdateRadius; iy++) {
				int indexSrc = ((iy + m_uYDim)%m_uYDim)*m_uXDim+x;

				for (int d=0; d<dimT; d++)
					m_pSum[d] += m_pNetUpdateHor[indexSrc][d];
				sumWeight += m_pWeightHor[indexSrc];
			}

			double oneOverSumWeight = 1./ sumWeight;
			for (int d=0; d<dimT; d++)
				m_pNet[indexDest][d] = (m_pSum[d]*oneOverSumWeight); // (int)

			for (int y = 1; y < m_uYDim ; y++) {
				indexDest += m_uXDim;

				// remove upper position
				int indexSrc = ((y - iUpdateRadius - 1 + m_uYDim)%m_uYDim)*m_uXDim+x;
				sumWeight -= m_pWeightHor[indexSrc];
				for (int d=0; d<dimT; d++)
					m_pSum[d] -= m_pNetUpdateHor[indexSrc][d];

				// add lowest position
				indexSrc = ((y + iUpdateRadius)%m_uYDim)*m_uXDim+x;

				sumWeight += m_pWeightHor[indexSrc];
				for (int d=0; d<dimT; d++)
					m_pSum[d] += m_pNetUpdateHor[indexSrc][d];

				oneOverSumWeight = 1./ sumWeight;
				for (int d=0; d<dimT; d++)
					m_pNet[indexDest][d] = m_pSum[d]*oneOverSumWeight; // (int)
			}
		}

		// note we may call a callback here, i.e to repaint...
		if(m_pCallBack&&!(loop%m_uInterval)){
			m_pCallBack(m_pUserData);
		}

		//// check if to abort
		//bool bAbort=false;
		//if(bCheckAbort){
		//	m_pMutex->lock();
		//	bAbort=*m_pAbort;
		//	m_pMutex->unlock();
		//}
		//if(bAbort){
		//	return false;
		//}
	}

	return true;
}
#endif

#if defined(USE_OLD_SOM)
template <typename T,typename TMutex,unsigned dim> bool som<T,TMutex,dim>::train()
{
	if(!m_bInitialized){
		assert(false);
		// this is not really true, it's an error
		// we'd better throw
		return false;
	}

	unsigned loop,node,d,pos;
	const unsigned iters=m_uIters;
	const unsigned nNodes=numNodes();
	T err,bestError,tTemp;
	int iUpdateRadius,iSearchRadius, ix,iy;
	unsigned changed,bestX,bestY;

	const bool bCheckAbort=m_pMutex&&m_pAbort;
	const bool bCheckPause=m_pMutex&&m_pPause&&m_pPauseCallBack;
	const bool bCheckPauseOrAbort=bCheckAbort||bCheckPause;
	bool found;

	// this loop will not be interrupted it will loop 35 times
	for(loop=0;loop<iters;++loop){

		// clear all taken positions and initialize update buffers
		for(int i=0;i<m_uYDim*m_uXDim;++i){
			m_pTaken[i]= !m_bTakenState;
			m_pNetUpdate[i] = &m_pNet[i]; // initialize the new som with the old values
			m_pWeight[i] = 1;             // give them a weight of 1
			m_pWeightHor[i] = 0;		  // horizontal filtered weights	

			for (int d=0; d<dim; d++)
				m_pNetUpdateHor[i][d] = 0; // horizontal filtered feature vectors of the new som
		}

		
		// neighbourhood radius
		iUpdateRadius=(int)(m_r0  * (1-loop/18.)) - 3;  // /18 do a linear decrease of the radius 
		if (iUpdateRadius < 1)						  // do not allow smaller radii than 1	
			iUpdateRadius = 1;

		iSearchRadius = (int) (m_r0/5.+1);	// use a reduced search width at the end of sorting
		if (iSearchRadius < 2)
			iSearchRadius = 2;

		//long vTest=0;

		for(node=0;node<nNodes;++node){
			// check if to abort
			// note the prior version did the abort check 
			// after the node loop. although this is more correct, since
			// we have a complete new iteration of the sort process after abort
			// in that case, it is bad on big soms, since it takes considerable time
			// for one iteration.

			// we now check for abort on each node
			// if we abort here, we'll have a sort result which isn't that good
			bool bAbort=false;
			bool bPause=false;
			if(bCheckPauseOrAbort){
				m_pMutex->lock();
				if(bCheckAbort)
					bAbort=*m_pAbort;
				if(bCheckPause)
					bPause=*m_pPause;
				m_pMutex->unlock();
			}
			if(bAbort)
				return false;

			// check if to pause
			// note m_pCauseCallBack() either returns immediately
			// or pauses the thread
			// thus, m_pPauseCallBack() has to run in this thread...
			if(bPause&&m_pPauseCallBack)
				m_pPauseCallBack(m_pPauseUserData);

			////////////////////////////////////////////////////////
			// Matching
			//
			found = false;

			bestError=numeric_limits<T>::max();

			// find best position for each node
			somNode<T,dim> &actNode=m_pNodes[node];
			unsigned i;

			// try to initialize bestError with the previous best position
			int bestXold = m_pNodes[node].m_uX;
			int bestYold = m_pNodes[node].m_uY;

			if (loop==0)
				bestXold = bestYold = 0;

			unsigned bestI = bestYold*m_uXDim + bestXold;

			if (m_pTaken[bestI]!=m_bTakenState) { 
				bestError = (T)0;
				fixedArray<T,dim> &actNet=m_pNet[bestI];
				for (int d=0; d<dim; d++){
					tTemp=actNode[d]-actNet[d];
					bestError += tTemp*tTemp;
				}
				bestError = (int) bestError;
				found = true;
			}


			if (loop < 15) { // full search for the first 15 loops
				// find best position for every node
				for(i=0;i<m_uYDim*m_uXDim;++i){
					// check if position is already taken
					if(m_pTaken[i]==m_bTakenState)
						continue;
					// calculate distance between current node vector and current net vector
					err=(T)0;
					fixedArray<T,dim> &actNet=m_pNet[i];
					for(d=0;d<dim;++d){
						T no = actNode[d];
						T ne = actNet[d];
						tTemp=no-ne;
						err+=tTemp*tTemp;
						if(err>bestError)
							break;
					}
					if(err<bestError){
						bestError=(int)err;
						bestI=i;
					}
				}
			}
			else {
				for (int y=bestYold-iSearchRadius; y<=bestYold+iSearchRadius; y++) {
					for (int x=bestXold-iSearchRadius; x<=bestXold+iSearchRadius; x++) {
						int y_ = (y + m_uYDim) % m_uYDim;
						int x_ = (x + m_uXDim) % m_uXDim;

						int i = y_*m_uXDim+x_;
						if(m_pTaken[i]==m_bTakenState)
							continue; 
						fixedArray<T,dim> &actNet=m_pNet[i];
						double err = 0;
						for (int d=0; d<dim; d++){
							tTemp=actNode[d]-actNet[d];
							err+=tTemp*tTemp;
							if(err>bestError)
								break;
						}
						if (err < bestError) {
							bestError = (int)err;
							bestI = i;
							found = true;
						}	
					}
				}

				// if nothing was found do a full search
				if (!found) {
					// find best position for every node
					for(i=0;i<m_uYDim*m_uXDim;++i){
						// check if position is already taken
						if(m_pTaken[i]==m_bTakenState)
							continue;
						// calculate distance between current node vector and current net vector
						err=(T)0;
						fixedArray<T,dim> &actNet=m_pNet[i];
						for(d=0;d<dim;++d){
							tTemp=actNode[d]-actNet[d];
							err+=tTemp*tTemp;
							if(err>bestError)
								break;
						}
						if(err<bestError){
							bestError=(int)err;
							bestI=i;
						}
					}
				}
			}

			m_pTaken[bestI]=m_bTakenState;
			m_pNetUpdate[bestI] = &m_pNodes[node].m_Data;  // set the reference of this feature vector for the new som 
			m_pWeight[bestI] = 4;	                       // giving it a weight of 4

			// remember the best position for this node
			m_pNodes[node].m_uX=bestX=bestI%m_uXDim;
			m_pNodes[node].m_uY=bestY=bestI/m_uXDim;
		}

		// SOM Update ///////////////////////////////////
		//
		// perform a box filtering of the new som 
		// filter is inplemented separable with incremental kernel calculations

		int wk = 0;

		// Horizontal filtering.
		for (int indexDest = 0, y = 0; y < m_uYDim ; y++) {

			int yOffset = y*m_uXDim;

			// do first line block (x = 0)
			for (int ix = -iUpdateRadius; ix <= iUpdateRadius; ix++) {

				int indexSrc = yOffset + ((ix + m_uXDim)%m_uXDim);

				int w =  m_pWeight[indexSrc];
				wk += w;
				m_pWeightHor[indexDest] += w;

				for (int d=0; d<dim; d++) {
					T value = (*m_pNetUpdate[indexSrc])[d]*w;
					m_pNetUpdateHor[indexDest][d] += value;	
				}
			}
			indexDest++;

			for (int x = 1; x < m_uXDim; x++, indexDest++){

				// copy sum of vectors and weights from previous line block 
				for (int d=0; d<dim; d++)
					m_pNetUpdateHor[indexDest][d] = m_pNetUpdateHor[indexDest-1][d];
				m_pWeightHor[indexDest] = m_pWeightHor[indexDest-1];

				// remove left position
				int indexSrc = yOffset + (x -iUpdateRadius-1 + m_uXDim)%m_uXDim;
				int w =  m_pWeight[indexSrc];
				m_pWeightHor[indexDest] -= w;
				for (int d=0; d<dim; d++)
					m_pNetUpdateHor[indexDest][d] -= w*(*m_pNetUpdate[indexSrc])[d];

				// add right position
				indexSrc = yOffset + (x + iUpdateRadius)%m_uXDim;
				w =  m_pWeight[indexSrc];
				m_pWeightHor[indexDest] += w;
				for (int d=0; d<dim; d++)
					m_pNetUpdateHor[indexDest][d] += w*(*m_pNetUpdate[indexSrc])[d];	
			}
		}

		// Vertical filtering
		for (int x = 0; x < m_uXDim; x++){
			int indexDest = x;

			for (int d=0; d<dim; d++)
				m_pSum[d] = 0;

			int sumWeight = 0;

			// upper coloumn block of pixels (y = 0)
			for (int iy = -iUpdateRadius; iy <= iUpdateRadius; iy++) {
				int indexSrc = ((iy + m_uYDim)%m_uYDim)*m_uXDim+x;

				for (int d=0; d<dim; d++)
					m_pSum[d] += m_pNetUpdateHor[indexSrc][d];
				sumWeight += m_pWeightHor[indexSrc];
			}

			double oneOverSumWeight = 1./ sumWeight;
			for (int d=0; d<dim; d++)
				m_pNet[indexDest][d] = (int)(m_pSum[d]*oneOverSumWeight);

			for (int y = 1; y < m_uYDim ; y++) {
				indexDest += m_uXDim;

				// remove upper position
				int indexSrc = ((y - iUpdateRadius - 1 + m_uYDim)%m_uYDim)*m_uXDim+x;
				sumWeight -= m_pWeightHor[indexSrc];
				for (int d=0; d<dim; d++)
					m_pSum[d] -= m_pNetUpdateHor[indexSrc][d];

				// add lowest position
				indexSrc = ((y + iUpdateRadius)%m_uYDim)*m_uXDim+x;

				sumWeight += m_pWeightHor[indexSrc];
				for (int d=0; d<dim; d++)
					m_pSum[d] += m_pNetUpdateHor[indexSrc][d];

				oneOverSumWeight = 1./ sumWeight;
				for (int d=0; d<dim; d++)
					m_pNet[indexDest][d] = (int)(m_pSum[d]*oneOverSumWeight);
			}
		}

		// note we may call a callback here, i.e to repaint...
		if(m_pCallBack&&!(loop%m_uInterval)){
			m_pCallBack(m_pUserData);
		}

		//// check if to abort
		//bool bAbort=false;
		//if(bCheckAbort){
		//	m_pMutex->lock();
		//	bAbort=*m_pAbort;
		//	m_pMutex->unlock();
		//}
		//if(bAbort){
		//	return false;
		//}
	}

	return true;
}
#endif

// excplicit instantiation
#include "som.expl_templ_inst"




