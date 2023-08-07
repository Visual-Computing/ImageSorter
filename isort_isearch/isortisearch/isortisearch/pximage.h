#if !defined(PXIMAGE_H)
#define PXIMAGE_H

#include "pxdefs.h"

namespace isortisearch{

// forwards
class imageImpl;
class image;

/*!
A helper struct describing the image.

A client has to set the public members and pass it to image::calculateFeatureData()
*/
struct imageDescriptor{
	/*!
	Constructor
	*/
	imageDescriptor() : m_pImage(0),m_ChannelLayout(UNDEFINED),m_pFirstPixel(0),/*m_AlphaMode(IGNORE_ALPHA),m_Red(0),m_Green(0),m_Blue(0),*/m_Width(0),m_Height(0) {;} 
	/*!
	Destructor
	*/
	~imageDescriptor() {;}
	/*!
	A pointer to the image itself

	Note NULL by constructor, i.e. a client has to set this, otherwise image::calculateFeatureData() returns false.

	Note non-const, since image::calculateFeatureData() will set the feature data.
	*/
	image* m_pImage;
	/*!
	The channel layout of the image, see isortisearch::channel_layout

	Note UNDEFINED by constructor, i.e. a client has to set this, otherwise image::calculateFeatureData() returns false.
	*/
	channel_layout m_ChannelLayout;
	/*!
	the width of the image in pixels

	Note 0 by constructor, i.e. a client has to set this. Note this has to be bigger or equal to 8,
	otherwise image::calculateFeatureData() returns false.

	Note also either this or m_Height has to be bigger or equal than 40, otherwise image::calculateFeatureData() returns false.
	*/
	unsigned __int32 m_Width;
	/*!
	the heigth of the image in pixels

	Note 0 by constructor, i.e. a client has to set this. Note this has to be bigger or equal to 8,
	otherwise image::calculateFeatureData() returns false.

	Note also either this or m_Width has to be bigger or equal than 40, otherwise image::calculateFeatureData() returns false.
	*/
	unsigned __int32 m_Height;
	/*!
	A pointer to the first pixel of the plain %image data.

	Note %image data has to be contignous, i.e. there may not be any padding bytes.

	Note NULL by constructor, i.e. a client has to set this, otherwise image::calculateFeatureData() returns false.
	*/
	const unsigned __int32* m_pFirstPixel;
	// V3.x: alpha modes no longer supported
	///*!
	//The way alpha channel data has to be handled, see isortisearch::alpha_mode

	//Note IGNORE_ALPHA by constructor, thus the default is to ignore any alpha channel data.
	//*/
	//alpha_mode m_AlphaMode;
	///*!
	//The red background color component

	//Note 0 by constructor, this is only needed if m_AlphaMode==APPLY_ALPHA.
	//*/
	//unsigned char m_Red;
	///*!
	//The green background color component

	//Note 0 by constructor, this is only needed if m_AlphaMode==APPLY_ALPHA.
	//*/
	//unsigned char m_Green;
	///*!
	//The blue background color component

	//Note 0 by constructor, this is only needed if m_AlphaMode==APPLY_ALPHA.
	//*/
	//unsigned char m_Blue;
};


/*!
This is a wrapper base class for the %images used by isortisearch::sorter, isortisearch::searcher, isortisearch::dynamicSearcher and isortisearch::guiController. 
image stores feature data used to sort images and to search for similar images as well as state information.

Note that image does not change the clients %image pixel data.

Note that the client has to call:

- calculateFeatureData() or setFeatureData()

Note that for calling calculateFeatureData() the client has to fill out and pass an imageDescriptor struct.

image implements a 32 bit field in flags() to store internal state informations. Derivates may also use 
these bits. Note that image reserves the lowest bit 1 of the flags(). 

Currently only 32 bit RGB color images with 8 bits per color channel are supported. The fourth channel is ignored, wether
or not it contains the alpha channel.
*/
class ISORT_API image{
public:
	/*!
	Constructor
	*/
	image();
	/*!
	Copy constructor
	*/
	image(const image& rOther);
	/*!
	Destructor
	*/
	virtual ~image();
	/*!
	Assignment
	*/
	image& operator=(const image& rOther);
	/*!
	Returns the current major version of this class.

	Note a major version change means that the data returned by featureData() and/or the sizes
	returned by featureDataSize() and/or featureDataByteSize() have changed.
	*/
	static unsigned __int32 majorVersion();
	/*!
	Returns the current minor version of this class.

	Note a minor version change means that the data returned by featureData() and the sizes
	returned by featureDataSize() and featureDataByteSize() have not changed.
	*/
	static unsigned __int32 minorVersion();
	/*!
	Sets the feature data.

	In general, the client does not need to call this. However, the client may store the feature
	data for an %image in a database and may call this to set the feature data later to prevent
	another call to calculateFeatureData().

	Version is the version number of the data, this has to be the same as majorVersion().

	The buffer passed has to be of the size given by featureDataByteSize().

	Returns false if data is NULL or if the version number is not equal to majorVersion().
	*/
	bool setFeatureData(unsigned __int32 version,void *data);

