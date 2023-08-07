/*!
\mainpage

This document decribes the <B>vBase</B>, <B>vSearch</B> and <B>vSort</B> libraries. vBase is needed
for either of the other two libraries as a support library. Note the classes in vSort and vSearch are template 
classes, and valid template parameters are the different image types from vBase or derivates from these.

All classes and template classes in these libraries may throw exceptions derived form std::exception, however, most likely these exceptions 
will be of type std::bad_alloc. If a std::bad_alloc exception is thrown, the libaries will clean up prior memory allocations, but will not 
handle the exception itself, but re-throw the exception.

The classes and functions in the isortisearch libs are not reentrant in general.

<H1>vBase</H1>

The libraries <B>vbase</B> and <B>vbase_d</B> contain basic classes used by isortisearch::sorter, isortisearch::searcher and/or
isortisearch::dynamicSearcher.
Depending on the platform, these may be static libraries (.lib or .a) or dynamic link libraries (.dll or .so).

isortisearch::image contains the feature data of images used to sort and search for similar images as well as state information.
isortisearch::image does not handle %image input/output. That means, the client has to open images and pass them as plain
memory buffers into isortisearch::imageDescriptor before calling isortisearch::image::calculateFeatureData(). The client also has to specify the format of each image, whether it is RGBA, ABGR 
or a different order.

Note that in contrast to most classes and functions, isortisearch::image::calculateFeatureData() is reentrant.

isortisearch::sortImage adds relative position information for the image when sorted by isortisearch::sorter.

isortisearch::controllerImage adds absolute pixel position information for the image when controlled by a isortisearch::guiController.

isortisearch::guiController is a helper class template which allows a client to zoom and drag the contents of a client-supplied canvas displaying
the sorted images and/or the search results.

isortisearch::guiController does not handle drawing of the map itself. However, it calculates the absolute position 
of each image in the clients canvas and handles mouse events and zoom events if the client passes these events in. 
The client can draw the images into it's gui canvas based on these positions.

The isortisearch::mutex class is a base class for a client supplied %mutex, which is needed in some multithreading situations.

The isortisearch::refPtr template class is a helper for the classes which implements reference counting for 
pointer types.

Note the debug version of the library (vbase_d.lib) does not contain any debugging information. 
However, it links against the debug version of the standard libraries and is needed for debug builds of a client.

<H1>vSort</H1>

The libraries <B>vsort</B> and <B>vsort_d</B> contain a class template to visually sort a set of given images by their color similarity on a two dimensional map.
Depending on the platform, these may be static libraries (.lib or .a) or dynamic link libraries (.dll or .so).

A client has to call at least 

- isortisearch::sorter::setImages()
- isortisearch::sorter::sort()

to sort a set of images. Note that in this case, the template specialization

- isortisearch::sorter<sortImage>

is sufficient. Note that the client may derive classes of it's own from isortisearch::sortImage, since it passes a std::vector<refPtr<sortImage>> to
isortisearch::sorter::setImages(), i.e. pointers at least. Note the destructors of any derived image classes have to be virtual.

The result of the sort for each %image is given in isortisearch::sortImage::mapXPos() and isortisearch::sortImage::mapYPos().
These are positions in a 2D map consisting of isortisearch::sorter::mapPlacesX() positions in X direction and isortisearch::sorter::mapPlacesY()
in Y direction. Position (0,0) is at the top left of the map, where X grows to the right, Y grows to the bottom.

Suppose a client passed 1500 images to setImages(), this will result in isortisearch::sorter::mapPlacesX() == isortisearch::sorter::mapPlacesY() == 43,
that is, the map has 43 free places in each direction, thus a total of 43*43=1849 free places. Note in general the number free places should be bigger
than the number of images. A client may set the ratio of the number of free places to the number of images by 
isortisearch::sorter::setDimFactor(). Note also that the 2D map can be changed from a quadratic map to any rectangular map by calling
isortisearch::sorter::setAspectRatio().

In our example above, suppose an %image has isortisearch::sortImage::mapXPos()==3 and isortisearch::sortImage::mapYPos()==7, this means that the 
%image is positioned in column four and row eight of the map.

A client may also attach a guiController to the sorter, in this case it has to call at least:

- isortisearch::sorter::attach()
- isortisearch::guiController::setCanvasSize()
- isortisearch::sorter::setImages()
- isortisearch::sorter::sort()

Note that in this case, the template specialization

- isortisearch::sorter<controllerImage>

is required. Note again, the client may derive classes of it's own from isortisearch::controllerImage, see above.

Using the guiController, the client does not have to deal with the pure map postions delivered by e.g. isortisearch::controllerImage::mapXPos(),
it can use the absolute pixel positions delivered in 

- isortisearch::controllerImage::upperLeftX()
- isortisearch::controllerImage::upperLeftY()
- isortisearch::controllerImage::lowerRightX()
- isortisearch::controllerImage::lowerRightY()

which are the positons of the top left corner and bottom right corner of the %image in the clients canvas. Note that the top left pixel
of the canvas has position (0,0) where X grows to the right and Y grows to the bottom.

Furthermore, the client can pass mouse movement events and zomm events into isortisearch::guiController, thus the map can be dragged and zoomed
interactively.

Note the debug version of the library (vsort_d.lib) does not contain any debugging information. 
However, it links against the debug version of the standard libraries and is needed for debug builds of a client.

Note that there is a complete multithreaded gui example written in c++ using the open source gui framework gtkmm. 

<H1>vSearch</H1>

The libraries <B>vsearch</B> and <B>vsearch_d</B> contain class templates to search for images similar to one or more
target images and/or to a number of pure colors.

Depending on the platform, these may be static libraries (.lib or .a) or dynamic link libraries (.dll or .so).

This library contains two different template classes for searching: isortisearch::searcher and isortisearch::dynamicSearcher

<H2>isortisearch::searcher</H2>

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
Note again, the client may derive classes of it's own from isortisearch::image or it's derivates, see above.

<H2>isortisearch::dynamicSearcher</H2>

isortisearch::dyamicSearcher() is used to search for images similar to the targets while loading the images, i.e. from a disk or from the net.

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
Note also that the client does not have to bother about the deletion of the image or it's derivate passed to dynamicSearch() because it is wrapped in a isortisearch::refPtr<>.
Note again, the client may derive classes of it's own from isortisearch::image or it's derivates, see above.

Note the debug version of the library (vsearch_d.lib) does not contain any debugging information. 
However, it links against the debug version of the standard libraries and is needed for debug builds of a client.

Note that there is a complete multithreaded gui example written in c++ using the open source gui framework gtkmm. 

Copyright Visual Computing Group at HTW Berlin 2010-2023
*/

