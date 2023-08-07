#include "pxsorter.h"
#include "pxsorterimpl.h"
#include "pximage.h"
#include <assert.h>

namespace isortisearch{

template <typename TIMG> sorter<TIMG>::sorter() : 
	m_pImpl(NULL)

{
	m_pImpl = new sorterImpl<TIMG>;
}

template <typename TIMG> sorter<TIMG>::~sorter()

{
	if(m_pImpl)
		delete m_pImpl;
}

template <typename TIMG> unsigned __int32 sorter<TIMG>::majorVersion() 
{
	return sorterImpl<TIMG>::m_uMajorVersion;
}

template <typename TIMG> unsigned __int32 sorter<TIMG>::minorVersion() 
{
	return sorterImpl<TIMG>::m_uMinorVersion;
}

#ifdef __GNUC__
	template <typename TIMG> bool sorter<TIMG>::setImages(std::vector<refPtr<TIMG> >* pImages)
#else
	template <typename TIMG> bool sorter<TIMG>::setImages(std::vector<refPtr<TIMG>>* pImages)
#endif
{
	assert(m_pImpl);
	return m_pImpl->setImages(pImages);
}

template <typename TIMG> bool sorter<TIMG>::setDimFactor(double dimFactor)
{
	assert(m_pImpl);
	return m_pImpl->setDimFactor(dimFactor);
}

template <typename TIMG> unsigned __int32 sorter<TIMG>::mapPlacesX() const
{
	assert(m_pImpl);
	return m_pImpl->m_MapPlacesX;
}

template <typename TIMG> unsigned __int32 sorter<TIMG>::mapPlacesY() const
{
	assert(m_pImpl);
	return m_pImpl->m_MapPlacesY;
}

template <typename TIMG> unsigned __int32 sorter<TIMG>::numImages() const
{
	assert(m_pImpl);
	return m_pImpl->numImages();
}

template <typename TIMG> double sorter<TIMG>::aspectRatio() const
{
	assert(m_pImpl);
	return m_pImpl->m_dAspectRatio;
}

template <typename TIMG> double sorter<TIMG>::dimFactor() const
{
	assert(m_pImpl);
	return m_pImpl->m_dDimFactor;
}
	 
template <typename TIMG> bool sorter<TIMG>::attach(guiController<TIMG>* pController)
{
	assert(m_pImpl);
	return m_pImpl->attach(pController);
}

template <typename TIMG> bool sorter<TIMG>::setAspectRatio(double ratio)
{
	assert(m_pImpl);
	if(ratio<=0.0)
		return false;
	m_pImpl->m_dAspectRatio=ratio;
	return true;
}

template <typename TIMG> void sorter<TIMG>::registerSortAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws)
{
	assert(m_pImpl);
	m_pImpl->registerSortAdvancedCallback(pCallBack,pUserData,interval,bDraws);
}

template <typename TIMG> sort_result sorter<TIMG>::sort(bool bSeparateThread)
{
	assert(m_pImpl);
	return m_pImpl->sort(bSeparateThread);
}

template <typename TIMG> void sorter<TIMG>::abort()
{
	assert(m_pImpl);
	m_pImpl->abort();
}


template <typename TIMG> bool sorter<TIMG>::setMutex(isortisearch::mutex* pMutex)
{
	assert(m_pImpl);
	return m_pImpl->setMutex(pMutex);
}

template <typename TIMG> void sorter<TIMG>::lock()
{
	assert(m_pImpl);
	m_pImpl->lock();
}

template <typename TIMG> void sorter<TIMG>::unlock()
{
	assert(m_pImpl);
	m_pImpl->unlock();
}

template <typename TIMG> void sorter<TIMG>::enableMutex()
{
	assert(m_pImpl);
	m_pImpl->enableMutex();
}

template <typename TIMG> void sorter<TIMG>::disableMutex()
{
	assert(m_pImpl);
	m_pImpl->disableMutex();
}
template <typename TIMG> void sorter<TIMG>::enableAutoMutex(bool bEnable)
{
	assert(m_pImpl);
	m_pImpl->enableAutoMutex(bEnable);
}

template class sorter<isortisearch::controllerImage>;
template class sorter<isortisearch::sortImage>;

}

