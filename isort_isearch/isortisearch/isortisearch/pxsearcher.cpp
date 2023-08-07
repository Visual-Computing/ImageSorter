#include "pxsearcher.h"
#include "pxsearcherimpl.h"
#include "pximage.h"
#include <assert.h>
namespace isortisearch{

template<typename TIMG> searcher<TIMG>::searcher()
{
	m_pImpl=new searcherImpl<TIMG>;
}

template<typename TIMG> searcher<TIMG>::~searcher()
{
	delete m_pImpl;
}

template<typename TIMG> bool searcher<TIMG>::addTarget(const TIMG& img)
{
	assert(m_pImpl);
	return m_pImpl->addTarget(img);
}

template<typename TIMG> void searcher<TIMG>::addTarget(unsigned char r,unsigned char g, unsigned char b)
{
	assert(m_pImpl);
	return m_pImpl->addTarget(byteRGB(r,g,b));
}


template<typename TIMG> void searcher<TIMG>::clearResults()
{
	assert(m_pImpl);
	m_pImpl->clearSearchResults();
}

template<typename TIMG> void searcher<TIMG>::clearTargets()
{
	assert(m_pImpl);
	m_pImpl->clearTargets();
}

template<typename TIMG> void searcher<TIMG>::clearTargetImages()
{
	assert(m_pImpl);
	m_pImpl->clearTargetImages();
}

template<typename TIMG> void searcher<TIMG>::clearTargetColors()
{
	assert(m_pImpl);
	m_pImpl->clearTargetColors();
}

template <typename TIMG> unsigned __int32 searcher<TIMG>::numImages() const
{
	assert(m_pImpl);
	return m_pImpl->numImages();
}

template<typename TIMG> bool searcher<TIMG>::search(unsigned numResults,bool bSeparateThread)
{
	assert(m_pImpl);
	return m_pImpl->search(numResults,bSeparateThread);
}
#ifdef __GNUC__
	template<typename TIMG> bool searcher<TIMG>::setImages(std::vector<refPtr<TIMG> >* pImages)
#else
	template<typename TIMG> bool searcher<TIMG>::setImages(std::vector<refPtr<TIMG>>* pImages)
#endif
{
	assert(m_pImpl);
	return m_pImpl->setImages(pImages);
}

#ifdef __GNUC__
	template<typename TIMG> std::vector<refPtr<TIMG> >& searcher<TIMG>::results()
#else
	template<typename TIMG> std::vector<refPtr<TIMG>>& searcher<TIMG>::results()
#endif
{
	assert(m_pImpl);
	return m_pImpl->searchResults();
}

template<typename TIMG> void searcher<TIMG>::registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws)
{
	assert(m_pImpl);
	m_pImpl->registerSearchAdvancedCallback(pCallBack,pUserData,interval,bDraws);
}

template<typename TIMG> unsigned searcher<TIMG>::numTargets() const
{
	assert(m_pImpl);
	return m_pImpl->numTargets();
}

template<typename TIMG> unsigned searcher<TIMG>::numTargetImages() const
{
	assert(m_pImpl);
	return m_pImpl->numTargetImages();
}

template<typename TIMG> unsigned searcher<TIMG>::numTargetColors() const
{
	assert(m_pImpl);
	return m_pImpl->numTargetColors();
}

template<typename TIMG> bool searcher<TIMG>::setMutex(isortisearch::mutex* pMutex)
{
	assert(m_pImpl);
	return m_pImpl->setMutex(pMutex);
}

template<typename TIMG> void searcher<TIMG>::lock()
{
	assert(m_pImpl);
	m_pImpl->lock();
}

template<typename TIMG> void searcher<TIMG>::unlock()
{
	assert(m_pImpl);
	m_pImpl->unlock();
}

template<typename TIMG> void searcher<TIMG>::enableMutex()
{
	assert(m_pImpl);
	m_pImpl->enableMutex();
}

template<typename TIMG> void searcher<TIMG>::disableMutex()
{
	assert(m_pImpl);
	m_pImpl->disableMutex();
}

template<typename TIMG> void searcher<TIMG>::enableAutoMutex(bool bEnable)
{
	assert(m_pImpl);
	m_pImpl->enableAutoMutex(bEnable);
}

template<typename TIMG> bool searcher<TIMG>::attach(guiController<TIMG>* pController)
{
	assert(m_pImpl);
	return m_pImpl->attach(pController);
}

