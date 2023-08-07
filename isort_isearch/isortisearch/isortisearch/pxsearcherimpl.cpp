#include "pxsearcherimpl.h"
#include "pxguicontroller.h"
#include "pxguicontrollerimpl.h"
#include "pximageimpl.h"
#include <assert.h>

namespace isortisearch{

// implementation of searchbase *****************************************************


template <typename TIMG> searchBase<TIMG>::searchBase() : 	
								m_pSearchAdvancedCallback(NULL),
								m_pSearchAdvancedUserData(NULL),
								m_uCallbackIntervall(1),
								m_bCallbackDraws(false),
								m_bDataForTargetsValid(false)
{
	// we assume no more than 10 targets
	// anyway, we may have more, but we get vector<> relocations then...
	m_TargetImages.reserve(10);								
}

template <typename TIMG> searchBase<TIMG>::~searchBase()
{
	clearTargets();
}

template <typename TIMG> bool searchBase<TIMG>::addTarget(const TIMG& img)
{
	if(!img.featureDataValid()){
		return false;
	}

	// do we allow duplicates in the target?
	// for now, yes, although this does not make sense...
	// TODOV5: do we need to lock/unlock?
	// lock();
	m_TargetImages.push_back(img);
	// unlock();
	// added a target, thus:
	m_bDataForTargetsValid=false;
	return true;
}
template <typename TIMG> void searchBase<TIMG>::addTarget(const byteRGB& rgb)
{

	// do we allow duplicates in the target?
	// for now, yes, although this does not make sense...
	// TODOV5: do we need to lock/unlock?
	// lock();
	m_TargetColors.push_back(rgb);
	// unlock();

	// added a target, thus:
	m_bDataForTargetsValid=false;

	return;
}

template <typename TIMG> void searchBase<TIMG>::registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws)
{
	m_pSearchAdvancedCallback=pCallBack;
	m_pSearchAdvancedUserData=pUserData;
	m_uCallbackIntervall=interval;
	m_bCallbackDraws=bDraws;
}

template <typename TIMG> float searchBase<TIMG>::distanceToImageTarget(const image* target,const image *test)
{
	// Note target tested in addTarget() already...
	if(/*!target->featureDataValid()||*/!test->featureDataValid()){
		assert(false);
		return std::numeric_limits<float>::max();
	}
	// same image?
	if(target==test)
		return (float)0.;

	// L1 distance for similarity search of images (sum of absolute differences of feature data)	
	float dist = (float)0.,d;
	const char* const pData1=target->featureData();
	const char* const pData2=test->featureData();
	for(unsigned i = 0; i<FEATURESIZE; ++i) {
		d = (float)((*(pData1+i))-(*(pData2+i)));
		dist += abs(d);
	}
	return dist;
}


template <typename TIMG> float searchBase<TIMG>::minDistToTargetImages(const image* pImg,void *pData)
{
	// Note pData not used here...
	float minDist=std::numeric_limits<float>::max();

#ifdef __GNUC__
	typename std::vector<TIMG>::iterator it= m_TargetImages.begin();
	typename std::vector<TIMG>::iterator itEnd= m_TargetImages.end();
#else
	std::vector<const TIMG>::const_iterator it=m_TargetImages.begin();
	std::vector<const TIMG>::const_iterator itEnd=m_TargetImages.end();
#endif
	// loop over all target images and get the minimum distance to image passed in pImg
	while(it!=itEnd){
		// note we may do this inlne...
		float dist = distanceToImageTarget(&(*it),pImg);

		if(dist<minDist)
			minDist=dist;

		++it;
	}
	return minDist;
}

template <typename TIMG> float searchBase<TIMG>::minDistToTargetColors(const image* pImg,void *pData)
{
	if(!pImg->featureDataValid()){
		assert(false);
		return std::numeric_limits<float>::max();
	}
	
	// here, pData is a ptr to the histo data of *all* target colors
	// type is char and size is HISTOSIZE
	assert(pData);
	const char* const pData1=(char*)pData;

	// L4 distance, but only for the color histogram data
	float dist = (float)0.,d;
	const char* const pData2=pImg->featureData()+imageImpl::m_FeatureGenerator.histoOffset();
	for(unsigned i = 0; i<HISTOSIZE; ++i) {
		d = (float)((*(pData1+i))-(*(pData2+i)));
		d*=d;
		dist += d;
	}
	return dist;
	
}

