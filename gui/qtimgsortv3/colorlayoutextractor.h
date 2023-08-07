#pragma once
#include "imagev2.h"
#include "array.h"
#include "geom.h"
#include "color.h"

/*!
the mpeg7 compliant color layout descriptor data
*/
typedef array<int> mpeg7ColorLayoutDescriptor;

/*!
a template class to extract color layout descriptors from an imagev2<TIMG>

note that the following expressions have to be true:

imagev2<TIMG>::primChannels()==3||imagev2<TIMG>::primChannels()==4

imagev2<TIMG>::width()>=8
imagev2<TIMG>::height()>=8

imagev2<TIMG>::isRGB()||imagev2<TIMG>::isBGR()||imagev2<TIMG>::isRGBA()
||imagev2<TIMG>::isABGR||imagev2<TIMG>::isBGRA||imagev2<TIMG>::isARGB()

otherwise init() throws...

TDATA is the type used for floating point calculations and shall be float or double.

extractMpeg7ColorLayout() extracts an mpeg7 compliant color layout descriptor. 
note that this is not implemented yet and throws...

extractColorLayout() extracts a similar descriptor. however, it's data elements are not casted to int but left
in TDATA and are not quantified. the size of the descriptor extracted can be set by calling setYCoeffs(), 
setCbCoeffs() and setCrCoeffs(). the chroma components can be multiplied with a factor.

note that if the image has an alpha channel and alphaMopde()==APPLY_ALPHA, the alpha channel is applied by 
mergings against bgColor() (which is black by default). the bg color can be set by setBgColor().

If alphaMode()==SKIP_ALPHA, pixels with alpha channel value of 100% are ignored. 
An image which is fully transparent will be treated as white...

If alphaMode()==IGNORE_ALPHA, the alpha channel is simply ignored.
*/
template <typename TIMG,typename TDATA> class colorLayoutExtractor
{
public:
	colorLayoutExtractor() : m_bInitialized(false), m_pImg(NULL), m_uYCoeffs(64),m_uCbCrCoeffs(64),m_uWidth(0),m_uHeight(0), m_BgColor(0,0,0), m_AlphaMode(SKIP_ALPHA) {;}
	~colorLayoutExtractor() {;}
	/*!
	describes the way alpha channel data is regarded

	IGNORE_ALPHA: alpha channel data is ignored, the image is treated as w/o alpha channel

	SKIP_ALPHA: (the default) when scaling down to the 8x8 thumbnail, pixels with 100% alpha are ignored, if a region contributing 
	to a single pixel in the 8x8 thumbnail consists of 100% alpha, the mean of the neighbour pixels in the 8x8 
	thumbnail is taken, if the whole image has 100% alpha, it is treated as 'white'

	APPLY_ALPHA: the alpha channel is applied by merging against the bgColor()
	*/
	enum alpha_mode{
		IGNORE_ALPHA=0,
		SKIP_ALPHA,		// default
		APPLY_ALPHA
	};
	/*!
	you need to call this again if you want to extract the color layout for more than one image
	*/
	void init(imagev2<TIMG>& rColImg);
	/*!
	the size of the returned feature descriptor
	*/
	unsigned featureLength() const {return m_uYCoeffs+2*m_uCbCrCoeffs;}
	/*!
	sets the number of Y Coeffs to be extracted

	note clamped silently to [1,64]

	default 64
	*/
	void setNumYCoeffs(unsigned num);
	/*!
	the number of Y Coeffs to be extracted
	*/
	unsigned numYCoeffs() const {return m_uYCoeffs;}
	/*!
	sets the number of Cb and Cr Coeffs to be extracted
	
	note clamped silently to [1,64]

	default 64
	*/
	void setNumCbCrCoeffs(unsigned num);
	/*!
	the number of Cb Cr Coeffs to be extracted
	*/
	unsigned numCbCrCoeffs() const {return m_uCbCrCoeffs;}
	/*!
	extracts the mpeg7 compliant color layout feature descriptor
	
	note it's the callers responsive to delete it

	note returns NULL on error
	*/
	mpeg7ColorLayoutDescriptor* extractMpeg7ColorLayout();
	/*!
	extracts the non compliant color layout feature descriptor
	
	note it's the callers responsive to delete it

	note throws on error
	*/
	array<TDATA>* extractColorLayout();
	/*!
	extracts the (non mpeg7 compliant) color layout feature descriptor
	
	note it's the callers responsive to pass enough space for numYCoeffs()+2*numCbCrCoeffs()
	values

	note that the chroma (Cb/Cr) coefficients are multiplied by chromafactor

	note throws on error
	*/
	void extractColorLayout(TDATA* pData,double chromaFactor=4.0);
	/*!
	extracts (non mpeg7 compliant) color layout feature descriptor
	
	note it's the callers responsive to pass enough space for numValues TDATA values
	values

	note that this calls extractColorLayout(TDATA* pData) internally and 
	does a re-sort of the output values. 

	note throws on error
	*/
	void extractOptimizedColorLayout(TDATA* pData,unsigned numValues=24,double chromaFactor=4.0);
	/*!
	sets the bg color

	default: black
	*/
	void setBgColor(const rgb<TIMG>& color) {m_BgColor=color;}
	/*!
	returns the bg color
	*/
	const rgb<TIMG>& bgColor() const {return m_BgColor;}
	/*!
	sets the alpha mode

	default: APPLY_ALPHA
	*/
	void setAlphaMode(alpha_mode mode) {m_AlphaMode=mode;}
	/*!
	returns the alpha mode
	*/
	alpha_mode alphaMode() const {return m_AlphaMode;}
protected:
	bool m_bInitialized;
	unsigned m_uYCoeffs;
	unsigned m_uCbCrCoeffs;
	unsigned m_uWidth;
	unsigned m_uHeight;
	imagev2<TIMG>* m_pImg;

	rgb<TIMG> m_BgColor;
	alpha_mode m_AlphaMode;
	bool m_bHasAlpha;

	TDATA m_AvgY[8][8];
	TDATA m_AvgCb[8][8];
	TDATA m_AvgCr[8][8];
	TDATA m_YCoeff[8][8];
	TDATA m_CbCoeff[8][8];
	TDATA m_CrCoeff[8][8];
	// here we store the roi's for the averaging
	// these can be re-used after init() and - if the image dims haven't changed - can stay...
	uRect m_Rois[8][8];
	// the index values for zizaggin...
	static const unsigned m_ZigZagIndex[64];

	// central processing used by the extract...() members above
	virtual void extractRawData();
	void extractRawDataSkipAlpha();

private:
	// not implemented:
	colorLayoutExtractor(const colorLayoutExtractor& rother);	
	colorLayoutExtractor& operator=(const colorLayoutExtractor& rother);
};