template<typename TIMG> dynamicSearcher<TIMG>::dynamicSearcher()
{
	m_pImpl=new dynamicSearcherImpl<TIMG>;
}

template<typename TIMG> dynamicSearcher<TIMG>::~dynamicSearcher()
{
	delete m_pImpl;
}

template<typename TIMG> bool dynamicSearcher<TIMG>::addTarget(const TIMG& img)
{
	assert(m_pImpl);
	return m_pImpl->addTarget(img);
}

template<typename TIMG> void dynamicSearcher<TIMG>::addTarget(unsigned char r,unsigned char g, unsigned char b)
{
	assert(m_pImpl);
	return m_pImpl->addTarget(byteRGB(r,g,b));
}


template<typename TIMG> void dynamicSearcher<TIMG>::clearResults()
{
	assert(m_pImpl);
	m_pImpl->clear();
}

template<typename TIMG> void dynamicSearcher<TIMG>::clearTargets()
{
	assert(m_pImpl);
	m_pImpl->clearTargets();
}

template<typename TIMG> void dynamicSearcher<TIMG>::clearTargetImages()
{
	assert(m_pImpl);
	m_pImpl->clearTargetImages();
}

template<typename TIMG> void dynamicSearcher<TIMG>::clearTargetColors()
{
	assert(m_pImpl);
	m_pImpl->clearTargetColors();
}

template<typename TIMG> bool dynamicSearcher<TIMG>::dynamicSearch(const refPtr<TIMG>& pImg)
{
	assert(m_pImpl);
	return m_pImpl->dynamicSearch(pImg);
}

template<typename TIMG> /*const*/ std::multimap<float,refPtr<TIMG> >& dynamicSearcher<TIMG>::dynamicSearchResults()
{
	assert(m_pImpl);
	return m_pImpl->dynamicSearchResults();
}

template<typename TIMG> void dynamicSearcher<TIMG>::registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws)
{
	assert(m_pImpl);
	m_pImpl->registerSearchAdvancedCallback(pCallBack,pUserData,interval,bDraws);
}

template<typename TIMG> unsigned dynamicSearcher<TIMG>::numTargets() const
{
	assert(m_pImpl);
	return m_pImpl->numTargets();
}

template<typename TIMG> unsigned dynamicSearcher<TIMG>::numTargetImages() const
{
	assert(m_pImpl);
	return m_pImpl->numTargetImages();
}

template<typename TIMG> unsigned dynamicSearcher<TIMG>::numTargetColors() const
{
	assert(m_pImpl);
	return m_pImpl->numTargetColors();
}

template<typename TIMG> bool dynamicSearcher<TIMG>::setMutex(isortisearch::mutex* pMutex)
{
	assert(m_pImpl);
	return m_pImpl->setMutex(pMutex);
}

template<typename TIMG> void dynamicSearcher<TIMG>::lock()
{
	assert(m_pImpl);
	m_pImpl->lock();
}

template<typename TIMG> void dynamicSearcher<TIMG>::unlock()
{
	assert(m_pImpl);
	m_pImpl->unlock();
}

template<typename TIMG> void dynamicSearcher<TIMG>::enableMutex()
{
	assert(m_pImpl);
	m_pImpl->enableMutex();
}

template<typename TIMG> void dynamicSearcher<TIMG>::disableMutex()
{
	assert(m_pImpl);
	m_pImpl->disableMutex();
}

template<typename TIMG> bool dynamicSearcher<TIMG>::attach(guiController<TIMG>* pController)
{
	assert(m_pImpl);
	return m_pImpl->attach(pController);
}

template<typename TIMG> bool dynamicSearcher<TIMG>::setResultSize(unsigned __int32 size)
{
	assert(m_pImpl);
	return m_pImpl->setResultSize(size);
}

template<typename TIMG> unsigned __int32 dynamicSearcher<TIMG>::resultSize() const
{
	assert(m_pImpl);
	return m_pImpl->resultSize();
}

template<typename TIMG> void dynamicSearcher<TIMG>::resetCallbackCounter()
{
	assert(m_pImpl);
	return m_pImpl->resetCallbackCounter();
}

template class searcher<isortisearch::image>;
template class searcher<isortisearch::controllerImage>;
template class searcher<isortisearch::sortImage>;

template class dynamicSearcher<isortisearch::image>;
template class dynamicSearcher<isortisearch::controllerImage>;
template class dynamicSearcher<isortisearch::sortImage>;
}