#if !defined(PXDEFS_H)
#define PXDEFS_H

namespace isortisearch {
/*!
The types for the callbacks used in classes 
*/
typedef void(*pxCallback)(void *pUserData);

// v3.0: no longer supported
///*!
//describes the way alpha channel data is regarded when calculting the feature data of an image
//
//see isortisearch::image::calculateFeatureData()
//*/
//enum alpha_mode{
//	IGNORE_ALPHA=0,	/**< ignores alpha, the default */
//	SKIP_ALPHA,		/**< skips alpha */
//	APPLY_ALPHA		/**< applies alpha */
//};

/*!
Describes the color channel layout of an image

Note may be extended in future releases
*/
enum channel_layout {
	UNDEFINED,  /**< the default, i.e. client has to call setChannelLayout() */
	ARGB,		/**< Alpha Red Green Blue */
	RGBA,		/**< Red Green Blue Alpha */
	BGRA,		/**< Blue Green Red Alpha */
	ABGR		/**< Alpha Blue Green Red */
};

/*!
The return value of isortisearch::sorter::sort()
*/
enum sort_result{
	SORT_ERROR=0,	/**< an error occured */
	SORT_COMPLETED, /**< no error, sort completed */
	SORT_ABORTED	/**< no error, sort aborted */
};

// v3.0: mo longer supported
///*!
//The type of target %image searched by isortisearch::searcher::search()
//*/
//enum target_type{
//	PHOTO=0,		/**< photorealistic */
//	GRAPHIC,		/**< graphic */
//	COLOR			/**< pure colors */
//};

/*!
The mode for isortisearch::guiController
*/
enum controller_mode{
	SORTED=0,						/**< sorted images */
	SORTER_INPUT,					/**< input images for the sorter row by row */
	LIST_SEARCH_RESULTS,			/**< results of image search row by row, most similar first */
	PYRAMID_SEARCH_RESULTS,			/**< results of image search row by row, most similar first and bigger */
	SEARCHER_INPUT,					/**< input images for the searcher row by row */
	LIST_DYNAMIC_SEARCH_RESULTS,	/**< results of dynamic image search row by row, most similar first */
	PYRAMID_DYNAMIC_SEARCH_RESULTS	/**< results of dynamic image search row by row, most similar first and bigger */

};

/*!
refPtr<T> is a smart pointer with reference counting.

This is a helper class for the isortisearch classes. A client has to use this to pass pointers to the image class or it's derivates
to the isortisearch classes. The client has to allocate the 'raw' pointer by new and has to wrap it into a refPtr<>. The client
does not have to delete the 'raw' pointer.
*/

template <class T> class refPtr {

public:
	/*!
	initializes the refPtr<> with the raw pointer p

	Note p has to be the return value of new.
	*/
    explicit refPtr(T* p=0) : m_Ptr(p), m_pCount(new unsigned long(1)) 
	{
		;
    }

    /*!
	copy constructor
	*/
    refPtr(const refPtr<T>& rOther) : m_Ptr(rOther.m_Ptr), m_pCount(rOther.m_pCount) 
	{
        ++(*m_pCount);
    }

    /*!
	destructor 

	Note deletes the raw pointer when the reference counter goes to zero
	*/
    ~refPtr()
	{
        cleanup();
    }

    /*! 
	assignment
	*/
    refPtr<T>& operator=(const refPtr<T>& rOther){
        if(this!=&rOther){
            cleanup();
            m_Ptr=rOther.m_Ptr;
            m_pCount=rOther.m_pCount;
            ++(*m_pCount);
        }
        return *this;
    }
    /*!
	read/write access value through reference
	*/
    T& operator*() 
	{
        return *m_Ptr;
    }
    /*!
	read access value through reference
	*/
    const T& operator*() const
	{
        return *m_Ptr;
    }
	/*!
	access raw pointer

	Note although this is const, the value may be changed. 
	This will be changed in the next version.
	*/
    T* operator->() const
	{
        return m_Ptr;
    }

	/*!
	returns true if the 'raw' pointer is NULL
	*/
	bool isNull() const {return !m_Ptr;}

private:
	// the raw pointer iself
    T* m_Ptr;
	// the counter
    unsigned long* m_pCount;

	/// @cond
	void cleanup()
	{
		--(*m_pCount);
		if(!(*m_pCount)){
			delete m_pCount;
			// m_Ptrs may be 0, see ctor, this is ok...
			delete m_Ptr;
		}
	}
	/// @endcond

};

}