// a specialization for unsigned char, double
// we can do some calculations by LUTs here
// not this only is useful if you use the alpaMode() APPLY_ALPHA and you have a big set 
// of images
#include "features.h"
class byteColorLayoutExtractor: public colorLayoutExtractor<unsigned char,double>
{
public:
	byteColorLayoutExtractor() {;}
	~byteColorLayoutExtractor() {;}
	/*!
	sets the bg color

	default: black
	*/
	void setBgColor(const rgb<unsigned char>& color);
protected:
	byteMeanExtractor m_Meaner;
	// central processing used by the extract...() members above
	virtual void extractRawData();
private:
	// not implemented:
	byteColorLayoutExtractor(const byteColorLayoutExtractor& rother);	
	byteColorLayoutExtractor& operator=(const byteColorLayoutExtractor& rother);
	// the static tables for RGB -> YCbCr conversion
	// these save 576 multiplications per extractRawData() call...
	static double m_Y_red[256];
	static double m_Y_green[256];
	static double m_Y_blue[256];
	static double m_Cb_red[256];
	static double m_Cb_green[256];
	static double m_Cb_blue[256];
	static double m_Cr_red[256];
	static double m_Cr_green[256];
	static double m_Cr_blue[256];
	static const bool m_bTablesInitialized;
	static bool initializeTables();


};