template <typename TIMG> float searchBase<TIMG>::minDistToTargets(const image* pImg,void *pData)
{
	float minDist=std::numeric_limits<float>::max();

	// here, pData is a ptr to a std::vector<fixedArray<char,HISTOSIZE>>
	// containing the combined histo data of all images and all colors
	const std::vector<fixedArrayV5<char,HISTOSIZE> >* histoData=(std::vector<fixedArrayV5<char,HISTOSIZE> >*)(pData);
	// note in this case, we do not loop over the targets, but the combined histo data
	// from the targets and the colors...
	std::vector<fixedArrayV5<char,HISTOSIZE> >::const_iterator it=histoData->begin();
	std::vector<fixedArrayV5<char,HISTOSIZE> >::const_iterator itEnd=histoData->end();

	//this loop will iterate over the target images and get the minimum distance to image passed
	// combined with the color target(s)
	while(it!=itEnd){
		float dist = distanceToTargetAndColors(*it,pImg);

		if(dist<minDist)
			minDist=dist;

		++it;
	}
	return minDist;
}

template <typename TIMG> float searchBase<TIMG>::distanceToTargetAndColors(const fixedArrayV5<char,HISTOSIZE>& data,const image *test)
{
	
	if(!test->featureDataValid()){
		assert(false);
		return std::numeric_limits<float>::max();
	}

	// NOTE: from Kai's current code, when doing a combined search for images and colors,
	// a combined feature verctor is build from the image feature data and the color feature data
	// However, *only* the histogram data from the image feature data is used...

	float dist = (float)0.,d;
	const char* const pData2=test->featureData();
	for(unsigned i = 0; i<HISTOSIZE; ++i) {
		d = (float)(data[i]-(*(pData2+i)));
		d*=d;
		dist += d;
	}
	return dist;
}

template <typename TIMG> void searchBase<TIMG>::generateCombinedHistoData(std::vector<fixedArrayV5<char,HISTOSIZE> >& data)
{
	assert(!(m_TargetImages.empty()));
	assert(!(m_TargetColors.empty()));
	// generate feature data for all colors
	char colorHistoData[HISTOSIZE];
	imageImpl::m_FeatureGenerator.calculateFeatureData(m_TargetColors,colorHistoData);
	fixedArrayV5<char,HISTOSIZE> combinedHistoData;
#ifdef __GNUC__
	typename std::vector<TIMG>::iterator it= m_TargetImages.begin();
	typename std::vector<TIMG>::iterator itEnd= m_TargetImages.end();
#else
	std::vector<const TIMG>::const_iterator it=m_TargetImages.begin();
	std::vector<const TIMG>::const_iterator itEnd=m_TargetImages.end();
#endif
	const char* const pData1=colorHistoData;
	int qc,qi,qic;
	// loop over all target images, get feature data and combine histo data with color histo data
	// push back to std::vector passed in data
	while(it!=itEnd){
		const char* const pData2=(*it).featureData()+imageImpl::m_FeatureGenerator.histoOffset();
		for(unsigned i = 0; i<HISTOSIZE; ++i) {
			qc = ((int)*(pData1+i))+127;
			qi = ((int)*(pData2+i))+127;
			qic = ((int) __max(0.5*(float)qc,(float)qi))-127;

			combinedHistoData[i] = (char) __min(127,qic);
		}

		data.push_back(combinedHistoData);
		++it;
	}
}

// implementation of searcherImpl *****************************************************

// a typedef for the distance function we use (based on wether we search for image(s), color(s) or both
// Note pData is something different in each of the three cases and is casted internally
// typedef float(*searchBase<typename TIMG>::pMinDistToTargetsFunc)(const image* pImg,void *pData);

template <typename TIMG> searcherImpl<TIMG>::searcherImpl() : 	
								m_pGuiControllerImpl(NULL),
								m_pImages(NULL)
{
}

template <typename TIMG> searcherImpl<TIMG>::~searcherImpl()
{
	clearSearchResults();
}

//template <typename TIMG> bool searcherImpl<TIMG>::setImages(std::vector</*const*/ TIMG*>* pImages)
#ifdef __GNUC__
	template <typename TIMG> bool searcherImpl<TIMG>::setImages(std::vector<refPtr<TIMG> >* pImages)
