#if !defined(PXSORTER_H)
#define PXSORTER_H

#include "pxdefs.h"
#include <vector>

namespace isortisearch {

// forwards
class mutex;
template <typename TIMG> class sorterImpl;
template <typename TIMG> class guiController;

/*!
The sorter template class

This template class sorts images onto a 2D map.

A client has to call at least

- setImages()
- sort()

to sort images.

A client may call attach() to attach a guiController which helps drawing the sorted images. 
Without a guiController, the client has to draw the images on behalf of sortImage::mapXPos() and sortImage::mapYPos()
as well as aspectRatio(), mapPlacesX() and mapPlacesY().

Note that there are two valid specializations for the sorter template class:

isortisearch::sorter<isortisearch::sortImage> can be used without a guiController attached.
The client has to pass a std::vector<isortisearch::refPtr<isortisearch::sortImage>> to setImages() in this case.

isortisearch::sorter<isortisearch::controllerImage> has to be used with a guiController attached.
The client has to pass a std::vector<isortisearch::refPtr<isortisearch::controllerImage>> to setImages() in this case.
*/
template <typename TIMG> class ISORT_API sorter
{
	public:
	/*!
	Constructor
	*/
	sorter();
	/*!
	Destructor
	*/
	~sorter();
	/*!
	Returns the current major version number of this class.

	Note a major version change indicates API changes, so that a client application has to be changed.
	*/
	static unsigned __int32 majorVersion();

	/*!
	Returns the current minor version number of this class.

	Note a major version indicates that the API has not changed.
	*/
	static unsigned __int32 minorVersion();

	/*!
	A client has to call this prior to setImages() to set the aspect ratio (width to height) of the map.

	The default aspect ratio is 1, i.e. the map is quadratic.

	Returns false if ratio <= 0, in this case the aspect ratio is not changed

	Note if a client attaches a guiController, the client does not need to call this, 
	this is called by guiController::setCanvasSize().
	*/
	bool setAspectRatio(double ratio);

	/*!
	returns the aspect ratio

	In general, a client does not have to call this.
	*/
	double aspectRatio() const;

	/*!
	A client has to call this before calling sort().

	pImages is the adress of a std::vector<isortisearch::refPtr<sortImage>> or of a std::vector<isortisearch::refPtr<controllerImage>>, these are the images to sort().
	The first form has to be used if the client uses a sorter<sortImage>, the second form if the client uses a sorter<controllerImage>.

	Returns false if pImages==NULL. Note pImages->size()==0 is allowed, i.e. images can be added before sort() is called.
	
	Note that std::vector<> relocates if elements are added to the vector and it's capacity is too small, see
	the std::vector<> reference. The client has to make sure that this does not happen to the vector passed here while it is used by sort() or an attached gui controller.
	*/
#ifdef __GNUC__
	bool setImages(std::vector<refPtr<TIMG> >* pImages);
#else
	bool setImages(std::vector<refPtr<TIMG>>* pImages);
#endif
	
	/*!
	Returns the number of images to sort
	*/
	unsigned __int32 numImages() const;

	/*!
	Sets the dim factor, a double which describes the fraction of number of places in the map to
	number of images. If this is bigger than one, there are places in the map 
	which stay 'free'. In general, a certain amount of 'free' places lead to better sort results. 
	The default value (1.2) is a good value. 

	Note it is possible to get a sorted map without any free places, in this case both the dimFactor() 
	and the aspectRatio() have to be 1.0

	Returns false if dimFactor < 1.0 or dimFactor > 2.0
	*/
	bool setDimFactor(double dimFactor);
	
	/*!
	returns the dim factor, see setDimFactor()

	In general, a client does not have to call this
	*/
	double dimFactor() const;

	/*!
	Returns the number of map places used in the sort in x direction of the map.
	
	The product mapPlacesX() * mapPlacesY() gives the total number of places on the map.
	This may be bigger than numImages() in general, because there may be 'free' places on the 
	map (see setDimFactor()).

	Note this is valid after sort().

	If attach() was called, the client does not need to call this.
	*/
	unsigned __int32 mapPlacesX() const;

	/*!
	Returns the number of map places used in the sort in y direction of the map

	The product mapPlacesX() * mapPlacesY() gives the total number of places on the map.
	This may be bigger than numImages() in general, because there may be 'free' places on the 
	map (see setDimFactor()).

	Note this is valid after sort().

	If attach() was called, the client does not need to call this.
	*/
	unsigned __int32 mapPlacesY() const;

	/*!
	Attaches a guiController to the %sorter

	Note this *only* works with the sorter<controllerImage> template specialization.

	Returns false if pController is NULL
	*/
	bool attach(guiController<TIMG>* pController);

	/*!
	Sets an instance of the clients derived isortisearch::mutex class to the %sorter.

	The client has to call this only if:
	
	- sort() is to be started as a separate thread 
	
	and 

	- registerSortAdvancedCallback() is called to paint the sort advance. In this case, 
	the clients draw routine has to lock/unlock the mutex passed here when it accesses the images.

	or

	-  the client want's to call abort() to be able to abort the sort.

	Returns false if NULL passed. 

	Note the sorter enables and disables the mutex based on the bool parameter passed to sort() by default.
	See enableAutoMutex().
	*/
	bool setMutex(isortisearch::mutex* pMutex);

	/*!
	This locks the mutex.

	Note a client should always call this convenience function to lock the mutex.

	Note in general, a client only has to call this if sort() runs in a separate thread and
	registerSortAdvancedCallback() was called (with last parameter==true) to draw the images
	while they are sorted. In this case, lock() has to be called in the callback provided by the client 
	which draws the images before accessing the images.
	*/
	void lock();

	/*!
	This unlocks the mutex.

	Note a client should always call this convenience function to unlock the mutex.

	Note in general, a client only has to call this if sort() runs in a separate thread and
	registerSortAdvancedCallback() was called (with last parameter==true) to draw the images
	while they are sorted. In this case, lock() has to be called in the callback provided by the client
	which draws the images after accessing the images.
	*/
	void unlock();

	/*!
	This enables the mutex.

	In general, a client does not have to call this.
	*/
	void enableMutex();

	/*!
	This disables the mutex. If the mutex is disabled, calling lock() and unlock() does nothing.

	In general, a client does not have to call this.
	*/
	void disableMutex();

	/*!
	This enables or disables the automatic mutex handling.

	If enabled (which is the default), the mutex passed to setMutex()
	is disabled automatically when sort(false) is called and enabled automatically
	when sort(true) is called.

	If disabled, the client has to call enableMutex() and disableMutex() on it's own.
	*/
	void enableAutoMutex(bool bEnable);

	/*!
	The client has to call this to sort the images set by setImages().

	If the client has called registerSortAdvancedCallback()
	before calling sort() the registered callback will be called when the sort advances.

	Returns SORT_COMPLETED if the sort completes.

	Returns SORT_ABORTED if the sort was aborted by abort().

	Returns SORT_ERROR on error, in this case setImages() was not called or numImages()==0.

	The client has to pass bSeparateThread==true if it is calling sort() from a separate thread.

	Note if called with bSeparateThread==true, this calls enableMutex() if enableAutoMutex(true) (the default) and setMutex() was called before.
	Thus, internal calls to lock() and unlock() will really lock/unlock the mutex.

	Note if called with bSeparateThread==false, this calls disable() if enableAutoMutex(true) (the default) and setMutex() was called before.
	Thus, internal calls to lock() and unlock() do nothing.
	*/
	sort_result sort(bool bSeparateThread=false);

	/*!
	The client may register a callback to be called when sort() is advanced.

	The registered callback will be called every interval sort steps.
	
	The sorting will take 25 iterations. 
	The first iterations will need more time than the final iterations. 

	If bDraws is false, the client may only implement a simple callback like a wait cursor or a progress bar.

	If the client draws the current sort step in it's GUI, it has to pass bDraws==true.
	
	Note depending on the speed of the machine and the complexity of the registered callback, 
	the application may slow down if interval is chosen too low.

	Note there is no error check. Passing NULL for pCallBack or 0 for interval simply does not register the callback.

	Note that in case sort() runs in a separate thread, the callback also runs in that thread. It's the clients
	responsibility to set up a safe inter thread communication with it's main GUI thread from the callback.
	*/
	void registerSortAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws);

	/*!
	The client may call this to abort sort().

	Note this will only have an effect if sort() is running in a separate thread.

	The results in each isortisearch::sortImage or isortisearch::controllerImage will be the result of the last sort step done.

	Note this calls isortisearch::mutex::lock() and isortisearch::mutex::unlock(), if setMutex() was called.
	*/
	void abort();

private:
	/*!
	the implementation pointer
	*/
	sorterImpl<TIMG> *m_pImpl;
	/*!
	Note: The copy constructor is private
	*/
	sorter(const sorter& rOther);
	/*!
	Note: The assignment operator is private
	*/
	sorter& operator=(const sorter& rOther);

};
}

#endif
