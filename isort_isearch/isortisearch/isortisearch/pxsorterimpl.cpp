#include "pxsorterimpl.h"
#include "pximage.h"
#include "pxguicontroller.h"
#include "pxguicontrollerimpl.h"
#include <math.h>
#include "hierarchicsom.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif

namespace isortisearch{

template <typename TIMG> const unsigned __int32 sorterImpl<TIMG>::m_uMajorVersion=2;
template <typename TIMG> const unsigned __int32 sorterImpl<TIMG>::m_uMinorVersion=0;


template <typename TIMG> sorterImpl<TIMG>::sorterImpl() : 
	m_bAbort(false),
	m_pSortAdvancedCallBack(NULL),
	m_uCallBackInterval(0),
	m_pSortAdvancedUserData(NULL),
	m_MapPlacesX(0),
	m_MapPlacesY(0),
	m_dAspectRatio(1.0),
	m_dDimFactor(1.2),
	m_pSortImages(NULL),
	m_pGuiControllerImpl(NULL),
	m_bCallBackDraws(false)
{
	;
}

#ifdef __GNUC__
template <typename TIMG> bool sorterImpl<TIMG>::setImages(std::vector<refPtr<TIMG> >* pImages)
#else
template <typename TIMG> bool sorterImpl<TIMG>::setImages(std::vector<refPtr<TIMG>>* pImages)
#endif
{
	m_pSortImages=NULL;
	if(!pImages)
		return false;
	//if(!pImages->size())
	//	return false;
	m_pSortImages=pImages;
	return true;
}
template <typename TIMG> bool sorterImpl<TIMG>::setDimFactor(double dimFactor)
{
	if(dimFactor<1.0||dimFactor>2.0)
		return false;
	m_dDimFactor=dimFactor;

	return true;
}

template <typename TIMG> bool sorterImpl<TIMG>::attach(guiController<TIMG>* pController)
{
	if(!pController||!pController->m_pImpl)
		return false;

	m_pGuiControllerImpl=pController->m_pImpl;
	m_pGuiControllerImpl->m_pSorterMutex=&m_pMutex;
	m_pGuiControllerImpl->m_pSorterImages=&m_pSortImages;
	m_pGuiControllerImpl->m_pSizeChangedCallback=(pxCallback)(&sizeChangedStaticCallback);
	m_pGuiControllerImpl->m_pSizeChangedUserData=this;
	m_pGuiControllerImpl->m_bAttachedToSorter=true;

	return true;
}

template <typename TIMG> bool sorterImpl<TIMG>::calcMapPlaces()
{
	if(!numImages())
		return false;

	// if we have an attached guiController, we have to compute the map places based on it's current canvas size
	// note therefore we have to call this before *each* sort, the size may have changed
	if(m_pGuiControllerImpl){
		const unsigned places=(unsigned)(numImages()*dimFactor());
		unsigned thumbSize;
		
		// we are the controllers friend...
		const unsigned __int32 canvasXSize=m_pGuiControllerImpl->m_CanvasXSize;
		const unsigned __int32 canvasYSize=m_pGuiControllerImpl->m_CanvasYSize;
		// Groesse eines thumbnail-Bereichs
		try{
			thumbSize = (unsigned) sqrt((double)canvasXSize*(double)canvasYSize/(double)places);
		}
		catch(...){
			return false;
		}

		if(!thumbSize)
			return false;

		while (thumbSize>0 && (canvasXSize/thumbSize) * (canvasYSize/thumbSize) < places)
			--thumbSize;

		// erste Annahme
		m_MapPlacesX = canvasXSize / thumbSize;
		m_MapPlacesY = canvasYSize / thumbSize;

		// avoid empty lines at the bottom
		while (m_MapPlacesX*(m_MapPlacesY-1) >= places) {
			m_MapPlacesY--;
		}

		m_pGuiControllerImpl->m_MapPlacesX=m_MapPlacesX;
		m_pGuiControllerImpl->m_MapPlacesY=m_MapPlacesY;
		return true;
	}
	// we don't hava a gui controller attached:
	// compute map places based on aspect ratio
	// we have:
	// m_MapPlacesX*m_MapPlacesY==m_uNumImages*m_dDimFactor
	// m_MapPlacesX/m_MapPlacesY==m_dAspectRatio 
	// => m_MapPlacesY=m_MapPlacesX/m_dAspectRatio
	// => m_MapPlacesX=sqrt(m_uNumImages*m_dDimFactor*m_dAspectRatio);
	try{
		m_MapPlacesX=(unsigned)ceil(sqrt(numImages()*m_dDimFactor*m_dAspectRatio));
		// => m_MapPlacesX=m_MapPlacesY*m_dAspectRatio
		// => m_MapPlacesY=sqrt(m_uNumImages*m_dDimFactor/m_dAspectRatio);
		m_MapPlacesY=(unsigned)ceil(sqrt(numImages()*m_dDimFactor/m_dAspectRatio));
	}
	catch(...){
		return false;
	}
	return true;
}
template <typename TIMG> void sorterImpl<TIMG>::registerSortAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws)
{
	// no error check at all here
	// passing NULL for the callback simply doesn't call it
	// passing NULL for the user data is ok
	m_pSortAdvancedCallBack=interval?pCallBack:NULL;
	m_pSortAdvancedUserData=pUserData;
	m_uCallBackInterval=interval;
	m_bCallBackDraws=bDraws;
}