/// @cond

// if you want to create or link to the static library, define ISORT_LIB
// note that inline does not work in this case..
// if you want to create the dll, define ISORT_EXPORTS
// if you want to link to the dll, undefine ISORT_EXPORTS
#define ISORT_LIB

#ifdef ISORT_LIB
#define ISORT_API
#define ISORT_INLINE
#elif defined(ISORT_EXPORTS)
#define ISORT_API __declspec(dllexport)
#define ISORT_INLINE inline
#else
#define ISORT_API __declspec(dllimport)
#define ISORT_INLINE inline
#endif

/// @endcond

#endif

//////////////////////////////////////////////////////////////////////////////////////
// added by Daniel 
//

#ifdef __GNUC__
#	include <stdio.h>
#	include <string.h>
#	ifndef __INT32
#		define __INT32
#		define __int32 int
#	endif // __INT32
#	ifndef __UINT32
#		define __UINT32
#		define __uint32 unsigned int
#	endif // __UINT32
#	define __max(a,b)  (((a) > (b)) ? (a) : (b))
#	define __min(a,b)  (((a) < (b)) ? (a) : (b))
#	define memcpy_s(a,b,c,d) memcpy(a,c,d)
#endif
//
// END CHANGES
//////////////////////////////////////////////////////////////////////////////////////
