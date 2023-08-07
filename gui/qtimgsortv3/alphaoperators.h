#include "imagev2.h"

/*!
a very 'rapid' over operator.

the FG image is composed onto the BG image for all pixels where the FG alpha channel is opaque at all, only if the FG image is fully transparent the BG
image is shown.

Note this is only useful for preview images.

Note all images have to be from the same template type...
*/
template <typename TIMG> class simpleOverCheckerBoard
{
public:
	simpleOverCheckerBoard();
	~simpleOverCheckerBoard(){;}
	/*!
	runs the operator

	if makesAlphaOpaque() returns true, the alpha channel of the image passed is opaque (i.e.
	imagev2<TIMG>::maxvalue()) afterwards.

	Note this throws if:
	
	- !pDest->hasAlpha()
	
	*/
	void run(imagev2<TIMG>* pDest);
	/*!
	sets the dark value
	*/
	void setDarkValue(TIMG val) {m_DarkValue=val;}
	/*!
	returns the dark value

	default: imagev2<TIMG>::maxvalue()*(1/3)
	*/
	TIMG darkValue() const {return m_DarkValue;}
	/*!
	sets the dark value
	*/
	void setBrightValue(TIMG val) {m_BrightValue=val;}
	/*!
	returns the bright value

	default: imagev2<TIMG>::maxvalue()*(2/3)
	*/
	TIMG brightValue() const {return m_DarkValue;}
	/*!
	if true passed, the alpha channel of the image passed to run() is made opaque
	*/
	void enableOpaqueAlpha(bool b) {m_bMakeAlphaOpaque=b;}
	/*!
	returns true if alpha is made opaque by run()

	default: false
	*/
	bool makesAlphaOpaque() const {return m_bMakeAlphaOpaque;}
	/*!
	sets the lenght of a check in pixels

	Note if 0 is passed, this is set to 1..-
	*/
	void setCheckLength(unsigned l) {m_uCheckLength=l; if(!m_uCheckLength) m_uCheckLength=1;} 
	/*!
	returns the length of a check
	*/
	unsigned checkLength() const {return m_uCheckLength;} 
protected:
	bool m_bMakeAlphaOpaque;
	unsigned m_uCheckLength;
	TIMG m_DarkValue;
	TIMG m_BrightValue;
	TIMG checkerVal(unsigned x,unsigned y) {
		TIMG val=m_BrightValue;
		if((x/m_uCheckLength)%2){
			val=m_DarkValue;
		}
		if((y/m_uCheckLength)%2){
			val=(val==m_DarkValue)?m_BrightValue:m_DarkValue;
		}
		return val;
	}
};