#else
	template <typename TIMG> bool searcherImpl<TIMG>::setImages(std::vector<refPtr<TIMG>>* pImages)
#endif
{
	m_pImages=NULL;
	if(!pImages/*||!pImages->size()*/)
		return false;
	m_pImages=pImages;
	return true;
}

template <typename TIMG> bool searcherImpl<TIMG>::search(unsigned numResults,bool bSeparateThread/*,target_type type*/)
{
	clearSearchResults();

	// any targets?
#ifdef __GNUC__
	if(searcherImpl<TIMG>::m_TargetImages.empty()&&m_TargetColors.empty())
		return false;
#else
	if(m_TargetImages.empty()&&m_TargetColors.empty())
		return false;
#endif
	// any images?
	if(!m_pImages)
		return false;
	// any images?
	if(m_pImages->empty())
		return false;
	// size given?
	if(!numResults)
		return false;


	// the combined feature data of the color targets
	// this will be calculated and used only when searching for colors only
	char colorFeatureData[HISTOSIZE];

	// the combined feature data of all target colors and all target images
	// this will be calculated and filled only when searchin for colors and images
	std::vector<fixedArrayV5<char,HISTOSIZE> > combinedHistoData;

	// (*) we either have only target colors or only target images or both
	// set the function pointer for the correct function before we loop...
	float (searchBase<TIMG>::*pMinDistToTargets)(const image* pImg,void *pData)=NULL;

	void *pData=NULL;
	if(m_TargetImages.empty()){
		// search for colors only...
		assert(!(m_TargetColors.empty()));
		// set up function ptr
		pMinDistToTargets=&searchBase<TIMG>::minDistToTargetColors;
		// calculate feature data for all target colors
		imageImpl::m_FeatureGenerator.calculateFeatureData(m_TargetColors,colorFeatureData);
		pData=(void*)colorFeatureData;
	}
	else if(m_TargetColors.empty()){
		// search for images only...
		assert(!(m_TargetImages.empty()));
		// set up function ptr
		pMinDistToTargets=&searchBase<TIMG>::minDistToTargetImages;
		// leave pData=NULL;
	}
	else{
		// search for both colors and images
		// set up function ptr
		pMinDistToTargets=&searchBase<TIMG>::minDistToTargets;
		// calculate combined feature data for all target colors and images
		generateCombinedHistoData(combinedHistoData);
		assert(!(combinedHistoData.empty()));
		pData=(void*)(&combinedHistoData);
	}

	// if we have a Gui controller, reset it and switch to LIST_SEARCH_RESULTS
	if(m_pGuiControllerImpl){
		if(m_pGuiControllerImpl->m_bResized)
			m_pGuiControllerImpl->m_bResizedAndResorted=true;
		m_pGuiControllerImpl->reset(false);
		// TODOV5: we may want to use the PYRAMID mode, too
		m_pGuiControllerImpl->setMode(LIST_SEARCH_RESULTS,false);
	}

	// check if we need to lock
	// note if client mutex not set, this is the internal dummy mutex...
#ifdef __GNUC__
	if(searcherImpl<TIMG>::m_bAutoMutex){
		if(bSeparateThread&&searcherImpl<TIMG>::m_pSearchAdvancedCallback&&searcherImpl<TIMG>::m_uCallbackIntervall&&searcherImpl<TIMG>::m_bCallbackDraws)
			this->enableMutex();
		else
			this->disableMutex();
	}
	
	std::multimap<float,refPtr<TIMG> > map;
	float minDist=std::numeric_limits<float>::max();
	float greatestDist=minDist;
	unsigned numInsertCalls=0;
	typename std::multimap<float,refPtr<TIMG> >::const_iterator mapIt;
	typename std::multimap<float,refPtr<TIMG> >::const_iterator mapItEnd;
	
	typename std::vector<refPtr<TIMG> >::const_iterator it=m_pImages->begin();
	typename std::vector<refPtr<TIMG> >::const_iterator itEnd=m_pImages->end();
	while(it!=itEnd){
		/*const*/ TIMG* pImg=(*it).operator->();
		
		// (*) see if-clause above...
		minDist=pMinDistToTargets(pImg,pData);
		
		// if we already have enough results and the dist is greater than the greatest,	do nothing
		if(map.size()>=numResults&&minDist>=greatestDist){
			++it;
			continue;
		}
		
		// add to map
		// note same keys are allowed
		//map.insert(std::pair<float,/*const*/ TIMG*>(minDist,pImg));
		map.insert(std::pair<float,refPtr<TIMG> >(minDist,*it));
		++numInsertCalls;
		++it;
		
		// if bigger than size(), shrink from end
		while(map.size()>numResults){
			mapIt=map.end();
			--mapIt;
			map.erase( --(map.end()) );
		}
		
		// get new greatest dist
		mapIt=map.end();
		--mapIt;
		greatestDist=(*mapIt).first;
		
		if(searcherImpl<TIMG>::m_pSearchAdvancedCallback&&searcherImpl<TIMG>::m_uCallbackIntervall&&!(numInsertCalls%searcherImpl<TIMG>::m_uCallbackIntervall)){
			if(searcherImpl<TIMG>::m_bCallbackDraws){
				// copy from multimap to the results vector
				this->lock();
				m_SearchResults.clear();
				mapIt=map.begin();
				mapItEnd=map.end();
				while(mapIt!=mapItEnd){
					m_SearchResults.push_back((*mapIt).second);
					++mapIt;
				}
				this->unlock();
				if(m_pGuiControllerImpl)
					m_pGuiControllerImpl->updateImagePositions(false);
			}
			m_pSearchAdvancedCallback(searcherImpl<TIMG>::m_pSearchAdvancedUserData);
		}
	}
	
	// note copy the sorted image pointers from the multimap to the results
	// lock this if needed
	this->lock();
	m_SearchResults.clear();
	mapIt=map.begin();
	mapItEnd=map.end();
	while(mapIt!=mapItEnd){
		m_SearchResults.push_back((*mapIt).second);
		++mapIt;
	}
	this->unlock();
	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);
	// clear result map, no longer needed...
	map.clear();
	
	// note this locks, too, if in main thread and we are in separate thread...
	if(searcherImpl<TIMG>::m_pSearchAdvancedCallback)
		m_pSearchAdvancedCallback(searcherImpl<TIMG>::m_pSearchAdvancedUserData);
	
	if(searcherImpl<TIMG>::m_bAutoMutex)
		this->disableMutex();
