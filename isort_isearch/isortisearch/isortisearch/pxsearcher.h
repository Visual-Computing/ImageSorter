#if !defined(SEARCHER_H)
#define SEARCHER_H

#include "pxdefs.h"
#include <map>
#include <vector>

namespace isortisearch {

// forwards
template<typename TIMG> class searcherImpl;
template<typename TIMG> class dynamicSearcherImpl;
template<typename TIMG> class guiController;
class mutex;

/*!
isortisearch::searcher is used to search in a given set of images for images similar to one or more targets. The image set which 
is searched is hold in memory. This search is very fast.

A client has to call at least 

- isortisearch::searcher::setImages()
- isortisearch::searcher::addTarget()
- isortisearch::searcher::search()

The results will be available in isortisearch::searcher::results().

Note that the following template specializations can be used:

- isortisearch::searcher<image>
- isortisearch::searcher<sortImage>
- isortisearch::searcher<controllerImage>

If a client wants to use both isortisearch::searcher and isortisearch::sorter on the same %image set, it has to use the second form.
If a client wants to attach() a isortisearch::guiController to the searcher (wether or not it uses isortisearch::sorter too), it has to use the third form.
*/

template<typename TIMG> class ISORT_API searcher
{
public:
	/*!
	Constructor
	*/
	searcher();
	/*!
	Destructor
	*/
	~searcher();
	/*!
	A client has to call this before calling search().

	pImages is the adress of a std::vector<isortisearch::refPtr<image>>, std::vector<isortisearch::refPtr<sorterImage>> or std::vector<isortisearch::refPtr<controllerImage>>, 
	these are the images to search for images similar to the targets.

	The first form has to be used if the client uses a searcher<image>, the second form if the client uses a searcher<sorterImage>, the third form if the client uses a searcher<controllerImage>.

	Returns false if pImages==NULL. Note pImages->size()==0 is allowed, i.e. images can be added before sort() is called.
	
	Note that std::vector<> relocates if elements are added to the vector and it's capacity is too small, see
	the std::vector<> reference. The client has to make sure that this does not happen to the vector passed here while it is used by search() or an attached gui controller.
	*/
#ifdef __GNUC__
	bool setImages(std::vector<refPtr<TIMG> >* pImages);
#else
	bool setImages(std::vector<refPtr<TIMG>>* pImages);
#endif
	/*!
	Returns the number of images to search
	*/
	unsigned __int32 numImages() const;
	/*!
	Sets an instance of the clients derived isortisearch::mutex class to the searcher.

	The client has to call this only if:
	
	- search() is to be started as a separate thread 
	
	and 

	- registerSearchAdvancedCallback() is called to paint the search advance. In this case, 
	the clients draw routine has to lock/unlock the mutex passed here when it accesses the images.

	Note the searcher enables and disables the mutex based on the bool parameter passed to search() by default. See enableAutoMutex(). 	
	*/
	bool setMutex(isortisearch::mutex* pMutex);

	/*!
	This locks the mutex.

	Note a client should always call this convenience function to lock the mutex.

	Note in general, a client only has to call this if search() runs in a separate thread and
	registerSearchAdvancedCallback() was called (with last parameter==true) to draw the results
	while they are searched. In this case, lock() has to be called in the callback provided by the client 
	which draws the images before accessing the images.
	*/
	void lock();

	/*!
	This unlocks the mutex.

	Note a client should always call this convenience function to unlock the mutex.

	Note in general, a client only has to call this if search() runs in a separate thread and
	registerSearchAdvancedCallback() was called (with last parameter==true) to draw the results
	while they are searched. In this case, lock() has to be called in the callback provided by the client 
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
	is disabled automatically when search(false) is called and enabled automatically
	when search(true) is called.

	If disabled, the client has to call enableMutex() and disableMutex() on it's own.
	*/
	void enableAutoMutex(bool bEnable);

	/*!
	Adds a target image

	Returns false if !img.featureDataValid(), in this case the target is not added.
	*/
	bool addTarget(const TIMG& img);
	/*!
	Adds a target rgb color
	*/
	void addTarget(unsigned char r,unsigned char g, unsigned char b);
	/*!
	Returns the total number of targets
	*/
	unsigned numTargets() const;
	/*!
	Returns the number of target iamges
	*/
	unsigned numTargetImages() const;
	/*!
	Returns the number of target colors
	*/
	unsigned numTargetColors() const;
	/*!
	Clears the results.

	Note the size of results() will be zero after this.
	*/
	void clearResults();
	/*!
	Clears all targets.
	*/
	void clearTargets();
	/*!
	Clears all target images.
	*/
	void clearTargetImages();
	/*!
	Clears all target colors.
	*/
	void clearTargetColors();
	/*!
	Attaches a guiController to the %searcher

	Note this *only* works with the searcher<controllerImage> template specialization.

	Returns false if pController is NULL
	*/
	bool attach(guiController<TIMG>* pController);
	/*!
	The client has to call this to search the images set by setImages() for images similar to the target image(s) added by addTarget().

	numResults is the number of desired results.

	The results are delivered in results().

	If the client has called registerSearchAdvancedCallback()
	before calling search() the registered callback will be called when the search advances.

	Returns false on error, in this case setImages() was not called, numResults==0 or numTargets()==0 or numImages()==0;

	The client has to pass bSeparateThread==true if it is calling search() from a separate thread.

	Note if called with bSeparateThread==true, this calls enableMutex() if enableAutoMutex(true) (the default) and setMutex() was called before.
	Thus, internal calls to lock() and unlock() will really lock/unlock the mutex.

	Note if called with bSeparateThread==false, this calls disableMutex() if enableAutoMutex(true) (the default) and setMutex() was called before.
	Thus, internal calls to lock() and unlock() do nothing.

	Note that search() is really fast, so multithreading is not required in general.
	*/
	bool search(unsigned numResults,bool bSeparateThread);

	/*!
	This returns the results of search().

	The std::vector<> returned contains the results, the most similar image is the first one, the least similar image is the last one. 
	*/
#ifdef __GNUC__
	std::vector<refPtr<TIMG> >& results();
#else
	std::vector<refPtr<TIMG>>& results();
#endif

	/*!
	The client may register a callback to be called when search() is advanced.

	The registered callback will be called every interval search steps.
	
	If bDraws is false, the client may only implement a simple callback like a wait cursor or a progress bar.

	If the client draws the current search step in it's GUI, it has to pass bDraws==true.
	
	Note depending on the speed of the machine and the complexity of the registered callback, 
	the application may slow down if interval is chosen too low.

	Note there is no error check. Passing NULL for pCallBack or 0 for interval simply does not register the callback.

	Note that in case search() runs in a separate thread, the callback also runs in that thread. It's the clients
	responsibility to set up a safe inter thread communication with it's main GUI thread from the callback.

	Note that search() is really fast, so drawing the search steps while searching is not really needed.
	*/
	void registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws);

private:
	/*!
	the implementation pointer
	*/
	searcherImpl<TIMG> *m_pImpl;
	/*!
	Note: The copy constructor is private
	*/
	searcher(const searcher& rOther);
	/*!
	Note: The assignment operator is private
	*/
	searcher& operator=(const searcher& rOther);
};