	/*!
	Returns a pointer to the feature data.

	May be used by the client to store the feature data in a database.

	The feature data size is featureDataByteSize() bytes. 

	Note the client should store majorVersion() and featureDataByteSize()
	along with the feature data.
	*/
	const char *featureData() const;

	/*!
	returns the size (in bytes) of the feature data
	*/
	static unsigned __int32 featureDataByteSize();

	/*!
	returns the size of the feature data. 
	
	Note in general, the client does not need to call this and shall not make assumptions on the
	type of feature data, since this may change with a major version change.
	*/
	static unsigned __int32 featureDataSize();

	/*!
	Calculates the feature data.

	Note this function is reentrant.

	Note the client has to fill an imageDescriptor as a parameter for this function, supplying all relevant
	information about the image.

	Note one of the image dimensions passed in imageDescriptor.m_Width and imageDescriptor.m_Height has to be equal or bigger than 40 pixels,
	the other has to be equal or bigger than 8 pixels. 
	
	Returns false if the imageDescriptor members are not valid.
	*/
	static bool calculateFeatureData(const imageDescriptor& iDesc);

	/*!
	Returns true if feature data is valid, either after calculateFeatureData() or after setFeatureData()
	*/
	bool featureDataValid() const;

	/*!
	Returns the flags

	image hold a 32 bit flag field wich is used internally to store states.

	A client which derives classes from image or one of it's derivates
	may use the remaining bits in the flags tho store states of it's own.
	However, the lowest bit 1 is reserved for image, other bits may be reserved for existing derivates.
	*/
	unsigned __int32 flags() const {return m_uFlags;}

	/*!
	Sets the flags

	Note use with care, do not change bit 1 or any other bits used by derivates.
	*/
	void setFlags(unsigned __int32 f) {m_uFlags=f;}

	// friends...
	friend class imageImpl;
	template<typename TIMG> friend class guiControllerImpl;
	template<typename TBIG> friend class featureGenerator;
protected:
	static const unsigned __int32 m_ValidFeatureDataFlag;
	static const unsigned __int32 m_ValidFeatureDataMask;
	unsigned __int32 m_uFlags;
private:
	imageImpl* m_pImpl;
};

/*!
sortImage stores the position information provided by isortisearch::sorter::sort().

These positions are given in mapXPos() and mapYPos() and describe the %image position
in a two dimensional map of dimension isortisearch::sorter::mapPlacesX() and isortisearch::sorter::mapPlacesY().

Note that position (0,0) is the top-left position in the map, position (sorter::mapPlacesX()-1,sorter::mapPlacesY()-1)
is the bottom-right position in the map.

Note a client may store the mapXPos() and mapYPos() positions of a set of sorted images in a 
database and load that again by calling setMapXPos() and setMapYPos() later to avoid a resort.

Note that a client using isortisearch::guiController along with isortisearch::sorter to draw the sorted
images on a two dimensional canvas does not have to call any of the sortImage members.
*/
class ISORT_API sortImage : public image{
public:
	/*!
	Constructor
	*/
	sortImage();
	/*!
	Copy constructor
	*/
	sortImage(const sortImage& rOther);
	/*!
	Destructor
	*/
	virtual ~sortImage();
	/*!
	Assignment
	*/
	sortImage& operator=(const sortImage& rOther);
	/*!
	Returns the x position of this image in the 2D sort map.

	Note a client which uses the isortisearch::controllerImage derivate along with isortisearch::guiController
	does not need to call this to implement drawing.

	Note that if isortisearch::sorter::sort() is running in a different thread, the client should not
	call this without locking and unlocking the mutex passed to isortisearch::sorter::setMutex(). 
	*/
	unsigned __int32 mapXPos() const;

	/*!
	Returns the y position of this image in the 2D sort map.

	Note a client which uses the isortisearch::controllerImage derivate along with isortisearch::guiController
	does not need to call this to implement drawing.

	Note that if isortisearch::sorter::sort() is running in a different thread, the client should not
	call this without locking and unlocking the mutex passed to isortisearch::sorter::setMutex(). 
	*/
	unsigned __int32 mapYPos() const;