#else
	if(m_bAutoMutex){
		if(bSeparateThread&&m_pSearchAdvancedCallback&&m_uCallbackIntervall&&m_bCallbackDraws)
			enableMutex();
		else
			disableMutex();
	}

	std::multimap<float,refPtr<TIMG>> map;
	float minDist=std::numeric_limits<float>::max();
	float greatestDist=minDist;
	unsigned numInsertCalls=0;
	std::multimap<float,refPtr<TIMG>>::const_iterator mapIt;
	std::multimap<float,refPtr<TIMG>>::const_iterator mapItEnd;

	std::vector<refPtr<TIMG>>::const_iterator it=m_pImages->begin();
	std::vector<refPtr<TIMG>>::const_iterator itEnd=m_pImages->end();
	while(it!=itEnd){
		TIMG* pImg=(*it).operator->();

		minDist=(this->*pMinDistToTargets)(pImg,pData);

		// if we already have enough results and the dist is greater than the greatest,	do nothing
		if(map.size()>=numResults&&minDist>=greatestDist){
			++it;
			continue;
		}

		// add to map
		// note same keys are allowed
		map.insert(std::pair<float,refPtr<TIMG>>(minDist,*it));
		++numInsertCalls;
		++it;

		// if bigger than size(), shrink from end
		while(map.size()>numResults){
			mapIt=map.end();
			--mapIt;
			map.erase(mapIt);
		}

		// get new greatest dist
		mapIt=map.end();
		--mapIt;
		greatestDist=(*mapIt).first;

		// take care: a modulo operation by 0 is a division by 0 !!!
		if(m_pSearchAdvancedCallback&&m_uCallbackIntervall&&!(numInsertCalls%m_uCallbackIntervall)){
			if(m_bCallbackDraws){
				// copy from multimap to the results vector
				lock();
				m_SearchResults.clear();
				mapIt=map.begin();
				mapItEnd=map.end();
				while(mapIt!=mapItEnd){
					m_SearchResults.push_back((*mapIt).second);
					++mapIt;
				}
				unlock();
				if(m_pGuiControllerImpl)
					m_pGuiControllerImpl->updateImagePositions(false);
			}
			m_pSearchAdvancedCallback(m_pSearchAdvancedUserData);
		}
	}

	// note copy the sorted image pointers from the multimap to the results
	// lock this if needed
	lock();
	m_SearchResults.clear();
	mapIt=map.begin();
	mapItEnd=map.end();
	while(mapIt!=mapItEnd){
		m_SearchResults.push_back((*mapIt).second);
		++mapIt;
	}
	unlock();
	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);
	// clear result map, no longer needed...
	map.clear();

	// note this locks, too, if in main thread and we are in separate thread...
	if(m_pSearchAdvancedCallback)
		m_pSearchAdvancedCallback(m_pSearchAdvancedUserData);

	if(m_bAutoMutex)
		disableMutex();