/*!
isortisearch::dyamicSearcher() is used to search for images similar to the targets while loading the images, e.g. from a disk or from the net.

A client has to call at least 

- isortisearch::dynamicSearcher::setResultSize()
- isortisearch::dynamicSearcher::addTarget()

It then may call 

- isortisearch::dynamicSearcher::dynamicSearch()

as often as desired. On each of these calls an image is passed to the dynamicSearcher. If the image is the most similar to the target(s) so far,
it will be inserted at the top of isortisearch::dynamicSearcher::dynamicSearchResults(). If it is the least similar, it will be inserted at the bottom
of dynamicSearchResults(). Furthermore, if resultSize() images are already
in dynamicSearchResults(), the least similar images will be deleted from the container.

Note that in contrast to the searcher, the dynamicSearcher is a container for images.

The results will be available in isortisearch::dynamicSearcher::dynamicSearchResults().

Note that the following template specializations can be used:

- isortisearch::dynamicSearcher<image>
- isortisearch::dynamicSearcher<sortImage>
- isortisearch::dynamicSearcher<controllerImage>

If a client wants to use both isortisearch::dynamicSearcher and isortisearch::sorter on the same %images it has to use the 2nd form.
If a client wants to attach() a isortisearch::guiController to the dynamicSearcher (wether or not it uses isortisearch::sorter too), it has to use the third form.
Note also that the client does not have to bother about the deletion of the image* or it's derivate passed to dynamicSearch() because it is wrapped in a isortisearch::refPtr<>.
*/
template<typename TIMG> class ISORT_API dynamicSearcher
{
public:
	/*!
	Constructor
	*/
	dynamicSearcher();
	/*!
	Destructor
	*/
	~dynamicSearcher();
	/*!
	A client has to call this before calling dynamicSearch(). This sets the maximum number of dynamic search results
	returned by dynamicSearchResults().

	Returns false if size==0.
	*/
	bool setResultSize(unsigned __int32 size);

	/*!
	returns the maximum number of results
	*/
	unsigned __int32 resultSize() const;

	/*!
	A client has to call this to search for similarities between the image passed and the target(s) based on the targetType().

	As long as dynamicSearchResults().size() is smaller than the size set by setResultSize(), the image passed will be inserted into
	dynamicSearchResults(). If is is the most similar image up to now, it will be inserted at the top position, if it is the least
	similar image, it will be inserted at the last position. If dynamicSearchResults().size() equals the size set by setResultSize(),
	the image passed will be inserted into dynamicSearchResults(), if there is a less similar image in the results, in this case, the 
	least similar image will be deleted from dynamicSearchResults().

	Returns false if numTargets()==0, resultSize()==0, the ref ptr passed is NULL or !pImg->featureDataValid(), in this case nothing happens.

	Note this calls lock() and unlock() internally. If the client calls this from a separate thread, it has to call
	enableMutex() before this.

	If the client has called registerSearchAdvancedCallback()
	before calling dynamicSearch() the registered callback will be called.
	*/
	bool dynamicSearch(const refPtr<TIMG>& pImg);

	/*!
	Sets an instance of the clients derived isortisearch::mutex class to the searcher.

	The client has to call this only if:
	
	- dynamicSearch() is to be started as a separate thread 
	
	and 

	- registerSearchAdvancedCallback() is called to paint the search advance. In this case, 
	the clients draw routine has to lock/unlock the mutex passed here when it accesses the images.
	*/
	bool setMutex(isortisearch::mutex* pMutex);
	/*!
	This locks the mutex.

	Note a client should always call this convenience function to lock the mutex.

	Note in general, a client only has to call this if dynamicSearch() runs in a separate thread and
	registerSearchAdvancedCallback() was called (with last parameter==true) to draw the results
	while they are searched.
	*/
	void lock();
	/*!
	This unlocks the mutex.

	Note a client should always call this convenience function to unlock the mutex.

	Note in general, a client only has to call this if dynamicSearch() runs in a separate thread and
	registerSearchAdvancedCallback() was called (with last parameter==true) to draw the results
	while they are searched.
	*/
	void unlock();
	/*!
	This enables the mutex.

	A client has to call this, if it calls dynamicSearch() from a separate thread and
	registerSearchAdvancedCallback() was called (with last parameter==true) to draw the results
	while they are searched.
	*/
	void enableMutex();
	/*!
	This disables the mutex. If the mutex is disabled, calling lock() and unlock() does nothing.

	In general, a client does not have to call this. This is the default.
	*/
	void disableMutex();
	/*!
	Adds a target image

	Returns false if !img.featureDataValid(), in this case the target is not added.

	Note this will change in the next release and accept refPtr<> parameters then.
	*/
	bool addTarget(const TIMG& img);
	/*!
	Adds a target rgb color
	*/
	void addTarget(unsigned char r,unsigned char g, unsigned char b);
	/*!
	Returns the number of targets
	*/
	unsigned numTargets() const;
	/*!
	Returns the number of target iamges
	*/
	unsigned numTargetImages() const;
	/*!
	Returns the number of target colors
	*/
	unsigned numTargetColors() const;
	/*!
	Clears the results.

	A client may call this to clear the results of prior calls to dynamicSearch().

	Note the size of dynamicSearchResults() will be zero after this.
	*/
	void clearResults();
	/*!
	Clears all targets.
	*/
	void clearTargets();
	/*!
	Clears all target images.
	*/
	void clearTargetImages();
	/*!
	Clears all target colors.
	*/
	void clearTargetColors();
	/*!
	Attaches a guiController to the %searcher

	Returns false if pController is NULL
	*/
	bool attach(guiController<TIMG>* pController);
	/*!
	This returns the results of dynamicSearch().

	Note the std::pair<float,TIMG> contained in the multimap returned contain the distance to the target(s) in the first() element and
	the image in the second() pair element. In general, the distance is not needed.

	Note if a client calls dynamicSearch() from a seperate thread and accesses the multimap from the main thread (i.e. to draw the results), it has to call
	lock() and unlock(). 
	*/
	/*const*/ std::multimap<float,refPtr<TIMG> >& dynamicSearchResults();
	/*!
	The client may register a callback to be called when dynamicSearch() advances.

	The registered callback will be called every interval calls to dynamicSearch().
	
	If bDraws is false, the client may only implement a simple callback like a wait cursor or a progress bar.

	If the client draws the current sort step in it's GUI, it has to pass bDraws==true.
	
	Note depending on the speed of the machine and the complexity of the registered callback, 
	the application may slow down if interval is chosen too low.

	Note there is no error check. Passing NULL for pCallBack or 0 for interval simply does not register the callback.

	Note that in case search() runs in a separate thread, the callback also runs in that thread. It's the clients
	responsibility to set up a safe inter thread communication with it's main GUI thread from the callback.
	*/
	void registerSearchAdvancedCallback(pxCallback pCallBack,void *pUserData,unsigned __int32 interval,bool bDraws);
	/*!
	This resest the counter for the registered callback to 0

	A client may call this in the beginning of a dynamicSearch() sequence when the callback is to be used.
	*/
	void resetCallbackCounter();
private:
	/*!
	the implementation pointer
	*/
	dynamicSearcherImpl<TIMG> *m_pImpl;
	/*!
	Note: The copy constructor is private
	*/
	dynamicSearcher(const dynamicSearcher& rOther);
	/*!
	Note: The assignment operator is private
	*/
	dynamicSearcher& operator=(const dynamicSearcher& rOther);
};



}
#endif