template <typename TIMG> sort_result sorterImpl<TIMG>::sort(bool bSeparateThread)
{
	m_bAbort=false;

	if(!calcMapPlaces())
		return SORT_ERROR;

	// if we have a Gui controller, reset it and switch to SORT
	if(m_pGuiControllerImpl){
		if(m_pGuiControllerImpl->m_bResized)
			m_pGuiControllerImpl->m_bResizedAndResorted=true;
		m_pGuiControllerImpl->reset(false);
		m_pGuiControllerImpl->setMode(SORTED,false);
	}

	if(bSeparateThread)
		return sortMT();

	// note this is the version of sort which runs in the main thread (hopefully)
	// therefore, we don't need any passed mutex, we use the internal dummy mutex anyway:
	if(m_bAutoMutex)
		disableMutex();
	// note wether or not we have an attached guiController, this uses m_pMuetx, too, which is fine...

	// the som to sort with
	// NOTE the MSVC compiler (9.0) does not compile this w(/o the scope resolution (::) operator
	// it assumes isortisearch::hierarchicSom<> then...
	::hierarchicSom<float,char,refPtr<TIMG>,isortisearch::mutex> mySom(m_pSortImages,m_pMutex,&m_bAbort);

	if(m_pSortAdvancedCallBack){
		// note we register our *own* static callback here if the user callback draws the images...
		// because we have to to more than just call the user callback,
		// we have to call the gui controllers  updateImagePositions
		if(m_bCallBackDraws)
			mySom.registerCallback((somCallBack)sorterImpl::updateImagePositionsStaticCallBack,this,m_uCallBackInterval);
		else
			mySom.registerCallback((somCallBack)m_pSortAdvancedCallBack,m_pSortAdvancedUserData,m_uCallBackInterval);

	}

	if(!mySom.init(m_MapPlacesX,m_MapPlacesY))
		return SORT_ERROR;

	if(!mySom.sort())
		return SORT_ERROR;
	
	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);

	// set and redraw last (probably aborted) sort
	if(m_pSortAdvancedCallBack)
		m_pSortAdvancedCallBack(m_pSortAdvancedUserData);

	// may have been aborted, reset the abort flag...
	if(m_bAbort){
		m_bAbort=false;
		return SORT_ABORTED;
	}

	return SORT_COMPLETED;

}

template <typename TIMG> sort_result sorterImpl<TIMG>::sortMT()
{
	// note this is the version of sort which runs in a separate thread
	// we have to set up a vector with nodes and copy data from the images
	// this is a sort of dbl buffering
	if(!m_Nodes.empty())
		m_Nodes.clear();

	// set up the client mutex (if any)
	if(m_bAutoMutex)
		//if(!enableMutex())
		//	return SORT_ERROR;	
		enableMutex(); // may also run without mutex....

	// note the std::vector<> implememtation of vc++ is like this:
	// at start, the capacity() of the vector is 0
	// when the first element is inserted, a reallocation is done to a capacity() of 1 (!!!)
	// afterwards, the copy ctor of the element we push_back is called
	// when the next element is inserted by push_back, a reallocation is done to a capacity of 2 (!!!)
	// this includes calling the copy ctor for the first element again. afterwards the copy ctor for the
	// next element is called, and so on.
	// i.e, when we insert 1000 elements, the copy ctor is called (1000*1001)/2 times, that is more than 
	// 500.000 times. better call reserve() *first*

	m_Nodes.reserve(m_pSortImages->size());

#ifdef __GNUC__
	typename std::vector<refPtr<TIMG> >::iterator it=m_pSortImages->begin();
	typename std::vector<refPtr<TIMG> >::iterator itEnd=m_pSortImages->end();
#else
	std::vector<refPtr<TIMG>>::iterator it=m_pSortImages->begin();
	std::vector<refPtr<TIMG>>::iterator itEnd=m_pSortImages->end();
#endif
	node n;
	const unsigned sz=n.featureDataByteSize();

	for(;it<itEnd;++it){
		memcpy_s((void*)(n.featureData()),sz,(**it).featureData(),sz);
		m_Nodes.push_back(n);
	}

	// the som to sort with
	// NOTE the MSVC compiler (9.0) does not compile this w(/o the scope resolution (::) operator
	// it assumes isortisearch::hierarchicSom<> then...
	::hierarchicSom<float,char,isortisearch::node,isortisearch::mutex> mySom(&m_Nodes,m_pMutex,&m_bAbort);

	if(m_pSortAdvancedCallBack){
		// note we register our *own* static callback here if the user callback draws the images...
		// because we have to to more than just call the user callback,
		// we have to call updateDataSet()...
		if(m_bCallBackDraws)
			mySom.registerCallback((somCallBack)sorterImpl::updateDataSetStaticCallBack,this,m_uCallBackInterval);
		else
			mySom.registerCallback((somCallBack)m_pSortAdvancedCallBack,m_pSortAdvancedUserData,m_uCallBackInterval);

	}

	if(!mySom.init(m_MapPlacesX,m_MapPlacesY)){
		m_Nodes.clear();
		return SORT_ERROR;
	}

	if(!mySom.sort()){
		m_Nodes.clear();
		return SORT_ERROR;
	}

	updateDataSet();

	// set and redraw last (probably aborted) sort
	if(m_pSortAdvancedCallBack)
		m_pSortAdvancedCallBack(m_pSortAdvancedUserData);

	m_Nodes.clear();
	// set the mutex pointer to the dummy mutex
	if(m_bAutoMutex)
		disableMutex();

	// may have been aborted, reset the abort flag...
	if(m_bAbort){
		m_bAbort=false;
		return SORT_ABORTED;
	}

	return SORT_COMPLETED;
}

