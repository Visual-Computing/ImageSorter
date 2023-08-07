#if !defined(SORTER_IMPL_H)
#define SORTER_IMPL_H

#include "pxdefs.h"
#include "pximpldefs.h"
#include "pxmutexhandler.h"
#include <vector>

namespace isortisearch {

/*
a little helper class
we need this if we run sort in a separate thread
*/
class node{
public:
	node() : m_uMapXPos(0),m_uMapYPos(0){;}
	~node(){;}
	unsigned m_uMapXPos;
	unsigned m_uMapYPos;
	char m_FeatureData[FEATURESIZE];
	const char *featureData() const {return m_FeatureData;}
	void setMapXPos(unsigned __int32 x) {m_uMapXPos=x;}
	void setMapYPos(unsigned __int32 y) {m_uMapYPos=y;}
	static unsigned __int32 featureDataByteSize() {return FEATURESIZE*sizeof(char);}
	static unsigned __int32 featureDataSize() {return FEATURESIZE;}
};

// forwards
template<typename TIMG> class guiController;
template<typename TIMG> class guiControllerImpl;

template <typename TIMG> class ISORT_API sorterImpl : public mutexHandler
{
	public:
	sorterImpl();
	~sorterImpl(){;}
	static unsigned __int32 majorVersion() {return m_uMajorVersion;}
	static unsigned __int32 minorVersion() {return m_uMinorVersion;}
	// note call setAspectRatio() *before* this
#ifdef __GNUC__
	bool setImages(std::vector<refPtr<TIMG> >* pImages);
#else
	bool setImages(std::vector<refPtr<TIMG>>* pImages);
#endif
	bool setDimFactor(double dimfactor);
	unsigned __int32 mapPlacesX() const {return m_MapPlacesX;}
	unsigned __int32 mapPlacesY() const {return m_MapPlacesY;}
	double aspectRatio() const {return m_dAspectRatio;}
	double dimFactor() const {return m_dDimFactor;}
	// note pass true, if this runs in a separate thread...
	sort_result sort(bool bSeparateThread=false);
	void registerSortAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws);
	void abort();
	void setAspectRatio(double ratio) {m_dAspectRatio=ratio;}
	unsigned __int32 numImages() const {return m_pSortImages?m_pSortImages->size():0;}
	bool attach(guiController<TIMG>* pController);

#ifdef __GNUC__
	template <typename UIMG> friend class sorter;
	template <typename UIMG> friend class guiControllerImpl;
#else
	template <typename TIMG> friend class sorter;
	template <typename TIMG> friend class guiControllerImpl;
#endif
protected:
	// a special variant of sort() designed to
	// run in a separate thread
	sort_result sortMT();

	// the images to sort
#ifdef __GNUC__
	std::vector<refPtr<TIMG> >* m_pSortImages;
#else
	std::vector<refPtr<TIMG>>* m_pSortImages;
#endif
	// if we sort in a separate thread, we need another vector:
	std::vector<isortisearch::node> m_Nodes;

	// the map places
	unsigned __int32 m_MapPlacesX;
	unsigned __int32 m_MapPlacesY;

	double m_dAspectRatio;
	double m_dDimFactor;
	
	static const unsigned __int32 m_uMajorVersion;
	static const unsigned __int32 m_uMinorVersion;

	// a flag to abort
	bool m_bAbort;

	// the callback to be called when the sort advances
	pxCallback m_pSortAdvancedCallBack;
	unsigned __int32 m_uCallBackInterval;
	void* m_pSortAdvancedUserData;
	bool m_bCallBackDraws;

	// callbacks called by the som
	static void updateDataSetStaticCallBack(sorterImpl<TIMG> *pThis);
	void updateDataSetCallBack();
	void updateDataSet();
	static void updateImagePositionsStaticCallBack(sorterImpl<TIMG> *pThis);
	void updateImagePositionsCallBack();

	bool calcMapPlaces();
	guiControllerImpl<TIMG>* m_pGuiControllerImpl;

	// a callback called by the attached gui controller
	static void sizeChangedStaticCallback(sorterImpl<TIMG> *pThis);
	void sizeChanged();
private:
	sorterImpl(const sorterImpl& rOther){;}
	sorterImpl& operator=(const sorterImpl& rOther){return *this;}

};

}


#endif