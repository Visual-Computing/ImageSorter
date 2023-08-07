#include "hierarchicsom.h"
#include <assert.h>
#include <limits>
#include "pxref.h"

using namespace std;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const unsigned hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_Weigth=16;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const T hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_wCL=(T)2;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const unsigned hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_ColorLayoutLen=15;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const T hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_R0=(T)0.5;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const T hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_UpdateDiv=(T)23;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const unsigned hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_MaxIter=26;
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const unsigned hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_Dims[54]={0,3,3,3,9,9,9,9,9,15,15,15,15,15,15,15,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const int hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_dx[9]={ 0, 0,-1, 1, 0,-1, 1,-1, 1};
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> const int hierarchicSom<T,TDATA,TSORT,TMUTEX>::m_dy[9]={ 0,-1, 0, 0, 1, 1, 1,-1,-1};


template <typename T,typename TDATA,typename TSORT,typename TMUTEX> hierarchicSom<T,TDATA,TSORT,TMUTEX>::hierarchicSom(std::vector<TSORT> *pSortData,TMUTEX* pMutex,bool *pAbort/*,bool *pPause*/) : 
																			  m_pMapPlacesX(NULL),
																			  m_pMapPlacesY(NULL),
																			  m_pMapPlaces(NULL),
																			  m_TotalMapPlaces(0),
																			  m_TotalMapPlacesX(0),
																			  m_TotalMapPlacesY(0),
																			  m_pNet(NULL),
																			  m_pNets(NULL),
																			  m_pNodes(NULL),
																			  m_pCount(NULL),
																			  m_pCounts(NULL),
																			  m_pCapacity(NULL),
																			  m_pCapacities(NULL),
																			  m_pCapacityX(NULL),
																			  m_pCapacitiesX(NULL),
																			  m_pCapacityY(NULL),
																			  m_pCapacitiesY(NULL),
																			  m_pWeight(NULL),
																			  m_pNetI(NULL),
																			  m_pNetIs(NULL),
																			  m_uNetISize(0),
																			  m_pWeightI(NULL),
																			  m_pSortData(NULL),
																			  m_pCallBack(NULL),
																			  m_pUserData(NULL),
																			  m_uInterval(0),
																			  //m_pPauseCallBack(NULL),
																			  //m_pPauseUserData(NULL),
																			  m_pMutex(NULL),
																			  m_pAbort(NULL),
																			  //m_pPause(NULL),
																			  m_bInitialized(false)
{
	if(!pSortData)
		return;
	if(!pSortData->size())
		return;
	
	m_pSortData=pSortData;

	m_pMutex=pMutex;
	m_pAbort=pAbort;
	//m_pPause=pPause;

}


template <typename T,typename TDATA,typename TSORT,typename TMUTEX> bool hierarchicSom<T,TDATA,TSORT,TMUTEX>::init(unsigned mapPlacesX,unsigned mapPlacesY)
//#ifndef NO_THROW_BADALLOC
//	throw(std::bad_alloc)
//#endif
{
	if(!m_pSortData)
		return false;

	dealloc();

	if(!mapPlacesX||!mapPlacesY)
		return false;

	// note this is the number of elements, *not* the byte size...
	// note the tricky trick:
	// wether m_pSortData contains TSORT or TSORT*, ref() will return a reference to the TSORT we'd like to have...
	//const unsigned featureSize=ref(m_pSortData->operator[](0)).featureDataSize();
	const unsigned featureSize=const_ref(m_pSortData->operator[](0)).featureDataSize();
	unsigned nXY = mapPlacesX*mapPlacesY;
	if (nXY < 250) 
		m_HierarchySteps = 0;
	else if (nXY < 500)
		m_HierarchySteps = 1;
	else if (nXY < 1250)
		m_HierarchySteps = 2;
	else if (nXY < 3500)
		m_HierarchySteps = 3;
	else if (nXY < 10000)
		m_HierarchySteps = 4;
	else if (nXY < 25000)
		m_HierarchySteps = 5;
	else
		m_HierarchySteps = 6;
	
	unsigned h,i;

	try{
		m_pMapPlacesX = new unsigned[m_HierarchySteps+1];
		m_pMapPlacesY = new unsigned[m_HierarchySteps+1];
		m_pMapPlaces  = new unsigned[m_HierarchySteps+1];
		
		// SOM net
		m_pNet = new T**[m_HierarchySteps+1];
		//if(m_pNet)
		//	for(h=0;h<=m_HierarchySteps;++h)
		//		m_pNet[h]=NULL;

		// count of taken places
		m_pCount = new unsigned*[m_HierarchySteps+1];
		//if(m_pCount)
		//	for(h=0;h<=m_HierarchySteps;++h)
		//		m_pCount[h]=NULL;

		// number of places of the next level
		m_pCapacity = new unsigned*[m_HierarchySteps+1];			
		//if(m_pCapacity)
		//	for(h=0;h<=m_HierarchySteps;++h)
		//		m_pCapacity[h]=NULL;
		m_pCapacityX = new unsigned*[m_HierarchySteps+1];			// number of places of the next level
		//if(m_pCapacityX)
		//	for(h=0;h<=m_HierarchySteps;++h)
		//		m_pCapacityX[h]=NULL;
		m_pCapacityY = new unsigned*[m_HierarchySteps+1];			// number of places of the next level
		//if(m_pCapacityY)
		//	for(h=0;h<=m_HierarchySteps;++h)
		//		m_pCapacityY[h]=NULL;
		
		m_pWeight =  new unsigned[nXY];

		memset(m_pWeight,0,nXY*sizeof(unsigned));

	}
	catch(std::bad_alloc){
		// at least one allocation failed, deallocate others
		dealloc();
		// pass out exception
		throw;
		// never reached
		// return false;
	}

	m_TotalMapPlacesX=m_pMapPlacesX[0] = mapPlacesX;
	m_TotalMapPlacesY=m_pMapPlacesY[0] = mapPlacesY;
	m_TotalMapPlaces=m_pMapPlaces[0] = nXY;

	for(h=1; h<=m_HierarchySteps; h++) {
		m_pMapPlacesX[h] = (m_pMapPlacesX[h-1]+1)/2;
		m_TotalMapPlacesX+=m_pMapPlacesX[h];

		m_pMapPlacesY[h] = (m_pMapPlacesY[h-1]+1)/2;
		m_TotalMapPlacesY+=m_pMapPlacesY[h];

		m_pMapPlaces[h] = m_pMapPlacesX[h]*m_pMapPlacesY[h];
		m_TotalMapPlaces+=m_pMapPlaces[h];
	}

	try{
		// allocate all nodes in one block
		m_pNodes=new T[m_TotalMapPlaces*featureSize];
		memset(m_pNodes,0,m_TotalMapPlaces*featureSize*sizeof(T));

		m_pNets=new T*[m_TotalMapPlaces];

		m_pCounts=new unsigned[m_TotalMapPlaces];
		memset(m_pCounts,0,m_TotalMapPlaces*sizeof(unsigned));

		m_pCapacities=new unsigned[m_TotalMapPlaces];
		memset(m_pCapacities,0,m_TotalMapPlaces*sizeof(unsigned));

		m_pCapacitiesX=new unsigned[m_TotalMapPlacesX];
		memset(m_pCapacitiesX,0,m_TotalMapPlacesX*sizeof(unsigned));

		m_pCapacitiesY=new unsigned[m_TotalMapPlacesY];
		memset(m_pCapacitiesY,0,m_TotalMapPlacesY*sizeof(unsigned));

		// first pointers:
		T* pNode=m_pNodes;
		T** pNets=m_pNets;
		unsigned *pCount=m_pCounts;
		unsigned *pCapacities=m_pCapacities;
		unsigned *pCapacitiesX=m_pCapacitiesX;
		unsigned *pCapacitiesY=m_pCapacitiesY;

		for(h=0; h<=m_HierarchySteps; h++) {
			// SOM net
			m_pNet[h]=pNets;
			pNets+=m_pMapPlaces[h];
			for(i=0;i<m_pMapPlaces[h];++i){
				m_pNet[h][i]=pNode;
				pNode+=featureSize;
			}

			m_pCount[h]=pCount;
			pCount+=m_pMapPlaces[h];
			
			m_pCapacity[h]=pCapacities;
			pCapacities+=m_pMapPlaces[h];

			m_pCapacityX[h] = pCapacitiesX;
			pCapacitiesX+=m_pMapPlacesX[h];

			m_pCapacityY[h] = pCapacitiesY;
			pCapacitiesY+=m_pMapPlacesY[h];

			//for(j=0;j<m_pMapPlaces[h];++j){
			//	// note done in sort() anyway
			//	m_pCount[h][j]=0;
			//	m_pCapacity[h][j]=0;
			//}
			//for(j=0;j<m_pMapPlacesX[h];++j){
			//	m_pCapacityX[h][j]=0;
			//}
			//for(j=0;j<m_pMapPlacesY[h];++j){
			//	m_pCapacityY[h][j]=0;
			//}

		}
	}
	catch(std::bad_alloc){
		// at least one allocation failed, deallocate others
		dealloc();
		// pass out exception
		throw;
		// never reached
		// return false;
	}
	
	// calculate the number of places for all levels
	for(h=0;h<=m_HierarchySteps;++h){	
		for (unsigned y = 0; y < m_pMapPlacesY[0]; ++y) {
			unsigned yh = y>>h;
			for (unsigned x = 0; x < m_pMapPlacesX[0]; ++x) {
				unsigned xh = x>>h;
				unsigned posh = yh*m_pMapPlacesX[h] + xh;
				m_pCapacity[h][posh]++;
			}	
		}		
	}
	
	for (unsigned h = 1; h <= m_HierarchySteps; ++h) {
		for (unsigned y = 0; y < m_pMapPlacesY[h-1]; ++y) 
			m_pCapacityY[h][y>>1]++;
		for (unsigned x = 0; x < m_pMapPlacesX[h-1]; ++x) 
			m_pCapacityX[h][x>>1]++;
	}	
	
	m_InitialRadiusX = m_pMapPlacesX[0]*m_R0; 
	m_InitialRadiusY = m_pMapPlacesY[0]*m_R0;

	m_bInitialized=true;
#if defined(_DEBUG)
	_CrtCheckMemory();
#endif
	return true;
}

template <typename T,typename TDATA,typename TSORT,typename TMUTEX> hierarchicSom<T,TDATA,TSORT,TMUTEX>::~hierarchicSom()
{
	dealloc();
}

template <typename T,typename TDATA,typename TSORT,typename TMUTEX> void hierarchicSom<T,TDATA,TSORT,TMUTEX>::dealloc()
{
	// three dim arrays
	if(m_pNodes){
		delete[] m_pNodes;
		m_pNodes=NULL;
	}

	if(m_pNets){
		delete[] m_pNets;
		m_pNets=NULL;
	}

	if(m_pNet){
		delete[] m_pNet;
		m_pNet=NULL;
	}
	// one dim arrays
	if(m_pMapPlacesX){
		delete[] m_pMapPlacesX;
		m_pMapPlacesX=NULL;
	}
	if(m_pMapPlacesY){
		delete[] m_pMapPlacesY;
		m_pMapPlacesY=NULL;
	}
	if(m_pMapPlaces){
		delete[] m_pMapPlaces;
		m_pMapPlaces=NULL;
	}
	if(m_pWeight){
		delete[] m_pWeight;
		m_pWeight=NULL;
	}
	if(m_pWeightI){
		delete[] m_pWeightI;
		m_pWeightI=NULL;
	}
	// two dim arrays: first dim is (m_HierarchySteps+1)
	if(m_pCounts){
		delete[] m_pCounts;
		m_pCounts=NULL;
	}

	if(m_pCount){
		delete[] m_pCount;
		m_pCount=NULL;
	}
	if(m_pCapacities){
		delete[] m_pCapacities;
		m_pCapacities=NULL;
	}

	if(m_pCapacity){
		delete[] m_pCapacity;
		m_pCapacity=NULL;
	}

	if(m_pCapacitiesX){
		delete[] m_pCapacitiesX;
		m_pCapacitiesX;
	}

	if(m_pCapacityX){
		delete[] m_pCapacityX;
		m_pCapacityX=NULL;
	}

	if(m_pCapacitiesY){
		delete[] m_pCapacitiesY;
		m_pCapacitiesY;
	}

	if(m_pCapacityY){
		delete[] m_pCapacityY;
		m_pCapacityY=NULL;
	}

	if(m_pNetIs){
		delete[] m_pNetIs;
		m_pNetIs=NULL;
	}

	if(m_pNetI){
		delete[] m_pNetI;
		m_pNetI=NULL;
	}

	m_bInitialized=false;

}



template <typename T,typename TDATA,typename TSORT,typename TMUTEX> bool hierarchicSom<T,TDATA,TSORT,TMUTEX>::calculateFeatureIntegralImage() 
{
	const unsigned mapPlacesX=m_pMapPlacesX[0];
	const unsigned mapPlacesY=m_pMapPlacesY[0];
	const unsigned actRadiusX=m_ActRadiusX;
	const unsigned actRadiusY=m_ActRadiusY;
	const unsigned mapPlacesXI = mapPlacesX + 2*actRadiusX + 1;
	const unsigned mapPlacesYI = mapPlacesY + 2*actRadiusY + 1;

	unsigned yI,y,xI,x,posI,pos,d;
	T* sumOfTheCurrentRow=NULL;

	try{
		//sumOfTheCurrentRow = new T[m_ActDim];
		sumOfTheCurrentRow = new T[m_NextDim];
	}
	catch(std::bad_alloc){
		if(sumOfTheCurrentRow)
			delete[] sumOfTheCurrentRow;

		// out of mem, what shall we do?
		dealloc();
		// pass out exception
		throw;
		// never reached
		// return false;
	}

	for(yI=1;yI<mapPlacesYI;++yI) {
		y=(yI-1+mapPlacesY-actRadiusY)%mapPlacesY;

		//memset(sumOfTheCurrentRow,0,sizeof(T)*m_ActDim);
		memset(sumOfTheCurrentRow,0,sizeof(T)*m_NextDim);

		for(xI=1;xI<mapPlacesXI;++xI){
			x=(xI-1+mapPlacesX-actRadiusX)%mapPlacesX;

			posI=yI*mapPlacesXI+xI;
			pos=y*mapPlacesX+x;

			T* a_pos=m_pNet[0][pos];
			T* aI_posI=m_pNetI[posI];
			T* aI_posImRow=m_pNetI[posI-mapPlacesXI];
           
			//for(d=0;d<m_ActDim;++d){
			for(d=0;d<m_NextDim;++d){
				sumOfTheCurrentRow[d]+=a_pos[d];
				aI_posI[d]=aI_posImRow[d]+sumOfTheCurrentRow[d];
			}
		}
	}
//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif
	delete[] sumOfTheCurrentRow;
	return true;
}

template <typename T,typename TDATA,typename TSORT,typename TMUTEX> void hierarchicSom<T,TDATA,TSORT,TMUTEX>::calculateWeightIntegralImage() 
{
	const unsigned mapPlacesX=m_pMapPlacesX[0];
	const unsigned mapPlacesY=m_pMapPlacesY[0];
	const unsigned radiusX=m_ActRadiusX;
	const unsigned radiusY=m_ActRadiusY;
	const unsigned mapPlacesXI = mapPlacesX + 2*radiusX + 1;
	const unsigned mapPlacesYI = mapPlacesY + 2*radiusY + 1;

	unsigned yI,y,xI,x,posI,pos;

    for(yI=1;yI<mapPlacesYI;++yI){

        y=(yI-1+mapPlacesY-radiusY)%mapPlacesY;

        int sumOfTheCurrentRow = 0;

        for (xI=1;xI<mapPlacesXI;++xI){
            x=(xI-1+mapPlacesX-radiusX)%mapPlacesX;

            posI = yI * mapPlacesXI + xI;
            pos  = y  * mapPlacesX  + x;

            sumOfTheCurrentRow += m_pWeight[pos];
            m_pWeightI[posI] = m_pWeightI[posI-mapPlacesXI] + sumOfTheCurrentRow;
        }
    }
//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif
}


template <typename T,typename TDATA,typename TSORT,typename TMUTEX> void hierarchicSom<T,TDATA,TSORT,TMUTEX>::averageNet(unsigned h)
{
	//float[][] pNet = m_pNet[h];
	//float[][] pNet1 = m_pNet[h+1];
	float** pNet = m_pNet[h];
	float** pNet1 = m_pNet[h+1];

	const unsigned mapPlacesX=m_pMapPlacesX[h];
	const unsigned mapPlacesY=m_pMapPlacesY[h];

	for (unsigned y0=0; y0<mapPlacesY; y0+=2) {
		const unsigned y1 = (y0+1 < mapPlacesY) ? y0+1 : y0;
		const unsigned pos_y0=y0*mapPlacesX;
		const unsigned pos_y1=y1*mapPlacesX;

		for (unsigned x0=0; x0<mapPlacesX; x0+=2) {
			const unsigned x1 = (x0+1 < mapPlacesX) ? x0+1 : x0;
			const unsigned pos00 = x0 + pos_y0;
			const unsigned pos01 = x1 + pos_y0;
			const unsigned pos10 = x0 + pos_y1;
			const unsigned pos11 = x1 + pos_y1;

			const unsigned pos2 = (y0>>1)*m_pMapPlacesX[h+1] + (x0>>1);
			//for (unsigned d = 0; d < m_ActDim; ++d) {
			for (unsigned d = 0; d < m_NextDim; ++d) {
				pNet1[pos2][d] = ( pNet[pos00][d] + pNet[pos01][d] + pNet[pos10][d] + pNet[pos11][d] ) * (T)0.25;
			}
		}
	}		
//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif
}

template <typename T,typename TDATA,typename TSORT,typename TMUTEX> bool hierarchicSom<T,TDATA,TSORT,TMUTEX>::sort() 
{
	const bool bCheckAbort=m_pMutex&&m_pAbort;
	//const bool bCheckPause=m_pMutex&&m_pPause&&m_pPauseCallBack;
	//const bool bCheckPauseOrAbort=bCheckAbort||bCheckPause;
	
	for(unsigned t=0;t<m_MaxIter;++t){
		// check if to abort
		bool bAbort=false;
		// bool bPause=false;
		if(bCheckAbort){
			m_pMutex->lock();
			bAbort=*m_pAbort;
			//if(bCheckPause)
			//	bPause=*m_pPause;
			m_pMutex->unlock();
		}
		if(bAbort)
			return false;

		//// check if to pause
		//// note m_pCauseCallBack() either returns immediately
		//// or pauses the thread
		//// thus, m_pPauseCallBack() has to run in this thread...
		//if(bPause&&m_pPauseCallBack)
		//	m_pPauseCallBack(m_pPauseUserData);

		m_ActDim = m_Dims[t];
		m_NextDim = m_Dims[t+1];
		const unsigned endWeigth=__min(m_ActDim,m_ColorLayoutLen);
		
		unsigned i,h;
		// initialize weights
		for (i=0; i < m_pMapPlaces[0]; ++i) 
			m_pWeight[i] = 1;
		
		// reset counts to 0
		for (h=0; h <= m_HierarchySteps; ++h) 
			for (i=0; i < m_pMapPlaces[h]; ++i) 
				m_pCount[h][i] = 0;

		// Matching (find the best free position for every image)
		unsigned n=0;
		//const unsigned sz=m_pSortData->size();
		//for (n=0; n < sz; ++n) {
#ifdef __GNUC__
		typename std::vector<TSORT>::iterator it=m_pSortData->begin();
		typename std::vector<TSORT>::iterator itEnd=m_pSortData->end();
#else
		std::vector<TSORT>::iterator it=m_pSortData->begin();
		std::vector<TSORT>::iterator itEnd=m_pSortData->end();
#endif
		while(it!=itEnd){
			const TDATA* actFeatureData = ((const_ref(*it)).featureData());
			
			T bestDist = std::numeric_limits<T>::max();
			unsigned bestI=n++;
			unsigned bestX;
			unsigned bestY;
			unsigned i;
			if (t > 0) {
				// full search for the highest level
				for (i=0; i<m_pMapPlaces[m_HierarchySteps]; ++i) {
					if (m_pCount[m_HierarchySteps][i]<m_pCapacity[m_HierarchySteps][i]){   // this position is not taken
						T* actNet = m_pNet[m_HierarchySteps][i];
						T dist = (T)0;
						// Note the color layout data is the first data, it is weighted by m_wCL
						// Note this is not really nice, since it ties the hierarchic som to the
						// type of data is sorts...
						// a better way would be to pass in a vector of weighting factors wich is 
						// applied to each node...
						for (unsigned d=0; d<endWeigth; ++d) {
							T ds = m_wCL * (T)(actFeatureData[d]) - actNet[d];
							dist += ds*ds;
						}
						if(dist<bestDist){
							for (unsigned d=m_ColorLayoutLen; d<m_ActDim; ++d) {
								T ds = (T)(actFeatureData[d]) - actNet[d];
								dist += ds*ds;
							}
							if (dist < bestDist) {
								bestDist = dist;
								bestI = i;
							}
						}
					}
				}

//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif
				
				bestX = bestI % m_pMapPlacesX[m_HierarchySteps];
				bestY = bestI / m_pMapPlacesX[m_HierarchySteps];

				// check next lower levels
				for (int h = m_HierarchySteps-1; h >= 0; h--) {
					int mph1X = m_pMapPlacesX[h+1];
					int mph1Y = m_pMapPlacesY[h+1];

					bestDist = std::numeric_limits<T>::max();
					// why? bestI is set below in the if(found) branch
					// and found will be always true in the first iteration...
					// bestI=-1;

					for (unsigned k=0; k < 9; ++k) { // also try surrounding places
						//int bestY_ = (bestY+m_dy[k]+m_pMapPlacesY[h+1])%m_pMapPlacesY[h+1];
						//int bestX_ = (bestX+m_dx[k]+m_pMapPlacesX[h+1])%m_pMapPlacesX[h+1];
						int bestY_ = (bestY+m_dy[k]+mph1Y)%mph1Y;
						int bestX_ = (bestX+m_dx[k]+mph1X)%mph1X;

						assert(bestY_>=0);
						assert(bestX_>=0);

						int yh = 2*bestY_;
						int xh = 2*bestX_;
						// find the the best position of the next level
						for (unsigned y=yh; y<yh+m_pCapacityY[h+1][bestY_]; y++) {
							for (unsigned x=xh; x<xh+m_pCapacityX[h+1][bestX_]; x++) {
								unsigned i = y*m_pMapPlacesX[h]+x; 
								if (m_pCount[h][i]<m_pCapacity[h][i]){ // if not taken
									T* actNet = m_pNet[h][i];
									T dist = (T)0;

									for (unsigned d=0; d<endWeigth; ++d) {
										T ds = m_wCL * (T)(actFeatureData[d]) - actNet[d];
										dist += ds*ds;
									}
									if(dist<bestDist){
										for (unsigned d=m_ColorLayoutLen; d<m_ActDim; ++d) {
											T ds = (T)(actFeatureData[d]) - actNet[d];
											dist += ds*ds;
										}
										if (dist < bestDist) {
											bestDist = dist;
											bestI = i;
										}
									}
								}
							}
						}
					}

//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif

					bestX = bestI % m_pMapPlacesX[h];
					bestY = bestI / m_pMapPlacesX[h];
				}
			}
			else {
				//bestI = n;
				bestX = bestI % m_pMapPlacesX[0];
				bestY = bestI / m_pMapPlacesX[0];
			}

			// we have found a position
			// make sure we are a friend of TSORT
			(ref(*it)).setMapXPos(bestX);
			(ref(*it)).setMapYPos(bestY);

			// copy the weighted feature vector to the SOM (which will be filtered next)
			T* actNet = m_pNet[0][bestI];
			const unsigned nextEnd=__min(m_NextDim,m_ColorLayoutLen);
			for (unsigned d = 0; d < nextEnd; ++d)
				actNet[d] = m_wCL * (T)actFeatureData[d] * m_Weigth;
			for (unsigned d = m_ColorLayoutLen; d < m_NextDim;++d)
				actNet[d] = (T)actFeatureData[d] * m_Weigth;

//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif

			// change the weight for this position
			m_pWeight[bestI] = m_Weigth;	
			
			// increase the capacity counters
			m_pCount[0][bestI]++;

			for (unsigned h = 1; h <= m_HierarchySteps; ++h) {
				bestX >>= 1;
				bestY >>= 1;
				bestI = bestX + bestY*m_pMapPlacesX[h];
				m_pCount[h][bestI]++;
			}

//#if defined(_DEBUG)
//	_CrtCheckMemory();
//#endif

	++it;
	}


	// note we may call a callback here, i.e to repaint...
	if(m_pCallBack&&!(t%m_uInterval)){
		m_pCallBack(m_pUserData);
	}

	// the last time no filtering is needed
	// TODO: check if this is the right place to call back...
	if (t == m_MaxIter-1){
		return true;
	}

	// calculate the actual filter radius

	/*
	this is from Kai's java code where m_ActRadiusX has to be signed
	however, it's the only place where m_ActRadiusX is assigned, and 
	only the first tem can be negative for bigger t, but this is clamped to 1 immediately afterwards
	thus m_ActRadiusX is unsigned in fact...

	note that m_ActRadiusX descends from  m_InitialRadiusX downto 1
	note that m_InitialRadiusX is half the initial map places in x dir, i.e. approx m_MapPlacesX[0]/2
	m_ActRadiusX = (int) (m_InitialRadiusX  * (1-t/m_UpdateDiv)) - 1; 
	if (m_ActRadiusX < 1)	
		m_ActRadiusX = 1;
	
	m_ActRadiusY = (int) (m_InitialRadiusY  * (1-t/m_UpdateDiv)) - 1;  
	if (m_ActRadiusY < 1)	
		m_ActRadiusY = 1;
	*/		
	m_ActRadiusX = (unsigned)(__max((int)1,(int)((m_InitialRadiusX  * (1-t/m_UpdateDiv)) - 1))); 
	m_ActRadiusY = (unsigned)(__max((int)1,(int)((m_InitialRadiusY  * (1-t/m_UpdateDiv)) - 1))); 

	// SOM Update (perform a lowpass box filtering) 
	// calculate the integral images for the weights and the features
	unsigned sizeX = m_pMapPlacesX[0] + 2*m_ActRadiusX+1;
	unsigned sizeY = m_pMapPlacesY[0] + 2*m_ActRadiusY+1;

	if(m_pWeightI){
		delete[] m_pWeightI;
		m_pWeightI=NULL;
	}

	try{
		m_pWeightI = new int[sizeX*sizeY];
		memset(m_pWeightI,0,sizeof(int)*sizeX*sizeY);
	}
	catch(std::bad_alloc){
		// out of mem, what can we do?
		dealloc();
		// pass out exception
		throw;
		// note m_bInitialized still false
		// never reached
		// return false;
	}

	// NOTE from Kai:
	// m_ActDim = m_Dims[t+1];

	if(m_pNetIs){
		delete[] m_pNetIs;
		m_pNetIs=NULL;
	}

	if(m_pNetI){
		delete[] m_pNetI;
		m_pNetI=NULL;
	}
	m_uNetISize=sizeX*sizeY;
	try{
		m_pNetIs=new T[m_NextDim*m_uNetISize];
		memset(m_pNetIs,0,m_NextDim*m_uNetISize*sizeof(T));

		m_pNetI = new T*[m_uNetISize];

		// first feature
		T* p=m_pNetIs;
		unsigned i;

		for(i=0;i<m_uNetISize;++i){
			m_pNetI[i]=p;
			p+=m_NextDim;
		}
	}
	catch(std::bad_alloc){
		// out of mem, what can we do?
		dealloc();
		// pass out exception
		throw;
		// note m_bInitialized still false
		// never reached
		// return false;
	}
	
	calculateWeightIntegralImage();

	// take care: this may return false OR throw a std::bad_alloc, depending on NO_THROW_BADALLOC
	// beeing defined. In either case, dealloc() is called internally. Thus:
	if(!calculateFeatureIntegralImage())
		return false; // if thrown, we don't get here...

	for (unsigned y1 = 0; y1 < m_pMapPlacesY[0]; y1++) {
		unsigned y2 = y1+2*m_ActRadiusY+1;

		for (unsigned x1 = 0; x1 < m_pMapPlacesX[0]; x1++){
			unsigned x2 = x1+2*m_ActRadiusX+1;

			unsigned p22 = y2*sizeX + x2;
			T* aI22 = m_pNetI[p22];
			unsigned p21 = y2*sizeX + x1;
			T* aI21 = m_pNetI[p21];
			unsigned p12 = y1*sizeX + x2;
			T* aI12 = m_pNetI[p12];
			unsigned p11 = y1*sizeX + x1;
			T* aI11 = m_pNetI[p11];

			T oneOverSumWeight = (T)1/(m_pWeightI[p22] - m_pWeightI[p21] - m_pWeightI[p12] + m_pWeightI[p11]);

			unsigned posNet= y1*m_pMapPlacesX[0] + x1;

			T* actNet = m_pNet[0][posNet];

			for (unsigned d=0; d<m_NextDim; ++d) 
				actNet[d] = (aI22[d] - aI21[d] - aI12[d] + aI11[d])*oneOverSumWeight;
		}
	}
	
	for (unsigned h = 0; h < m_HierarchySteps; h++)
		averageNet(h);
	}
#if defined(_DEBUG)
	_CrtCheckMemory();
#endif

	return true;
}


template <typename T,typename TDATA,typename TSORT,typename TMUTEX> void hierarchicSom<T,TDATA,TSORT,TMUTEX>::registerCallback(somCallBack pCallBack,void* pUserData,unsigned interval) 
{
	m_pCallBack=pCallBack;
	m_pUserData=pUserData;
	m_uInterval=interval?interval:1;
}
/*
template <typename T,typename TDATA,typename TSORT,typename TMUTEX> void hierarchicSom<T,TDATA,TSORT,TMUTEX>::registerPauseCallback(somPauseCallBack pCallBack,void* pUserData) 
{
	m_pPauseCallBack=pCallBack;
	m_pPauseUserData=pUserData;
}
*/

// excplicit instantiation
#include "hierarchicsom.expl_templ_inst"
#if defined(_LEAK_H)
	#define new new
#endif