#endif


	return true;
}

template <typename TIMG> bool searcherImpl<TIMG>::attach(guiController<TIMG>* pController)
{
	if(!pController||!pController->m_pImpl)
		return false;

	m_pGuiControllerImpl=pController->m_pImpl;
	m_pGuiControllerImpl->m_pSearcherImages=&m_pImages;
	m_pGuiControllerImpl->m_pSearcherResults=&m_SearchResults;
#ifdef __GNUC__
	// i have no idea know why gcc need the this pointer here:
	m_pGuiControllerImpl->m_pSearcherMutex=&(this->m_pMutex);
#else
	m_pGuiControllerImpl->m_pSearcherMutex=&m_pMutex;
#endif
	m_pGuiControllerImpl->m_bAttachedToSearcher=true;
	return true;
}


// implementation of dynamicSearcherImpl *****************************************************


template <typename TIMG> dynamicSearcherImpl<TIMG>::dynamicSearcherImpl() : 	
								m_pGuiControllerImpl(NULL),
								m_ResultSize(0),
								// m_TargetType(PHOTO),
								m_NumInsertCalls(0),
								m_pMinDistToTargets(NULL),
								m_pData(NULL)
{
	// the greatest dist so far
	m_GreatestDist=std::numeric_limits<float>::max();
#ifdef __GNUC__
	 this->enableAutoMutex(false);
#else
	enableAutoMutex(false);
#endif
}

template <typename TIMG> dynamicSearcherImpl<TIMG>::~dynamicSearcherImpl()
{
	clear();
	m_CombinedHistoData.clear();
}

template <typename TIMG> bool dynamicSearcherImpl<TIMG>::setResultSize(unsigned size)
{
	if(size){
		m_ResultSize=size;
		return true;
	}
	return false;
}

template <typename TIMG> bool dynamicSearcherImpl<TIMG>::dynamicSearch(const refPtr<TIMG>& pImg)
{
	// do we have any targets?
	// result size set?
	// feature data valid?
#ifdef __GNUC__
	if(dynamicSearcherImpl<TIMG>::m_TargetImages.empty()||!m_ResultSize||!pImg->featureDataValid())
		return false;
	
	float minDist=minDistToTargets(pImg.operator->(),m_TargetType);
	
	// if we already have enough results and the dist is greater than the greatest,	do nothing
	if(m_DynamicSearchResults.size()>=m_ResultSize&&minDist>=m_GreatestDist)
		return true;
	
	// add to inserted
	// note same keys are allowed
	this->lock();
	m_DynamicSearchResults.insert(std::pair<float,refPtr<TIMG> >(minDist,pImg));
	
	++m_NumInsertCalls;
	
	// if bigger than m_ResultSize, shrink from end
	while(m_DynamicSearchResults.size()>m_ResultSize){
		typename std::multimap<float,refPtr<TIMG> >::iterator it=m_DynamicSearchResults.end();
		--it;
		m_DynamicSearchResults.erase(it);
	}
	
	// get new greatest dist
	typename std::multimap<float,refPtr<TIMG> >::iterator it2End=m_DynamicSearchResults.end();
	--it2End;
	m_GreatestDist=(*it2End).first;
	
	this->unlock();
	
	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);
	
	if(dynamicSearcherImpl<TIMG>::m_pSearchAdvancedCallback&&dynamicSearcherImpl<TIMG>::m_uCallbackIntervall&&!(m_NumInsertCalls%dynamicSearcherImpl<TIMG>::m_uCallbackIntervall)){
		dynamicSearcherImpl<TIMG>::m_pSearchAdvancedCallback(dynamicSearcherImpl<TIMG>::m_pSearchAdvancedUserData);
	}
