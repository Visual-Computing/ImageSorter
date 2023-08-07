#include "alphaoperators.h"
#include "imageerror.h"

template <typename TIMG> simpleOverCheckerBoard<TIMG>::simpleOverCheckerBoard()
{
	m_DarkValue=(TIMG)((double)(imagev2<TIMG>::maxChannelValue())/3.);
	m_BrightValue=(TIMG)((double)(imagev2<TIMG>::maxChannelValue())*2./3.);
	m_bMakeAlphaOpaque=false;
	m_uCheckLength=32;
}

template <typename TIMG> void simpleOverCheckerBoard<TIMG>::run(imagev2<TIMG>* pDst)
{
	if(!pDst->hasAlpha())
		throw fqImageError(imageError::badColorSpace);

	const unsigned w=pDst->width();
	const unsigned h=pDst->height();

	const unsigned alphaChan=pDst->alphaChannel();
	// the alpha channel is either the first or the last...
	assert(alphaChan==0||alphaChan==3);

	// ROI aware
	const unsigned wrap=pDst->lineWrap();
	TIMG *pDstPix=pDst->pFirstPixel();
	TIMG val;

	unsigned x,y;
	if(m_bMakeAlphaOpaque){
		const TIMG opaqueVal=pDst->maxChannelValue();
		if(!alphaChan){
			for(y=0;y<h;++y)
				for(x=0;x<w;++x){
					if(!(*pDstPix)){
						*pDstPix++=opaqueVal;
						val=checkerVal(x,y);
						*pDstPix++=val;
						*pDstPix++=val;
						*pDstPix++=val;
					}
					else{
						*pDstPix++=opaqueVal;
						pDstPix+=3;
					}
					pDstPix+=wrap;
				}
		}
		else{
			for(y=0;y<h;++y)
				for(x=0;x<w;++x){
					if(!(*(pDstPix+alphaChan))){
						val=checkerVal(x,y);
						*pDstPix++=val;
						*pDstPix++=val;
						*pDstPix++=val;
						*pDstPix++=opaqueVal;
					}
					else{
						pDstPix+=3;
						*pDstPix++=opaqueVal;

					}
					pDstPix+=wrap;
				}
		}

	}
	else{
		if(!alphaChan){
			for(y=0;y<h;++y)
				for(x=0;x<w;++x){
					if(!(*pDstPix++)){
						val=checkerVal(x,y);
						*pDstPix++=val;
						*pDstPix++=val;
						*pDstPix++=val;
					}
					else{
						pDstPix+=3;
					}
					pDstPix+=wrap;
				}
			}
		else{
			for(y=0;y<h;++y)
				for(x=0;x<w;++x){
					if(!(*(pDstPix+alphaChan))){
						val=checkerVal(x,y);
						*pDstPix++=val;
						*pDstPix++=val;
						*pDstPix++=val;
						++pDstPix;
					}
					else{
						pDstPix+=4;
					}
					pDstPix+=wrap;
			}
		}
	}
}

// excplicit instantiation
#include "alphaoperators.expl_templ_inst"

