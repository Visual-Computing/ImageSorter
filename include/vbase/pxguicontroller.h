#if !defined(PXGUICONTROLLER_H)
#define PXGUICONTROLLER_H

#include "pxdefs.h"
//#include "pximage.h"
#include <vector>

namespace isortisearch{

// forwards
class controllerImage;
class sortImage;
class image;
template <typename TIMG> class guiControllerImpl;

/*!
The guiController template class.

A client of the isortisearch libraries may attach an instance of guiController to isortisearch::sorter, isortisearch::searcher and/or isortisearch::dynamicSearcher.
The client can pass mouse and zoom events to the guiController, which in turn calculates the drawing positions of each controllerImage set to the sorter, searcher or dynamicSearcher.
The guiController instance does not draw any of the images, since this is platform dependant. However, the client can probe
the controllerImage::upperLeftX(),controllerImage::upperLeftY(),controllerImage::lowerRightX() and controllerImage::lowerRightY() members
to draw the image at it's position.

Note that although this is a class template, the only specialisation which is usefull is

	- template class guiController<isortisearch::controllerImage>;

*/
template<typename TIMG> class ISORT_API guiController
{
public :
	/*!
	Constructor

	note throws std::bad_alloc if allocation fails
	*/
	guiController();
	/*!
	Destructor
	*/
	~guiController();
	/*!
	Returns the current major version number of this class.

	This may increase in future releases.
	*/
	static unsigned __int32 majorVersion();

	/*!
	Returns the current minor version number of this class.

	This may increase in future releases.
	*/
	static unsigned __int32 minorVersion();

	/*!
	Sets the size of the clients drawing canvas in pixels.

	The client has to call this on startup (with bRedraw==false) and whenever the size of it's drawing 
	canvas changes.

	If bRedraw==true, the callback registered by registerPaintRequestCallback() is called.

	Note: The upper left corner of the canvas has coordinate (0,0), the lower right has (x-1,y-1)

	Returns false if x<=0 or y<=0
	*/
	bool setCanvasSize(__int32 x,__int32 y, bool bRedraw=true);

	/*!
	The client may call this to set the spacing between images 
	as they will be displayed by the client.

	val has to be a value in the range of [0.0,0.5], other values will be clamped silently
	to these values.

	This value will be used as a relative gap between images. 
	A value of 0.1 will result in a gap of 10% of the image width or height (whatever is bigger) between images.

	Note the default value is 0.1, which is ok in the most cases.
	*/
	void setImageSpacing(double val);

	/*!
	The client has to call this on a (left) mouse button down event in it's canvas. 
	xPos and yPos are the position in the callers canvas.

	See onMouseMove() for more information.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note: xPos and yPos have to be positive. Negative Values will be clipped to 0.

	Note: 'mouse' and '(left) mouse button' are meant as an abstraction here for any pointing device.
	*/
	void onMouseDown(__int32 xPos,__int32 yPos);

	/*!
	The client has to call this on a mouse move event in it's canvas. 
	xPos and yPos are the position in the callers canvas.

	Note there are two different situations when this is called:

	1. The (left) mouse button is not down (there has not been an initial onMouseDown() call)

	Nothing happens, but isortisearch::guiController keeps track of the mouse position, which is imortant for onZoomIn(), onZoomOut() and onZoom(). 

	2. The (left) mouse button is down (there has been an initial onMouseDown() call)

	The images will be dragged (with wrap around in case of mode()==SORTED).

	The callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note xPos and yPos have to be positive. Negative Values will be clipped to 0.

	Note depending on the amount of images, the machine and the actual drawing code of the client,
	dragging the images may be slow. Therefore, the client might not pass in every mouse move event.
	However, it should call onMouseUp() with the bRedraw parameter set to true in the end.

	Note: 'mouse' and '(left) mouse button' are meant as an abstraction here for any pointing device.
	*/
	void onMouseMove(__int32 xPos,__int32 yPos);

	/*!
	The client has to call this on a (left) mouse button up event in it's canvas. 
	xPos and yPos are the position in the callers canvas.

	If bReturnImage==true and the (manhattan length) distance between the initial onMouseDown() call and this
	onMouseUp() call ist smaller than clickSensitivity(), a refPtr<TIMG> to the current position is 
	returned, otherwise a refPtr<TIMG> with refPtr<T>::isNull().

	If bRedraw==true, the callback registered by registerPaintRequestCallback() is called.
	Normally, this is not needed. However, if the client does not pass every mouse move event,
	because drawing is too slow, redrawing is needed here.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note xPos and yPos have to be positive. Negative Values will be clipped to 0.

	Note: 'mouse' and '(left) mouse button' are meant as an abstraction here for any pointing device.
	*/
	refPtr<TIMG> onMouseUp(__int32 xPos,__int32 yPos,bool bRedraw=false,bool bReturnImage=false);

	/*!
	The client has to call this if the mouse leaves the clients canvas.

	This does nothing if the (left) mouse button is not down (there has not been an initial onMouseDown() call).

	If the mouse button is down, onMouseUp(x,y) will be called internally
	where x nd y are the last coordinates passed into onMouseMove().

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note: 'mouse' and '(left) mouse button' are meant as an abstraction here for any pointing device.
	*/
	void onMouseLeave();

	/*!
	Sets the click sensitivity. The default is 5.

	See onMouseUp()
	*/
	void setClickSensitivity(unsigned __int32 num);

	/*!
	Returns the click sensitivity.

	See onMouseUp()
	*/
	unsigned __int32 clickSensitivity() const;

	/*!
	The client has to call this on a zoom in event (e.g. a mouse wheel ecent) in it's canvas.

	The zoom factor will be multiplicated by zoomStep() and clipped to maxZoomFactor().

	If the zoom factor has changed, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().
	*/
	void onZoomIn();

	/*!
	The client has to call this on a zoom out event (e.g. a mouse wheel ecent) in it's canvas.

	The zoom factor will be divided by zoomStep() until it is 1.0

	If the zoom factor has changed, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().
	*/
	void onZoomOut();

	/*!
	Returns the zoom step.

	see onZoomIn() and onZoomOut()
	*/
	double zoomStep() const;

	/*!
	Sets the zoom step. The default is 1.1 which is ok.

	Returns false and changes nothing if step<=1

	see onZoomIn() and onZoomOut()
	*/
	bool setZoomStep(double step);

	/*!
	Returns the maximum zoom factor
	*/
	double maxZoomFactor() const;

	/*!
	Sets the maximum zoom factor

	The default is 200.0

	Returns false if the value passed is smaller than 1.0, in this case the maximum zoom factor is unchanged
	*/
	bool setMaxZoomFactor(double f);

	/*!
	The client may call this to set the zoom factor directly

	The zoom factor will be set to factor and clipped to [1.0,maxZoomFactor()]

	If the zoom factor has changed, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().
	*/
	void onZoom(double factor);

	/*!
	The client may call this to reset both drag and zoom.

	If bRepaint is true, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note that isortisearch::sorter::sort() and isortisearch::searcher::search() call this (if a guiConroller is attached)
	with bRedraw==false, so the zoom an drag are reset automatically.
	*/
	void reset(bool bRepaint=true);

	/*!
	The client may call this to reset the zoom only.

	If bRepaint is true, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().
	*/
	void resetZoom(bool bRepaint=true);

	/*!
	The client has to register a static callback which is called every time the clients canvas
	has to be repainted.

	Returns false if pCallBack is NULL.
	*/
	bool registerPaintRequestCallback(pxCallback pCallBack,void *pUserData);

	/*!
	Returns a refPtr<TIMG> to the image at position (x,y) or a refPtr<TIMG> whith refPtr<T>::isNull() if there is no image
	at this position.

	Note x and y have to be positive. Negative valus are clipped to 0.
	*/
	refPtr<TIMG> getImageAt(__int32 x,__int32 y);

	/*!
	Sets the controller_mode mode

	- SORTED: the %guiController calculates the position of the images sorted by isortisearch::sorter::sort() (the default)

	- SORTER_INPUT: the %guiController calculates the position of the images as they are stored in the vector<> passed to isortisearch::sorter::setImages() (row by row)

	- LIST_SEARCH_RESULTS: the %guiController calculates the position of the images as they are stored in the vector<> returned by isortisearch::searcher::results() (row by row, most similar image in the top row in the leftmost position)

	- PYRAMID_SEARCH_RESULTS: the %guiController calculates the position of the images as they are stored in the vector<> returned by isortisearch::searcher::results() (row by row, most similar image in the top row in the leftmost position, images more similar to the targets are bigger than less similar images)

	- SEARCHER_INPUT: the %guiController calculates the position of the images as they are stored in the vector<> passed to isortisearch::searcher::setImages() (row by row)

	- LIST_DYNAMIC_SEARCH_RESULTS: the %guiController calculates the position of the images as they are stored in the multimap<> returned by isortisearch::dynamicSearcher::dynamicSearchResults() (row by row, most similar image in the top row in the leftmost position)
	
	- PYRAMID_DYNAMIC_SEARCH_RESULTS: the %guiController calculates the position of the images as they are stored in the multimap<> returned by isortisearch::dynamicSearcher::dynamicSearchResults() (row by row, most similar image in the top row in the leftmost position, images more similar to the targets are bigger than less similar images)

	If bRepaint is true, the callback registered by registerPaintRequestCallback() is called.

	Note that the client may not call this during isortisearch::sorter::sort(), isortisearch::searcher::search() or isortisearch::dynamicSearcher::dynamicSearch().

	Note that isortisearch::sorter::sort() and isortisearch::searcher::search() call this (if a guiConroller is attached)
	with bRedraw==false with either SORTED or LIST_SEARCH_RESULTS.

	Returns false if mode is SORTER_INPUT or SORTED and no sorter is attached.

	Returns false if mode is LIST_SEARCH_RESULTS or PYRAMID_SEARCH_RESULTS and no searcher is attached.

	Returns false if mode is LIST_DYNAMIC_SEARCH_RESULTS or PYRAMID_DYNAMIC_SEARCH_RESULTS and no dynamicSearcher is attached.
	*/
	bool setMode(controller_mode mode,bool bRepaint=true);

	/*!
	Returns the current controller_mode
	*/
	controller_mode mode() const;

	/*!
	Calculates the %image positions for the current mode().

	In general, a client does not need to call this, because either isortisearch::sorter, isortisearch::searcher and isortisearch::dynamicSearcher
	as well as onMouseMove(), onZoomIn() and onZoomOut() call this. However, a client may e.g. use the mode() SORTER_INPUT to display all 
	images currently loaded (probably while loading) without any mouse or zoom inputs and thus has to call this. 

	If bRepaint is true, the callback registered by registerPaintRequestCallback() is called.
	*/
	void calculate(bool bRepaint=false);

#ifdef __GNUC__
	template<typename UIMG> friend class sorterImpl;
	template<typename UIMG> friend class searcherImpl;
	template<typename UIMG> friend class dynamicSearcherImpl;
#else
	template<typename TIMG> friend class sorterImpl;
	template<typename TIMG> friend class searcherImpl;
	template<typename TIMG> friend class dynamicSearcherImpl;
#endif

private:
	/*!
	Note: The copy constructor is private.
	*/
	guiController(const guiController& rOther);
	/*!
	Note: The assignment operator is private.
	*/
	guiController& operator=(const guiController& rOther);
	/*!
	The implementation pointer
	*/
	guiControllerImpl<TIMG> *m_pImpl;
};

// full specialisations for sortImage and sortImage*
// these simply do nothing

/*!
\cond
*/

template<> class ISORT_API guiController<isortisearch::sortImage>
{
public :
	guiController():m_pImpl(NULL) {;}
	~guiController() {;}
	guiControllerImpl<isortisearch::sortImage>* m_pImpl;
};

template<> class ISORT_API guiController<isortisearch::image>
{
public :
	guiController():m_pImpl(NULL) {;}
	~guiController() {;}
	guiControllerImpl<isortisearch::image>* m_pImpl;
};

/*!
\endcond
*/

}


#endif