#else
	if(!numTargets()||!m_ResultSize||pImg.isNull()||!pImg->featureDataValid())
		return false;

	// re-calculate the data whe need for the different targets (colors only, images only (none), images and colors)
	if(!m_bDataForTargetsValid){
		// we either don't need this any longer or have to recalculate it, thus:
		m_CombinedHistoData.clear();

		if(m_TargetImages.empty()){
			// search for colors only...
			assert(!(m_TargetColors.empty()));
			// set up function ptr
			m_pMinDistToTargets=&searchBase<TIMG>::minDistToTargetColors;
			// calculate feature data for all target colors
			imageImpl::m_FeatureGenerator.calculateFeatureData(m_TargetColors,m_ColorFeatureData);
			m_pData=m_ColorFeatureData;
		}
		else if(m_TargetColors.empty()){
			// search for images only...
			assert(!(m_TargetImages.empty()));
			// set up function ptr
			m_pMinDistToTargets=&searchBase<TIMG>::minDistToTargetImages;
			m_pData=NULL;
		}
		else{
			// search for both colors and images
			// set up function ptr
			m_pMinDistToTargets=&searchBase<TIMG>::minDistToTargets;
			// calculate combined feature data for all target colors and images
			generateCombinedHistoData(m_CombinedHistoData);
			assert(!(m_CombinedHistoData.empty()));
			m_pData=(void*)(&m_CombinedHistoData);
		}
		// do this once only
		m_bDataForTargetsValid=true;
	}


	float minDist=(this->*m_pMinDistToTargets)(pImg.operator->(),m_pData);

	// if we already have enough results and the dist is greater than the greatest,	do nothing
	if(m_DynamicSearchResults.size()>=m_ResultSize&&minDist>=m_GreatestDist)
		return true;

	// add to inserted
	// note same keys are allowed
	lock();
	m_DynamicSearchResults.insert(std::pair<float,refPtr<TIMG> >(minDist,pImg));

	++m_NumInsertCalls;

	// if bigger than m_ResultSize, shrink from end
	while(m_DynamicSearchResults.size()>m_ResultSize){
		std::multimap<float,refPtr<TIMG> >::iterator it=m_DynamicSearchResults.end();
		--it;
		m_DynamicSearchResults.erase(it);
	}

	// get new greatest dist
	std::multimap<float,refPtr<TIMG> >::iterator it2End=m_DynamicSearchResults.end();
	--it2End;
	m_GreatestDist=(*it2End).first;

	unlock();

	if(m_pGuiControllerImpl)
		m_pGuiControllerImpl->updateImagePositions(false);

	if(m_pSearchAdvancedCallback&&m_uCallbackIntervall&&!(m_NumInsertCalls%m_uCallbackIntervall)){
		m_pSearchAdvancedCallback(m_pSearchAdvancedUserData);
	}
#endif

	return true;
}

template <typename TIMG> bool dynamicSearcherImpl<TIMG>::attach(guiController<TIMG>* pController)
{
	if(!pController||!pController->m_pImpl)
		return false;

	m_pGuiControllerImpl=pController->m_pImpl;
	m_pGuiControllerImpl->m_pDynamicSearcherResults=&m_DynamicSearchResults;
#ifdef __GNUC__
	// i have no idea know why gcc need the this pointer here:
	m_pGuiControllerImpl->m_pDynamicSearcherMutex=&(this->m_pMutex);
#else
	m_pGuiControllerImpl->m_pDynamicSearcherMutex=&m_pMutex;
#endif
	m_pGuiControllerImpl->m_bAttachedToDynamicSearcher=true;
	return true;
}

template class searchBase<isortisearch::image>;
template class searchBase<isortisearch::controllerImage>;
template class searchBase<isortisearch::sortImage>;

template class searcherImpl<isortisearch::image>;
template class searcherImpl<isortisearch::controllerImage>;
template class searcherImpl<isortisearch::sortImage>;

template class dynamicSearcherImpl<isortisearch::image>;
template class dynamicSearcherImpl<isortisearch::sortImage>;
template class dynamicSearcherImpl<isortisearch::controllerImage>;

}