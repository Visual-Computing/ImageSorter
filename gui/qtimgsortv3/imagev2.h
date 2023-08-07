#pragma once
#include "imageerror.h"
#include "geom.h"
#include "color.h"

#include <math.h>
#include <assert.h>
// #include "tiff.h"
// #include "tiffio.h"
#include <string>
#include <stack>
using namespace std;

/*!
imagev2<> is a template class for images. It is a better version of image<>. It will be image<> later,
(and the current image<> will be oldImage<>, probably).

note that type T should be a build-in unsigned integral value like unsigned char or unsigned short.

other types will probably have malfunctions, thus this template class and derived has to be overloaded
for other types T. anyway, float and double are usable, too, but several member functions will throw
imageError exceptions for these types. if this is the case, it is stated in this documentation. however,
take care when using these types, this is not really tested...

call create() to create an image. 

note that an image can be created based on another or an external image. in this case, no image memory is 
allocated, but a pointer to the image memory is passed. this is useful if the image is from another context
(like a QImage from Qt or something else...)

note that an image can have a relatively arbitrary channel layout:

primChannels() is the number of primary image channels per pixel. e.g. in a rgb color image, these are red, 
green and blue. the primary channels have to be adjacent and are the pixels algorithms work on.
primChannels() cannot be 0.

preChannels() are any channels prior to the primChannels() in a pixel. algorithms do not work on these.
e.g. in an argb image, this can be the alpha channel. preChannels() have to be adjacent and can be 0.

postChannels() are any channels after the primChannels() in a pixel. algorithms do not work on these.
e.g. in an rgba image, this can be the alpha channel. postChannels() have to be adjacent and can be 0.

note that if you want to operate on an alpha channel, too, you need so set primChannels()==4 and set the appropriate 
colorSpace.

channel layout can be set in create() or later in setPreChannels() and setPostChannels(). channel layouts
can be push'ed and pop'ed with pushChannelLayout() and popChannelLayout().

note that with this approach, we can e.g. work on a single channel of a multichannel image. furthermore,
together with the based approach, we can define a single channel image based on a multichannel image...

note that image width and height are restricted to maxWidth() and maxHeight(), currently this
is 16384 both which should be big enough.

either call member functions to do operations on the image or access the image buffer
by pFirstPixel() or pPixelAt().

note that imagev2<T> can have regions of interest (ROIs). call setRoi() to set the roi, 
call enableRoi() to enable roi processing, call disableRoi() to disable roi processing. if 
roiEnabled(), allmost all member function will (transparently) work on the roi only instead of the full
image, i.e fill() will fill the roi and saveTiff() will save the roi.

note that the roi can be stacked with pushRoi() and retrieved with popRoi().

note that imagev2<T> may hasBorders(). in this case, the image has a border to all sides which is
borderWidth() pixels wide. these borders are useful (only) for filtering operations, where each pixel
in the source image needs a number of neighbours (depending on the size of filter kernel). 
at the moment, the border can only be specified on create(), i.e. the border width cannot change after creation.

note that the border width is restricted to maxBorderWidth().

extendToBorders() can be used to extend the image (edges) to the borders before a filter operation.
fillBorders() can be used to fill the borders with a const value.
this is the only way to set pixels in the borders. 

note that other members never access the border pixel. 
in a way, borders set up an roi which is always enabled.

<H2>
accessing image memory:
</H2>

to access image memory, the following procedure is working wether roiEnabled() or not:

\code
unsigned x,y,c;
// roi or image width:
const unsigned w=width();
// roi or image height:
const unsigned h=height();
// offset from one behind roi line to next roi line start or 0
// note in units of T*channels():
const unsigned wrap=lineWrap();
const unsigned chans=channels();
// first roi or first image ptr:
T* pPix=pFirstPixel();				

for(y=0;y<h;++y){
	for(x=0;x<w;++x){
		for(c=0;c<chans;++c){
			*pPix++=foo();
			}
		}
	pPix+=wrap;
	}
\endcode

to access e.g. column 10 of image memory, this will work wether roiEnabled() or not:

\code
unsigned y,colNum=10;
// roi or image height:
const unsigned h=height();
// offset from a pixel to the pixel above/below, either in roi or image
// do *not* use width() here:
const unsigned lOffset=lineOffset();
// ptr at (x,y), either in roi or image:
T* pPix=pPixelAt(colNum,0);		

for(y=0;y<h;++y){
	*pPix=foo();
	pPix+=lOffset;
	}
\endcode

note that the code above is definitly faster than calling e.g. width() inside the
loops, wether the compiler inlines width() or not. width(), height() and many others 
check if roiEnabled() internally, thus do an if() inside...

<H2>
error handling:
</H2>

note that most functions throw imageError ín case of error. be sure to test for these like e.g.
in the following code:

\code
#include <iostream>
#include "imagev2.h"
void main(void)
{
	try{
		imagev2<T> rImg;
		// will throw exception because image dims too big:
		rImg.create(0xFFFF,0xFFFF,1,0);
		// something else:
		rImg.foo();
		}
	catch(imageError e){
		cout << "catched error:\n";
		cout << "cause: " << e.causeStr() <<"\n";
		cout << "src file: " << e.srcFileStr() << "\n";
		cout << "src line: " << e.srcLine() <<"\n";
		cout << "press return to terminate...\n";
		cin.get();
	}
	return;
}
\endcode

\todo check if virtual functions really needed

\todo build a non-template abstract base class on top of imagev2<T>, this shall hold all 
members *not* depending on type T, like dimesions and so on. this will be usefull when
calling members between different specializations of the template type T.
*/
template <class T> class imagev2
{
public:
	/*!
	currently supported color spaces
	*/
	enum colorSpace{
		colorSpaceNone=0,
		colorSpaceGrey,		// this is for single channel images
		colorSpaceRGB,
		colorSpaceBGR,		// this is a 'hack' for big/little endian...
		colorSpaceARGB,
		colorSpaceRGBA,
		colorSpaceABGR,		
		colorSpaceBGRA,		
		colorSpaceYUV,		
		colorSpaceLUV,
		colorSpaceLAB,
		colorSpaceLmaxAB,
		colorSpaceHMMD,
		colorSpaceHMMDImproved
	};
	/*!
	note you need to call create() after ctor
	*/
	imagev2(void);
	virtual ~imagev2(void);
	/*!
	call this to create an image

	width() is the width in pixels, height() is the height.

	note that width and height are restricted to maxWidth() and maxHeight(), respectively

	borderWidth is the width of the image border in pixels to each side, pass 0 if not required

	chans is the (primary) number of channels of type T per pixel.

	if preChans!=0, each pixel has preChans channels of type T before the (pimary) image channels.

	if postChans!=0, each pixel has postChans channels of type T after the (pimary) image channels.

	note thay you may also call setPreChans() and setPostChans() after create(). note that the preChans()
	ans postChans() channels are not meant to be worked on in algorithms. this seems to be strange,
	but with this property (especially together with the isBased() property) it is possible, e.g. to
	access the rgb channels in an (external) argb image or to work on a sinlgle channel of a multichannel
	image. however, alogorithm will be slower with this feature.

	if bBased is true, no image memory is allocated, but the adress in pBase is taken as the 
	first channel in the first pixel, i.e. the image can be allocated by a different source. 
	note that the caller has to gurantee that the adress passed has a lifetime sufficient for
	the lifetime of this instance. note that this instance will not delete the memory if 
	bBased is true. note that pBase is not evaluated if !bBased

	throws imageError in case of error
	*/
	virtual void create(unsigned width,
						unsigned height,
						unsigned chans,
						unsigned borderWidth=0,
						unsigned preChans=0,
						unsigned postChans=0,
						bool bBased=false,
						T* pBase=NULL);
	/*!
	returns true if the image is based on external image memory
	*/
	bool isBased() const {return m_bBased;}
    /*!
	the total number channels

	note is the sum of primChannels, preChannels() and postChannels()
	*/
	unsigned channels() const {return m_uChannels;}
    /*!
	the number of primary channels
	*/
	unsigned primChannels() const {return m_uPrimChans;}
	/*!
	returns true if the image has an additional alpha channel
	*/
	unsigned preChannels() const {return m_uPreChans;}
	/*!
	returns true if the image has an additional alpha channel
	*/
	unsigned postChannels() const {return m_uPostChans;}
	/*!
	returns true if this has preChannels()
	*/
	bool hasPreChannels() const {return m_uPreChans!=0;}
	/*!
	returns true if this has postChannels()
	*/
	bool hasPostChannels() const {return m_uPostChans!=0;}
	/*!
	sets preChannels() to preChans

	note that primChannels() will in turn change, of course

	note throws primChannels() will be <= 0, i.e. if preChans>=primChannels()+postChannels()
	*/
	void setPreChannels(unsigned preChans);
	/*!
	sets postChannels() to postChans

	note that primChannels() will in turn change, of course

	note throws if primChannels() will be <= 0, i.e. if postChans>=primChannels()+preChannels()
	*/
	void setPostChannels(unsigned postChans);
	/*!
	pushes the current pre/prim/post channel state to an internal stack
	*/
	void pushChannelLayout();
	/*!
	pops the current pre/prim/post channel state from an internal stack

	note throws if stack empty
	*/
	void popChannelLayout();
	/*!
	if the image is set from outside, i.e. through pFirstPixel(), this may
	be used to set the color space
	*/
	void setColorSpace(typename imagev2<T>::colorSpace cSpace) {m_eColorSpace=cSpace;}
	/*!
	returns the color space
	*/
	typename imagev2<T>::colorSpace getColorSpace() const {return m_eColorSpace;}
	/*!
	returns true if grey image

	note there is a colorSpaceGrey value in the colorSpace enum.
	anyway, we assune any single channel image to be grey. that is, if (temporary) you 
	change a e.g. three channel image into a single channel image by setPreChannels() and/or
	setPostChannels(), this will be (temporary) grey...
	*/
	bool isGrey() const {return primChannels()==1;}
	/*!
	returns true if rgb image

	note that the image has to have colorSpaceRGB and has to have three (primary) channels currently
	*/
	bool isRGB() const {return primChannels()==3&&m_eColorSpace==colorSpaceRGB;}
	/*!
	returns true if bgr image

	note that the image has to have colorSpaceBGR and has to have three (primary) channels currently
	*/
	bool isBGR() const {return primChannels()==3&&m_eColorSpace==colorSpaceBGR;}
	/*!
	returns true if argb image

	note that the image has to have colorSpaceARGB and has to have four (primary) channels currently
	*/
	bool isARGB() const {return primChannels()==4&&m_eColorSpace==colorSpaceARGB;}
	/*!
	returns true if rgba image

	note that the image has to have colorSpaceRGBA and has to have four (primary) channels currently
	*/
	bool isRGBA() const {return primChannels()==4&&m_eColorSpace==colorSpaceRGBA;}
	/*!
	returns true if abgr image

	note that the image has to have colorSpaceABGR and has to have four (primary) channels currently
	*/
	bool isABGR() const {return primChannels()==4&&m_eColorSpace==colorSpaceABGR;}
	/*!
	returns true if bgra image

	note that the image has to have colorSpaceBGRA and has to have four (primary) channels currently
	*/
	bool isBGRA() const {return primChannels()==4&&m_eColorSpace==colorSpaceBGRA;}
	/*!
	returns true if this has an alpha channel
	*/
	bool hasAlpha() const {return isARGB()||isARGB()||isBGRA()||isRGBA();}
	/*!
	returns true if this is any of the RedGreenBlue color spaces (w or w/o alpha)
	*/
	bool isRGBSpace() const {return isBGR()||isRGB()||hasAlpha();}
	/*!
	returns the blue channel offset

	note this returns 0xFFFFFFFF if getColorSpace() is not any of the RGB spaces
	*/
	unsigned blueChannel() const;
	/*!
	returns the green channel offset

	note this returns 0xFFFFFFFF if getColorSpace() is not any of the RGB spaces
	*/
	unsigned greenChannel() const;
	/*!
	returns the red channel offset

	note this returns 0xFFFFFFFF if getColorSpace() is not any of the RGB spaces
	*/
	unsigned redChannel() const;
	/*!
	returns the alpha channel offset

	note this returns 0xFFFFFFFF if getColorSpace() is not any of the RGB spaces with alpha
	*/
	unsigned alphaChannel() const;
	/*! 
	destroys this image. call this to create() or loadTiff() again...
	*/
	void destroy(void);
	/*!
	loads a tiff file if !created() 
	note that currently, only uncompressed rgb, color mapped and grey images are supported.

	borderWidth is the border width, if needed

	throws imageError in case of error

	\todo: loading the resolution does not work currently
	*/
	// void loadTiff(const char* name,unsigned borderWidth=0);
	/*!
	an overload of loadTiff(const char*) for <string> parameters

	throws imageError in case of error
	*/
	// void loadTiff(const string& name,unsigned borderWidth=0) {loadTiff(name.c_str(),borderWidth);}
	/*!
	saves a tiff file.
	note that currently, only uncompressed rgb and grey images are supported.

	note if roiEnabled(), the roi is saved.

	throws imageError in case of error
	*/
	// void saveTiff(const char* name);
	/*!
	an overload of saveTiff(const char*) for <string> parameters

	throws imageError in case of error
	*/
	// void saveTiff(const string& name) {saveTiff(name.c_str());}
	/*!
	note you *have* to overload this in derived classes.
	this is called by loadTiff(). it should check if the tiff tags in pTiff are 
	compatible to this derived image and should return the number of image channels required.

	note the derived shall throw imageError on error
	*/
	// virtual unsigned checkTiffTags(TIFF* pTiff) {throw fqImageError(imageError::illegalBaseCall);}
	/*!
	note you *have* to overload this in derived classes
	this is called by saveTiff(). it should sets the tiff tags in pTiff to match
	the derived image requirements.

	note the derived shall throw imageError on error
	*/
	// virtual void setTiffTags(TIFF* pTiff) {throw fqImageError(imageError::illegalBaseCall);}
	/*!
	a wrapper for TIFFWriteScanline

	this may be overloaded by subclasses if data written to tiff files has to be different from image data

	subclasses may call this base implementation to write data after conversion
	*/
	// virtual int onTiffWriteScanline(TIFF* tiff,void *buf,unsigned line) {return TIFFWriteScanline(tiff,buf,line);}
	/*!
	a wrapper for TIFFReadScanline

	this may be overloaded by subclasses if data read from tiff files has to be different from image data

	subclasses may call this base implementation to read data prior to conversion
	*/
	// virtual int onTiffReadScanline(TIFF* tiff,void *buf,unsigned line) {return TIFFReadScanline(tiff,buf,line);}
	/*!
	this gets called prior to data being loaded from a tiff file in loadTiff()

	subclasses may overload this to set special properties from the tiff file,
	i.e. color space tags

	the base implementation is NOP
	*/
	// virtual void onPreLoadTiff(TIFF* pTiff) {return;}
	/*!
	returns true if create() or loadTiff() where successfull
	*/
	bool created() const {return m_bCreated;}
	/*!
	returns the current width

	this is either the roi width (if roiEnabled()) or the full width
	*/
	unsigned width() const {return m_Roi.width;}
	/*!
	returns the current height

	this is either the roi height (if roiEnabled()) or the full height
	*/
	unsigned height() const {return m_Roi.height;}
	/*!
	returns a pointer to the first pixel

	note this is the first pixel of the roi if roiEnabled()
	*/
	T* pFirstPixel() {return m_pRoiFirstPixel;}
	/*!
	same as pFirstPixel(), but returns a pointer to a constant pixel.
	*/
	const T* pcFirstPixel() const {return (const T*)m_pRoiFirstPixel;}
	/*!
	return the offset (in sizeof(T)) from one behind the end of an image line
	to the start of the next line

	note this is zero only if !roiEnabled() && !hasBorders()
	*/
	unsigned lineWrap() const {return m_uRoiLineWrap;}
	/*!
	return the offset (in sizeof(T)) from a pixel/component to a pixel/component just
	one line below (or above)

	note this is always the same, regardless of roiEnabled() and is like width() when
	!roiEnabled() && !hasBorders().
	*/
	unsigned lineOffset() const {return (m_FullRoi.width+2*borderWidth())*channels();}
	/*!
	returns a pointer to the pixel (x,y)

	note: no bounds check on x and y
	*/
	T* pPixelAt(unsigned x,unsigned y) {return(pFirstPixel()+x*channels()+y*lineOffset());}
	/*!
	like pPixelAt(), but returns a pointer to primary channel chan of the pixel at x,y.

	note primary channel count is zero based

	note preChannels() taken into account

	note no bounds check on x and y and chan
	*/
	T* pAtChannel(unsigned x,unsigned y,unsigned chan) {return pPixelAt(x,y)+preChannels()+chan;}
	/*!
	returns a reference to pixel (x,y)

	note the return value is an l-value, i.e. you can assing to it. use tcAt() if not needed.

	note: no bounds check on x and y
	*/
	T& tAt(unsigned x,unsigned y) {return *pPixelAt(x,y);}
	/*!
	returns a reference to pixel (x,y) at channel chan

	note the return value can be an l-value, i.e. you can assing to it. 
	use tcAtChannel() if not needed.

	note: no bounds check on x and y and chan
	*/
	T& tAtChannel(unsigned x,unsigned y,unsigned chan) {return *pAtChannel(x,y,chan);}
	/*!
	returns the value of pixel (x,y)

	note: no bounds check on x and y
	*/
	const T& tcAt(unsigned x,unsigned y) {return *pPixelAt(x,y);}
	/*!
	returns the value of pixel (x,y) at channel chan

	note: no bounds check on x and y and chan
	*/
	const T& tcAtChannel(unsigned x,unsigned y,unsigned chan) {return *pAtChannel(x,y,chan);}
	/*!
	fills the image with a constant value.
	
	make sure to pass primChannels() values.
	
	note preChannels() and postChannels() are not filled.

	note optimized version for !hasPreChannels()&&!hasPostChannels() included.

	note for greyImage<T>, better use greyImage<T>::fill(T val).

	throws imageError if error
	*/
	void fill(const T* const values);
	/*!
	fills the image with val where pixels are comp.

	make sure to pass primChannels() values.

	note preChannels() and postChannels() are not filled.

	note optimized version for !hasPreChannels()&&!hasPostChannels() included.

	note for greyImage<T>, better use greyImage<T>::fillCompared(const T val,const T comp).

	note that this is essentially the same as fillMasked(val,this,comp), but faster

	throws imageError if error
	*/
	void fillCompared(const T* const values,const T* const compValues);
	/*!
	fills the image with a constant value where maskImg has value maskVal

	make sure to pass primChannels() values.

	note preChannels() and postChannels() are not filled.

	note that this as well as the mask image can have preChannels() and postChannels()

	note optimized version for !
	hasPreChannels()&&
	!hasPostChannels()&&
	!maksImg.hasPreChannels()&&
	!maksImg.hasPostChannels() included.

	note for greyImage<T>, better use greyImage<T>::fillMasked().

	throws imageError if error
	*/
	template <class TMask> void fillMasked(const T* const values,imagev2<TMask>& maksImg,TMask maskVal);
	/*!
	returns the maximum value for the channel type T, e.g. 255 for unsigned char
	*/
	static T maxChannelValue(void) {return m_tMaxChannelValue;}
	/*!
	returns the maximum value for the channel type T, e.g. 0 for unsigned char
	*/
	static T minChannelValue(void) {return m_tMinChannelValue;}
	/*!
	return the maximum width we can create()
	*/
	static unsigned maxWidth() {return m_uMaxWidth;}
	/*!
	return the maximum height we can create()
	*/
	static unsigned maxHeight() {return m_uMaxHeight;}
	/*!
	return the maximum border width we can create()
	*/
	static unsigned maxBorderWidth() {return m_uMaxBorderWidth;}
	/*!
	returns the number of bits of channel type T
	*/
	static unsigned channelBitDepth() {return sizeof(T)*8;}
	/*!
	returns the number of bytes of channel type T
	*/
	static unsigned channelByteDepth() {return sizeof(T);}
	/*!
	copy rOther to this

    note this works with any combination of roiEnabled() and rOther.roiEnabled(),
	i.e. if roiEnabled(), rOther is copied to roi, and if rOther.roiEnabled(),
	rOther's roi is copied...

	note also that this works with any valid combination of preChannels() and postChannels()
	as well as rOther.preChannels() and rOther.postChannels(), there are optimized versions
	included if neither has any of these.

	note however that is is possible to copy e.g. a single channel of rOther to a (different) single 
	channel	of this or - even more elaborated - a single channel of this to a different single channel
	of this (by creating rOther based on this). note that in the latter case, source and destination 
	shall not overlap...

	note calling img<>::copy(*this) is NOP

	throws imageError if error
	*/
	virtual void copy(const imagev2<T>& rOther);
	/*!
	return true if rOther has same layout, i.e. same width(), same height(), same primChannels()

	note that neither this nor rOther has to be created()
	
	note this works with any combination of roiEnabled() and rOther.roiEnabled().

	note that because of hasBoarders(), preChannels() and postChannels() this does
	not test for same memory layout...
	*/
	bool sameLayout(const imagev2<T>& rOther) const {return rOther.width()==width()&&
														  rOther.height()==height()&&
														  rOther.primChannels()==primChannels();}
	/*!
	returns the x resolution (in dpi) 

	note x resolution is set either by loadTiff() or by setXResolution()
	*/
	double xResolution() const {return m_dXResolution;}
	/*!
	returns the y resolution (in dpi)

	note y resolution is set either by loadTiff() or by setYResolution()
	*/
	double yResolution() const {return m_dYResolution;}
	/*!
	sets the x resolution (in dpi)
	*/
	void setXResolution(double res) {m_dXResolution=res;}
	/*!
	sets the y resolution (in dpi)
	*/
	void setYResolution(double res) {m_dYResolution=res;}
	/*!
	enable roi processing
	*/
	void enableRoi() const {;}
	/*!
	disable roi processing
	*/
	void disableRoi() {setRoi(m_FullRoi);}
	/*!
	tells if roi processing is turned on
	*/
	bool roiEnabled() const {return !(m_Roi==m_FullRoi);}
	/*!
	sets the roi rect

    passing a roi exceeding the image rect throws an imageError
	*/
	void setRoi(unsigned left,unsigned top,unsigned width,unsigned height)
		{return setRoi(uRect(left,top,width,height));}
	/*!
	sets the roi rect

    passing a roi exceeding the image rect throws an imageError

	take care: the roi is always set based on the non-roi image dims and coordinate system.
	*/
	void setRoi(const uRect& rRect);
	/*!
	sets the roi rect clipped to the image rect. you may pass in negative coors here.

	an exception is thrown if the roi passed is either completely outside the
	image rect or if width and/or height are negative.

	take care: the roi is always set based on the non-roi image dims and coordinate system.
	*/
	void setClippedRoi(const iRect& rRect);
	/*!
	sets the roi rect clipped to the image rect

    passing a roi exceeding the image rect throws an imageError
	*/
	void setClippedRoi(int left,int top,int width,int height)
		{
		return setClippedRoi(iRect(left,top,width,height));		
		}
	/*!
	gets the roi rect
	*/
	const uRect& getRoi(void) const {return m_Roi;}
	/*!
	pushes the roi on an internal stack

	note the roi is not changed
	*/
	void pushRoi();
	/*!
	pops the roi from an internal stack

	note the roi is changed

	throws an exception if the stack is empty, thus do not call this w/o any prior pushRoi() call
	*/
	void popRoi();
	/*!
	return the current id count

	note this is the current number of imagev2<T> instances.
	*/
	static unsigned id() {return m_uId;}
	/*!
	return the max id count

	note this is the number of possible imagev2<T> instances.
	*/
	static unsigned maxId() {return m_uMaxId;}
	/*!
	returns true if the image has borders
	*/
	bool hasBorders() const {return bool(m_uBorderWidth);}
	/*!
	returns the border width
	*/
	unsigned borderWidth() const {return m_uBorderWidth;}
	/*!
	extends the image edges to the borders, if hasBorders() 

	each border pixel is filled with the nearest non-border pixel

	note that currently this does work on all channels(), that is, preChannels(), 
	primChannels() and postChannels();
	*/
	void extendToBorders();
	/*!
	fills the borders, if hasBorders() 

	each border pixel is with the values passed

	extends the image edges to the borders, if hasBorders() 

	each border pixel is filled with the nearest non-border pixel

	note that currently this does work on all channels(), that is, preChannels(), 
	primChannels() and postChannels()

	note be sure to pass channels() values
	*/
	void fillBorders(const T* const values);
#if defined(IMIDEBUG)
	/*!
	returns the image name

	note you need to #define IMIDEBUG for this
	*/
	const string& name() const {return m_Name;}
	/*!
	sets the image name

	note you need to #define IMIDEBUG for this
	*/
	void setName(const string& name) {m_Name=name;}
#endif
protected:
	// object count
	static unsigned m_uId;
	static const unsigned m_uMaxId;
	// the maximum and minimum values in a channel
	static const T m_tMaxChannelValue;
	static const T m_tMinChannelValue;
	// the maximum width and height
	static const unsigned m_uMaxWidth;
	static const unsigned m_uMaxHeight;
	static const unsigned m_uMaxBorderWidth;
	bool m_bCreated;
	bool m_bBased;
	T* m_pFirstPixel;
	unsigned m_uChannels;
	unsigned m_uPrimChans;
	unsigned m_uPreChans;
	unsigned m_uPostChans;
	// the color space
	typename imagev2<T>::colorSpace m_eColorSpace;
	// a struct to pop and push the current channel state
	struct channelLayout{
		unsigned preChans;
		unsigned primChans;
		unsigned postChans;
	};
	// we can push and pop channel states
	stack<typename imagev2<T>::channelLayout> m_ChannelLayoutStack;
	void reset();
	/*!
	a sort of assignment:

	calls destroy() for this image, and, if rOther.created(), copies all members 
	including rOther.pFirstPixel() and destroys rOther

	this spares create() and memcpy() and is usefull if rOther is temporary

	note that this takes the whole image wether rOther.roiEnabled() or not.
	anyway, if rOther.roiEnabled(), this will have (the same) roiEnabled() afterwards.
	*/
	void take(imagev2<T>& rOther);

	// the resolution of the image in dpi
	double m_dXResolution;
	double m_dYResolution;

	// roi processing
	// the full image as a rect
	uRect m_FullRoi;
	// the roi as a rect
	uRect m_Roi;

	T* m_pRoiFirstPixel;

	// we can push and pop roi's, now...
	stack<uRect> m_roiStack;

	// the width of the borders, if any
	unsigned m_uBorderWidth;
	// note image mem is allocated here, this may include borders
	T* m_pAlloc;

#if defined(IMIDEBUG)
	string m_Name;
#endif

private:
	// no assignment operator, no copy ctor
	void operator=(const imagev2<T>& rOther) {;}
	imagev2(const imagev2<T>& rOther) {;}

	// offset from last pixel in a roi line
	// to first pixel in next roi line
	unsigned m_uRoiLineWrap;
};

// some types for simplicity
typedef imagev2<unsigned char> byteImage;
