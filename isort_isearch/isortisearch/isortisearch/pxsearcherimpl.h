#if !defined(SEARCHERIMPL_H)
#define SEARCHERIMPL_H

#include "pxmutexhandler.h"
#include "pxdefs.h"
#include "color.h"
#include "arrayV5.h"
#include "pximpldefs.h"
#include <map>
#include <vector>
#include <limits>

namespace isortisearch{

// forwards
class image;
template<typename TIMG> class guiController;
template<typename TIMG> class guiControllerImpl;

// a base for searcherImpl and dynamicSearcherImpl

template<typename TIMG> class searchBase : public mutexHandler
{
public:
	searchBase();
	~searchBase();
	// target handling
	bool addTarget(const TIMG& image);
	void addTarget(const byteRGB& rgb);
	unsigned numTargetImages() const {return m_TargetImages.size();}
	unsigned numTargetColors() const {return m_TargetColors.size();}
	unsigned numTargets() const {return numTargetImages()+numTargetColors();}
	void clearTargetImages() {m_TargetImages.clear();}
	void clearTargetColors() {m_TargetColors.clear();}
	void clearTargets() {clearTargetImages(); clearTargetColors();}
	// callback handling
	void registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws);

	// the minimum distance to the image targets
	// NOTE i don't know why, but these have to be public, otherwise we cannot set a ptr to these
	// in the derived classes, altough these derive as protected...
	float minDistToTargetImages(const image* pImg,void *pData);
	// the minimum distance to the color targets
	float minDistToTargetColors(const image* pImg,void *pData);
	// the minimum distance to both color and image targets
	float minDistToTargets(const image* pImg,void *pData);

protected:
	// one of the following functions is called in the derived classes search()
	// member function when searching for colors, images or colors and images

	// the distance between two images
	// reentrant
	float distanceToImageTarget(const image* target,const image *test);
	// the distance to the target image and all color targets
	float distanceToTargetAndColors(const fixedArrayV5<char,HISTOSIZE>& data,const image *test);
	// this generates the combined histo data for all target colors and all target images
	void generateCombinedHistoData(std::vector<fixedArrayV5<char,HISTOSIZE> >& data);
	// the target images
#ifdef __GNUC__
	std::vector<TIMG> m_TargetImages;
#else
	std::vector<const TIMG> m_TargetImages;
#endif
	// the target colors
	std::vector<byteRGB> m_TargetColors;
	// callback stuff
	unsigned m_uCallbackIntervall;
	pxCallback m_pSearchAdvancedCallback;
	void* m_pSearchAdvancedUserData;
	bool m_bCallbackDraws;
	// used only by the dynamicSearcherImpl, currently
	// reset to false by addTarget()
	bool m_bDataForTargetsValid;
};

template <typename TIMG> class searcherImpl : public searchBase<TIMG>
{
public:
	searcherImpl();
	~searcherImpl();

	// note the white space between the closing template bracket '>'
	// gnu gcc may not compile w/o this
	bool setImages(std::vector<refPtr<TIMG> >* pImages);
	unsigned __int32 numImages() const {return ((m_pImages==NULL)?0:m_pImages->size());}
	std::vector<refPtr<TIMG> >& searchResults() {return m_SearchResults;}
	
	bool search(unsigned numResults,bool bSeparateThread/*,target_type type*/);

	void clearSearchResults() {m_SearchResults.clear();}

	bool attach(guiController<TIMG>* pController);
	
#ifdef __GNUC__
	template <typename UIMG> friend class guiControllerImpl;
private:
	std::vector<refPtr<TIMG> >* m_pImages;
	std::vector<refPtr<TIMG> > m_SearchResults;
	guiControllerImpl<TIMG>* m_pGuiControllerImpl;
#else
	template <typename TIMG> friend class guiControllerImpl;
private:
	std::vector<refPtr<TIMG>>* m_pImages;
	std::vector<refPtr<TIMG>> m_SearchResults;
	guiControllerImpl<TIMG>* m_pGuiControllerImpl;
#endif

};


template <typename TIMG> class dynamicSearcherImpl : public searchBase<TIMG>
{
public:
	dynamicSearcherImpl();
	~dynamicSearcherImpl();
	bool setResultSize(unsigned __int32 size);
	unsigned __int32 resultSize() const {return m_ResultSize;}
	// void setTargetType(target_type type) {m_TargetType=type;}
	// target_type targetType() const {return m_TargetType;}
	bool dynamicSearch(const refPtr<TIMG>& image);
#ifdef __GNUC__
	/*const*/ std::multimap<float,refPtr<TIMG> >& dynamicSearchResults() {return m_DynamicSearchResults;}
#else
	/*const*/ std::multimap<float,refPtr<TIMG>>& dynamicSearchResults() {return m_DynamicSearchResults;}
#endif

	void clear() {m_DynamicSearchResults.clear();m_NumInsertCalls=0;m_GreatestDist=std::numeric_limits<float>::max();}
	void resetCallbackCounter() {m_NumInsertCalls=0;}

	bool attach(guiController<TIMG>* pController);
#ifdef __GNUC__
	template <typename UIMG> friend class guiControllerImpl;
#else
	template <typename TIMG> friend class guiControllerImpl;
#endif
private:
	unsigned m_NumInsertCalls;
	std::multimap<float,refPtr<TIMG> > m_DynamicSearchResults;
	unsigned __int32 m_ResultSize;
	// target_type m_TargetType;
	float m_GreatestDist;
	guiControllerImpl<TIMG>* m_pGuiControllerImpl;
	// the combined feature data of the color targets
	// this will be calculated and used only when searching for colors only
	char m_ColorFeatureData[HISTOSIZE];
	// the combined histo data when searching for colors and images
	std::vector<fixedArrayV5<char,HISTOSIZE> > m_CombinedHistoData;

	float (searchBase<TIMG>::*m_pMinDistToTargets)(const image* pImg,void *pData);
	void *m_pData;

};

}
#endif