template <typename TIMG> void sorterImpl<TIMG>::updateDataSetStaticCallBack(sorterImpl<TIMG> *pThis)
{
	assert(pThis);

	pThis->updateDataSetCallBack();
}


template <typename TIMG> void sorterImpl<TIMG>::updateDataSetCallBack()
{
	if(m_pSortAdvancedCallBack){
		// note this locks:
		updateDataSet();
		m_pSortAdvancedCallBack(m_pSortAdvancedUserData);
	}
}

template <typename TIMG> void sorterImpl<TIMG>::updateImagePositionsStaticCallBack(sorterImpl<TIMG> *pThis)
{
	assert(pThis);

	pThis->updateImagePositionsCallBack();
}


template <typename TIMG> void sorterImpl<TIMG>::updateImagePositionsCallBack()
{
	if(m_pSortAdvancedCallBack){
		// note this locks:
		if(m_pGuiControllerImpl)
			m_pGuiControllerImpl->updateImagePositions();
		m_pSortAdvancedCallBack(m_pSortAdvancedUserData);
	}
}



template <typename TIMG> void sorterImpl<TIMG>::abort()
{
	// note m_pMutex now points either to our internal dummy mutex or a client supplied one
	m_pMutex->lock();
	m_bAbort=true;
	m_pMutex->unlock();

}

template <typename TIMG> void sorterImpl<TIMG>::updateDataSet()
{
	assert(m_pSortImages->size()==m_Nodes.size());

#ifdef __GNUC__
	typename std::vector<refPtr<TIMG> >::iterator it=m_pSortImages->begin();
	typename std::vector<refPtr<TIMG> >::iterator itEnd=m_pSortImages->end();
#else
	std::vector<refPtr<TIMG>>::iterator it=m_pSortImages->begin();
	std::vector<refPtr<TIMG>>::iterator itEnd=m_pSortImages->end();
#endif
	std::vector<node>::const_iterator itN=m_Nodes.begin();

	// note m_pMutex now points either to our internal dummy mutex or a client supplied one
	m_pMutex->lock();

	for(;it<itEnd;++it,++itN){
		((**it)).m_uMapXPos=(*itN).m_uMapXPos;
		((**it)).m_uMapYPos=(*itN).m_uMapYPos;
	}	

	m_pMutex->unlock();

	// let the guiController recalculate image positions
	// note this locks
	// note do not let the guiController call it's callback
	// we may run in a separate thread here...
	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);

}

template <typename TIMG> void sorterImpl<TIMG>::sizeChangedStaticCallback(sorterImpl<TIMG> *pThis)
{
	assert(pThis);
	if(pThis)
		pThis->sizeChanged();
}

template <typename TIMG> void sorterImpl<TIMG>::sizeChanged()
{
	assert(m_pGuiControllerImpl);
	if(m_pGuiControllerImpl)
		setAspectRatio((double)m_pGuiControllerImpl->m_CanvasXSize/(double)m_pGuiControllerImpl->m_CanvasYSize);

}
template class sorterImpl<isortisearch::controllerImage>;
template class sorterImpl<isortisearch::sortImage>;
}

#if defined(_LEAK_H)
	#define new new
#endif
