#if !defined(PXGUICONTROLLER_IMPL)
#define PXGUICONTROLLER_IMPL

#include "pxdefs.h"
//#include "pximage.h"
//#include "pxmutex.h"
#include <vector>
#include <map>

namespace isortisearch{

// forwards
class controllerImage;
class sortImage;
class image;
class mutex;

template <typename TIMG> class sorterImpl;
template <typename TIMG> class searcherImpl;

template <typename TIMG> class ISORT_API guiControllerImpl
{
public :
	guiControllerImpl();
	~guiControllerImpl() {;}
	static unsigned __int32 majorVersion() {return m_uMajorVersion;}
	static unsigned __int32 minorVersion() {return m_uMinorVersion;}
	// TODOV5: we may have to resort on canvas size change ....
	// note call this *before* init()
	bool setCanvasSize(__int32 x,__int32 y, bool bRedraw=true);
	void setImageSpacing(double val);
	void onMouseDown(__int32 xPos,__int32 yPos);
	void onMouseMove(__int32 xPos,__int32 yPos);
	refPtr<TIMG> onMouseUp(__int32 xPos,__int32 yPos,bool bRedraw,bool bReturnImage);
	void setClickSensitivity(unsigned __int32 num);
	unsigned __int32 clickSensitivity() const {return m_uClickSensitivity;}
	void onMouseLeave();
	void onZoomIn();
	void onZoomOut();
	double maxZoomFactor() const {return m_dMaxZoomFactor;}
	bool setMaxZoomFactor(double f);
	double zoomStep() const {return m_dZoomStep;}
	bool setZoomStep(double step);
	void onZoom(double factor);
	void reset(bool bRepaint=true);
	void resetZoom(bool bRepaint=true);
	bool registerPaintRequestCallback(pxCallback pCallBack,void *pUserData);
	refPtr<TIMG> getImageAt(__int32 x,__int32 y);
	bool setMode(controller_mode mode,bool bRepaint=true);
	int getIdealPyramidResultNumber(int desiredResultNumber);
	controller_mode mode() const {return m_ControllerMode;}

	// has to be accessed by controller, otherwise we have a separate sort() here which calls sorterImpl::sort()
	bool m_bResized;
	bool m_bResizedAndResorted;
	void updateImagePositions(bool bCallBack=true);


#ifdef __GNUC__
	// if we are attached to a sorter, it will set several of our private members
	template<typename UIMG> friend class sorterImpl;
	template<typename UIMG> friend class searcherImpl;
	template<typename UIMG> friend class dynamicSearcherImpl;
#else
	// if we are attached to a sorter, it will set several of our private members
	template<typename TIMG> friend class sorterImpl;
	template<typename TIMG> friend class searcherImpl;
	template<typename TIMG> friend class dynamicSearcherImpl;
#endif
private:
	pxCallback m_pPaintRequestCallBack;
	void* m_pPaintRequestUserData;

	unsigned __int32 m_CanvasXSize;
	unsigned __int32 m_CanvasYSize;
	double m_ThumbSpacing;
	__int32 m_MouseXPos;
	__int32 m_MouseYPos;
	__int32 m_MouseLastXPos;
	__int32 m_MouseLastYPos;
	
	__int32 m_MouseMoveX;
	__int32 m_MouseMoveY;
	__int32 m_MousePressXPos;
	__int32 m_MousePressYPos;
	double m_dZoomFactor;
	double m_dMaxZoomFactor;
	double m_dLastZoomFactor;
	double m_dZoomStep;
	bool m_bMouseDown;
	
	__int32 m_MinPosX;
	__int32 m_MinPosY;
	__int32 m_MaxPosX;
	__int32 m_MaxPosY;

	// pyramid
	double *m_pThumbBoxSizesX;
	double *m_pThumbBoxSizesY;
	int	   *m_pImgsPerRow;
	int		m_NumRows;
	
	const double m_dPyramidFactor;

	// private member functions

	void calculateListLayout(unsigned nThumbs);
	// note may throw std::bad_alloc
	void calculatePyramidLayout(unsigned nThumbs);
	void calculateDrawingLimits();
	void calculateDrawingPositionList(controllerImage& img, int pos); 
	void calculateDrawingPositionPyramid(controllerImage* pImg, int pos); 

	// called by updateImagePositions() if m_ControllerMode==LIST...
	void updateImagePositionsList(bool bCallBack);
	void updateImagePositionsPyramid(bool bCallBack);

	bool m_bFirstTimeColor;

	unsigned __int32 m_uClickSensitivity;

	controller_mode m_ControllerMode;

	// these are set and controlled by sorterImpl
#ifdef __GNUC__
	std::vector<refPtr<TIMG> >** m_pSorterImages;
#else
	std::vector<refPtr<TIMG>>** m_pSorterImages;
#endif
	mutex** m_pSorterMutex;
	unsigned m_MapPlacesX;
	unsigned m_MapPlacesY;
	pxCallback m_pSizeChangedCallback;
	void *m_pSizeChangedUserData;
#ifdef __GNUC__
	// these are set and controlled by searcherImpl
	std::vector<refPtr<TIMG> >** m_pSearcherImages;
	std::vector<refPtr<TIMG> >* m_pSearcherResults;
#else
	// these are set and controlled by searcherImpl
	std::vector<refPtr<TIMG>>** m_pSearcherImages;
	std::vector<refPtr<TIMG>>* m_pSearcherResults;
#endif
	mutex** m_pSearcherMutex;

	searcherImpl<TIMG>* m_pSearcher;

	// the (sorted) images to control
	std::vector<TIMG>* m_pSortImages;

	unsigned __int32 m_MapPlacesListX;
	unsigned __int32 m_MapPlacesListY;

	double m_ScaledThumbBoxSizeX;
	double m_ScaledThumbBoxSizeY;

	static const unsigned __int32 m_uMajorVersion;
	static const unsigned __int32 m_uMinorVersion;

	//bool m_bSearchResultsVisibleOnly;
	void hideAllSearchImages();

	bool m_bAttachedToSearcher;
	bool m_bAttachedToSorter;

	bool m_bAttachedToDynamicSearcher;
	std::multimap<float,refPtr<TIMG> >* m_pDynamicSearcherResults;
	mutex** m_pDynamicSearcherMutex;
	// a NULL refPtr to be returned by onMouseMove() and getImageAt()
	static refPtr<TIMG> m_RefPtrNull;

};

// full specialisations for sortImage and image
// these simply do nothing
template <> class ISORT_API guiControllerImpl<isortisearch::sortImage>
{
public :
	guiControllerImpl():m_CanvasXSize(0),m_CanvasYSize(0) {;}
	~guiControllerImpl() {;}
	void setMode(controller_mode mode,bool bRepaint=true) {;}
	void reset(bool bRepaint=true){;}
	void resetZoom(bool bRepaint=true){;}
	void updateImagePositions(bool bCallBack=true){;}
	unsigned __int32 m_CanvasXSize;
	unsigned __int32 m_CanvasYSize;
	bool m_bResized;
	bool m_bResizedAndResorted;
#ifdef __GNUC__
	std::vector<refPtr<sortImage> >** m_pSorterImages;
#else
	std::vector<refPtr<sortImage>>** m_pSorterImages;
#endif
	mutex** m_pSorterMutex;
	unsigned m_MapPlacesX;
	unsigned m_MapPlacesY;
	pxCallback m_pSizeChangedCallback;
	void *m_pSizeChangedUserData;
#ifdef __GNUC__
	std::vector<refPtr<sortImage> >** m_pSearcherImages;
	std::vector<refPtr<sortImage> >* m_pSearcherResults;
#else
	std::vector<refPtr<sortImage>>** m_pSearcherImages;
	std::vector<refPtr<sortImage>>* m_pSearcherResults;
#endif
	mutex** m_pSearcherMutex;
	bool m_bAttachedToSearcher;
	bool m_bAttachedToSorter;
	bool m_bAttachedToDynamicSearcher;
#ifdef __GNUC__
	std::multimap<float,refPtr<sortImage> > *m_pDynamicSearcherResults;
#else
	std::multimap<float,refPtr<sortImage>> *m_pDynamicSearcherResults;
#endif
	mutex** m_pDynamicSearcherMutex;
};
template <> class ISORT_API guiControllerImpl<isortisearch::image>
{
public :
	guiControllerImpl():m_CanvasXSize(0),m_CanvasYSize(0) {;}
	~guiControllerImpl() {;}
	void setMode(controller_mode mode,bool bRepaint=true) {;}
	void reset(bool bRepaint=true){;}
	void resetZoom(bool bRepaint=true){;}
	void updateImagePositions(bool bCallBack=true){;}
	unsigned __int32 m_CanvasXSize;
	unsigned __int32 m_CanvasYSize;
	bool m_bResized;
	bool m_bResizedAndResorted;
#ifdef __GNUC__
	std::vector<refPtr<image> >** m_pSorterImages;
#else
	std::vector<refPtr<image>>** m_pSorterImages;	
#endif
	mutex** m_pSorterMutex;
	unsigned m_MapPlacesX;
	unsigned m_MapPlacesY;
	pxCallback m_pSizeChangedCallback;
	void *m_pSizeChangedUserData;
#ifdef __GNUC__
	std::vector<refPtr<image> >** m_pSearcherImages;
	std::vector<refPtr<image> >* m_pSearcherResults;
#else
	std::vector<refPtr<image>>** m_pSearcherImages;
	std::vector<refPtr<image>>* m_pSearcherResults;
#endif
	mutex** m_pSearcherMutex;
	bool m_bAttachedToSearcher;
	bool m_bAttachedToSorter;
	bool m_bAttachedToDynamicSearcher;
#ifdef __GNUC__
	std::multimap<float,refPtr<image> > *m_pDynamicSearcherResults;
#else
	std::multimap<float,refPtr<image>> *m_pDynamicSearcherResults;
#endif
	mutex** m_pDynamicSearcherMutex;
};
}

#endif