	/*!
	Sets the x position of this image in the 2D sort map.

	In general, the client does not need to call this.
	*/
	void setMapXPos(unsigned __int32 x) {m_uMapXPos=x;}

	/*!
	Sets the y position of this image in the 2D sort map.

	In general, the client does not need to call this.
	*/
	void setMapYPos(unsigned __int32 y) {m_uMapYPos=y;}

	// friends
	template<typename TIMG> friend class sorterImpl;
	template<typename TIMG> friend class guiControllerImpl;
private:
	unsigned __int32 m_uMapXPos;
	unsigned __int32 m_uMapYPos;
};

/*!
controllerImage stores 2D drawing informations provided by isortisearch::guiController in the upperLeftX(), upperLeftY(),
lowerRightX() and lowerRightY() positions and in the visible() flag.

Note that this class reserves bit 2 of isortisearch::image::flags().
*/

class ISORT_API controllerImage : public sortImage{
public:
	/*!
	Constructor
	*/
	controllerImage();
	/*!
	Copy constructor
	*/
	controllerImage(const controllerImage& rOther);
	/*!
	Destructor
	*/
	virtual ~controllerImage();
	/*!
	Assignment
	*/
	controllerImage& operator=(const controllerImage& rOther);

	/*!
	The client has to call this to set the width of the %image in pixels.

	Note width has to be bigger or equal to 8 pixels.

	Returns false on error, in this case the value passed is not valid.

	Note the %image dimensions are needed by the guiController only. Therefore, these are not part
	of the base class image. However, a client has to pass the width to the base class image::calculateFeatureData()
	via an imageDescriptor.
	*/
	bool setWidth(unsigned __int32 width);

	/*!
	The client has to call this to set the height of the %image in pixels.

	Note height has to be bigger or equal to 8 pixels.

	Returns false on error, in this case the value passed is not valid.

	Note the %image dimensions are needed by the guiController only. Therefore, these are not part
	of the base class image. However, a client has to pass the height to the base class image::calculateFeatureData()
	via an imageDescriptor.
	*/
	bool setHeight(unsigned __int32 height);
	/*!
	This is just an abbreveation for setWidth() and setHeight()
	*/
	bool setDimensions(unsigned __int32 width,unsigned __int32 height);
	/*!
	Returns the width of this %image in pixels.
	*/
	unsigned __int32 width() const;
	/*!
	Returns the height of this %image in pixels.
	*/
	unsigned __int32 height() const;
	/*!
	Returns true if visible. An image may be invisible if isortisearch::guiController::onZoom() has been called.

	The client has to call this to check whether or not the image shall be drawn.
	
	Note if an image is not visible, upperLeftX(), upperLeftY(), lowerRightX() and lowerRightY()
	will return invalid values.
	*/
	bool visible() const;

	/*!
	Returns the x coordinate of the upper left corner in the clients canvas.

	The client has to call this to draw the image in it's canvas.

	Note that this value can be negative, in this case the image is at the clients canvas border 
	and is partially visible.
	*/
	__int32 upperLeftX() const {return m_UpperLeftX;}

	/*!
	Returns the y coordinate of the upper left corner in the clients canvas.

	The client has to call this to draw the image in it's canvas.

	Note that this value can be negative, in this case the image is at the clients canvas border 
	and is partially visible.
	*/
	__int32 upperLeftY() const {return m_UpperLeftY;}

	/*!
	Returns the x coordinate of the lower right corner in the clients canvas.

	The client has to call this to draw the image in it's canvas.

	Note that this value can be negative, in this case the image is at the clients canvas border 
	and is partially visible.
	*/
	__int32 lowerRightX() const {return m_LowerRightX;}

	/*!
	Returns the y coordinate of the lower right corner in the clients canvas.

	The client has to call this to draw the image in it's canvas.

	Note that this value can be negative, in this case the image is at the clients canvas border 
	and is partially visible.
	*/
	__int32 lowerRightY() const {return m_LowerRightY;}

	// friends
	template<typename TIMG> friend class guiControllerImpl;
protected:
	static const unsigned __int32 m_VisibleFlag;
	static const unsigned __int32 m_VisibleMask;

	__int32 m_UpperLeftX;
	__int32 m_UpperLeftY;
	__int32 m_LowerRightX;
	__int32 m_LowerRightY;

	unsigned __int32 m_uWidth;
	unsigned __int32 m_uHeight;
};

}

#endif