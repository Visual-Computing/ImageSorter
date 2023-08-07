#include "imagev2.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <vector>
#include <stdlib.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
// use inline memcpy() for this source file
#pragma intrinsic(memcpy)

// initialize static const values
// overloads for specific types T

#if defined(_MSC_VER)
const float imagev2<float>::m_tMaxChannelValue=FLT_MAX;
const float imagev2<float>::m_tMinChannelValue=-FLT_MAX;
const double imagev2<double>::m_tMaxChannelValue=DBL_MAX;
const double imagev2<double>::m_tMinChannelValue=-DBL_MAX;
#elif defined(__GNUC__)
template <class T> const float imagev2<float>::m_tMaxChannelValue=FLT_MAX;
template <class T> const float imagev2<float>::m_tMinChannelValue=-FLT_MAX;
template <class T> const double imagev2<double>::m_tMaxChannelValue=DBL_MAX;
template <class T> const double imagev2<double>::m_tMinChannelValue=-DBL_MAX;
#else
#error neither GNU nor MSC compiler
#endif

// note that this works only for unsigned integral types T
template <class T> const T imagev2<T>::m_tMaxChannelValue=(T)(pow(2.0,((double)(sizeof(T)*8)))-1.0);
template <class T> const T imagev2<T>::m_tMinChannelValue=(T)0;
template <class T> const unsigned imagev2<T>::m_uMaxBorderWidth=10;
template <class T> const unsigned imagev2<T>::m_uMaxWidth=16384;
template <class T> const unsigned imagev2<T>::m_uMaxHeight=16384;
template <class T> unsigned imagev2<T>::m_uId=0;
template <class T> const unsigned imagev2<T>::m_uMaxId=(unsigned)(pow(2.0,((double)(sizeof(unsigned)*8)))-1.0);

template <class T> imagev2<T>::imagev2(void)
{
	if(m_uId==m_uMaxId)
		throw fqImageError(imageError::imageIdExceed);
	++m_uId;
	reset();
}

template <class T> imagev2<T>::~imagev2(void)
{
	m_uId--;
	destroy();
}

template <class T> void imagev2<T>::reset(void)
{
	m_bCreated=false;
	m_pAlloc=NULL;
	m_pFirstPixel=NULL;
	m_uChannels=0;
	m_uPrimChans=0;
	m_uPreChans=0;
	m_uPostChans=0;
	m_bBased=false;
#pragma message("correct code for tiff resolution, current resolution is 300 dpi always")
	m_dXResolution=300.;
	m_dYResolution=300.;
	m_FullRoi.left=0;
	m_FullRoi.top=0;
	m_FullRoi.width=0;
	m_FullRoi.height=0;
	m_Roi.left=0;
	m_Roi.top=0;
	m_Roi.width=0;
	m_Roi.height=0;
	m_pRoiFirstPixel=NULL;
	m_uRoiLineWrap=0;
	// clear stacks
	// why can't is call the underlying deque?
	// m_roiStack.clear()
	while(!m_roiStack.empty())
		m_roiStack.pop();
	while(!m_ChannelLayoutStack.empty())
		m_ChannelLayoutStack.pop();

	// no borders
	m_uBorderWidth=0;

	// no color space
	m_eColorSpace=colorSpaceNone;
}

template <class T> void imagev2<T>::destroy()
{
	// note new behaviour: delete m_pAlloc, not m_pFirstPixel
	if(m_bCreated){
		if(isBased()){		// we don't own the image mem... 
			m_pAlloc=NULL;
		}
		else if(m_pAlloc){	// NOTE this is *not* redundant, see take()
			delete[] m_pAlloc;
			m_pAlloc=NULL;
		}
	}
	assert(m_pAlloc==NULL);
	reset();
}

template <class T> void imagev2<T>::create(unsigned width,
										 unsigned height,
										 unsigned primChans,
										 unsigned borderWidth,
										 unsigned preChans,
										 unsigned postChans,
										 bool bBased,
										 T* pBase)
{
	// already done?
	if(created())
		throw fqImageError(imageError::alreadyCreated);
	
	// note if we have an alpha channel, this has to be either the first or the last channel...
	if(!width||!height||!primChans||(bBased&&pBase==NULL))
		throw fqImageError(imageError::badParameter);

	if(width>m_uMaxWidth||height>m_uMaxHeight)
		throw fqImageError(imageError::imageTooBig);

	if(borderWidth>m_uMaxBorderWidth)
		throw fqImageError(imageError::imageBorderTooBig);
	
	// currently, we support only single channel (grey)
	// or three channel (rgb) 
	// assert(primChans==1||primChans==3);

	m_uPrimChans=primChans;
	m_uPreChans=preChans;
	m_uPostChans=postChans;
	m_uChannels=m_uPrimChans+m_uPreChans+m_uPostChans;

	m_FullRoi.left=0;
	m_FullRoi.top=0;
	m_FullRoi.width=width;
	m_FullRoi.height=height;
	m_uBorderWidth=borderWidth;

	// NOTE this may throw an exception
	// m_pFirstPixel=new T[m_uNumComponents];
	// new behaviour: allocate to m_pAlloc, not to m_pFirstPixel
	// leave space for borders, if any

	// note new behaviour: don't alloc but use passed adress
	if(m_bBased=bBased){
		m_pAlloc=pBase;
	}
	else{
		m_pAlloc=new T[(width+2*m_uBorderWidth)*(height+2*m_uBorderWidth)*m_uChannels];
	}

	if(!m_pAlloc){
		reset();	// clean up other members...
		throw fqImageError(imageError::badAlloc);
	}
	
	// new behavoiur: set m_pFirstPixel according to borders, if any
	if(!hasBorders())
		m_pFirstPixel=m_pAlloc;
	else
		m_pFirstPixel=m_pAlloc+(m_uBorderWidth*(width+2*m_uBorderWidth)+m_uBorderWidth)*m_uChannels;


	m_bCreated=true;
	setRoi(m_FullRoi);
	
	// set color space
	// use rgb per default if 3 channels
	if(primChannels()==1)
		m_eColorSpace=colorSpaceGrey;
	else if(primChannels()==3)
		m_eColorSpace=colorSpaceRGB;
	else
		m_eColorSpace=colorSpaceNone;

	return;
}

template <class T> void imagev2<T>::setPreChannels(unsigned preChans)
{
	if(!created())
		throw imageError(imageError::imageNotCreated);

	//if(preChans>=primChannels()+postChannels())
	if(preChans>=channels()-postChannels())
		throw imageError(imageError::badParameter);

	m_uPreChans=preChans;
	m_uPrimChans=channels()-preChannels()-postChannels();
}

template <class T> void imagev2<T>::setPostChannels(unsigned postChans)
{
	if(!created())
		throw imageError(imageError::imageNotCreated);

	// if(postChans>=primChannels()+preChannels())
	if(postChans>=channels()-preChannels())
		throw imageError(imageError::badParameter);

	m_uPostChans=postChans;
	m_uPrimChans=channels()-preChannels()-postChannels();
}

template <class T> void imagev2<T>::pushChannelLayout()
{
	channelLayout layout;
	layout.preChans=preChannels();
	layout.primChans=primChannels();
	layout.postChans=postChannels();

	m_ChannelLayoutStack.push(layout);
}
template <class T> void imagev2<T>::popChannelLayout()
{
	if(m_ChannelLayoutStack.empty())
		throw fqImageError(imageError::badChannelLayoutStack);

	// note the funny stl stack: you need to call top() to access the last pushed elem...
	channelLayout layout=m_ChannelLayoutStack.top();
	// ... pop does not return this...
	m_ChannelLayoutStack.pop();
	m_uPreChans=layout.preChans;
	m_uPrimChans=layout.primChans;
	m_uPostChans=layout.postChans;

	assert(m_uPreChans+m_uPrimChans+m_uPostChans==m_uChannels);
}

template <class T> void imagev2<T>::setRoi(const uRect& rRect)
{
	if(!created())
		throw imageError(imageError::imageNotCreated);

	// be sure to test against m_FullRoi members like width, *not* width()
	if(rRect.left>=m_FullRoi.width||
	   rRect.top>=m_FullRoi.height||
	   rRect.left+rRect.width>m_FullRoi.width||
	   rRect.top+rRect.height>m_FullRoi.height||
	   !rRect.width||
	   !rRect.height)
		throw imageError(imageError::badRoi);

	m_Roi=rRect;

	m_pRoiFirstPixel=m_pFirstPixel+(m_Roi.top*(m_FullRoi.width+2*m_uBorderWidth)+m_Roi.left)*channels();
	m_uRoiLineWrap=(m_FullRoi.width+2*m_uBorderWidth-m_Roi.width)*channels();

	return;
}

template <class T> void imagev2<T>::setClippedRoi(const iRect& rRect)
{
	if(!created())
		throw imageError(imageError::imageNotCreated);

	// upper left corner below or right of image?
	// negative width/height?
	if(rRect.left>=(int)(m_FullRoi.width)||rRect.top>=(int)(m_FullRoi.height)||rRect.width<=0||rRect.height<=0)
		throw imageError(imageError::badRoi);

	int left=rRect.left,top=rRect.top,right=rRect.left+rRect.width,bottom=rRect.top+rRect.height;
	if(right<0||bottom<0)
		throw imageError(imageError::badRoi);

	// be sure to test against non-roi members like m_uWidth, *not* width()
	left=max(0,left);
	top=max(0,top);
	bottom=min(m_FullRoi.height,(unsigned)bottom);
	right=min(m_FullRoi.width,(unsigned)right);

	m_Roi.left=(unsigned)left;
	m_Roi.top=(unsigned)top;
	m_Roi.width=(unsigned)(right-left);
	m_Roi.height=(unsigned)(bottom-top);

	m_pRoiFirstPixel=m_pFirstPixel+(m_Roi.top*(m_FullRoi.width+2*m_uBorderWidth)+m_Roi.left)*channels();
	m_uRoiLineWrap=(m_FullRoi.width+2*m_uBorderWidth-m_Roi.width)*channels();

	return;
}

template <class T> void imagev2<T>::pushRoi()
{
	m_roiStack.push(m_Roi);	
}
template <class T> void imagev2<T>::popRoi()
{
	if(m_roiStack.empty())
		throw fqImageError(imageError::badRoiStack);

	// note the funny stl stack: you need to call top() to access the last pushed elem...
	uRect r=m_roiStack.top();
	// ... pop does not return this...
	m_roiStack.pop();
	setRoi(r);
}


template <class T> void imagev2<T>::fill(const T* const pValues)
{
	if(!created())
		throw fqImageError(imageError::imageNotCreated);

	if(!pValues)
		throw fqImageError(imageError::badParameter);

	// note for single channel byte images w/o roi, we may use memset()...
	/*
	better use greyImage<T>::fill(T val)...
	if(channels()==1&&channelByteDepth()==1&&!roiEnabled()){
		T val=*pValues;
		memset(pFirstPixel(),val,width()*height()));
		return;
	}
	*/

	// note memory may be non-contignous if roiEnabled()
	T* pPix=pFirstPixel();
	unsigned x,y;
	const unsigned w=width();
	const unsigned h=height();
	// note since we use memcpy(), we can handle post channels, too...
	const unsigned chans=primChannels()+postChannels();
	const unsigned preChans=preChannels();
	const unsigned wrap=lineWrap();
	// ... but take care to copy only primChannels() !!!
	const unsigned nbytes=primChannels()*sizeof(T);

	// general version:
	if(preChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				memcpy(pPix,pValues,nbytes);
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				memcpy(pPix,pValues,nbytes);
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	return;
}

template <class T> void imagev2<T>::fillCompared(const T* const pValues,const T* const pComp)
{
	if(!created())
		throw fqImageError(imageError::imageNotCreated);

	if(!pValues||!pComp)
		throw fqImageError(imageError::badParameter);

	// note for single channel byte images w/o roi, we may use memset()...
	/*
	better use greyImage<T>::fill(T val)...
	if(channels()==1&&channelByteDepth()==1&&!roiEnabled()){
		T val=*pValues;
		memset(pFirstPixel(),val,width()*height());
		return;
	}
	*/

	// note memory may be non-contignous if roiEnabled()
	// note works with alpha channel, too
	T* pPix=pFirstPixel();
	unsigned x,y;
	const unsigned w=width();
	const unsigned h=height();
	const unsigned chans=primChannels()+postChannels();
	const unsigned preChans=preChannels();
	const unsigned wrap=lineWrap();
	const unsigned nbytes=primChannels()*sizeof(T);

	if(preChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				if(!memcmp(pPix,pComp,nbytes))
					memcpy(pPix,pValues,nbytes);
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				if(!memcmp(pPix,pComp,nbytes))
					memcpy(pPix,pValues,nbytes);
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	return;
}

template <class T> template <class TMask> void imagev2<T>::fillMasked(const T* const pValues,
																	imagev2<TMask>& rMaskImg,
																	TMask maskVal)
{
	if(!created()||!rMaskImg.created())
		throw fqImageError(imageError::imageNotCreated);

	if(!pValues)
		throw fqImageError(imageError::badParameter);

	if(width()!=rMaskImg.width()||
		height()!=rMaskImg.height())
		throw fqImageError(imageError::imageNotSameDim);

	T* pPix=pFirstPixel();
	const TMask* pMaskPix=rMaskImg.pcFirstPixel();
	const unsigned w=width();
	const unsigned h=height();
	const TMask cMask=maskVal;
	const unsigned wrap=lineWrap();
	const unsigned maskWrap=rMaskImg.lineWrap();
	const unsigned chans=primChannels()+postChannels();
	const unsigned preChans=preChannels();
	const unsigned maskChans=rMaskImg.primChannels()+rMaskImg.postChannels();
	const unsigned maskPreChans=rMaskImg.preChannels();
	const unsigned nbytes=primChannels()*sizeof(T);
	unsigned x,y;

	if(preChans||maskPreChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				pMaskPix+=maskPreChans;
				if(*pMaskPix==cMask)
					memcpy(pPix,pValues,nbytes);
				pPix+=chans;
				pMaskPix+=maskChans;
			}
			pPix+=wrap;
			pMaskPix+=maskWrap;
		}
	}
	else{
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				if(*pMaskPix==cMask)
					memcpy(pPix,pValues,nbytes);
				pPix+=chans;
				pMaskPix+=maskChans;
			}
			pPix+=wrap;
			pMaskPix+=maskWrap;
		}
	}
}

template <class T> void imagev2<T>::copy(const imagev2<T>& rOther)
{
	// check if self reference
	if(&rOther==this)
		return;

	if(!created()||!rOther.created())
		throw fqImageError(imageError::imageNotCreated);

	// note this is hard: we can only copy to images with
	// same dims and channels. this propably has to be changed
	if(!sameLayout(rOther))
		throw fqImageError(imageError::imageNotSameLayout);
	// just copy memory, do *not* copy dimension relevant members
	// these are o.k. since sameLayout(), furthermore 
	// this or rOther may have roiEnabled()...

	// fast versions for non-pre non-post channels in both src and dest
	if(!preChannels()&&
	   !rOther.preChannels()&&
	   !postChannels()&&
	   !rOther.postChannels()){
			// copy memory
			// faster non-roi non-border version:
			if(!roiEnabled()&&
			   !rOther.roiEnabled()&&
			   !hasBorders()&&
			   !rOther.hasBorders()){
				memcpy(pFirstPixel(),rOther.pcFirstPixel(),width()*height()*channels()*channelByteDepth());
				return;
			}
	
			// line copy version for any roi
			T* pDst=pFirstPixel();
			const T* pSrc=rOther.pcFirstPixel();
			unsigned y;
			// note sameLayout()...
			const unsigned h=height();
			const unsigned linesize=width()*channels()*channelByteDepth();
			// ...but propably not same lineoffset !!!
			const unsigned srcLineOffset=rOther.lineOffset();
			const unsigned dstLineOffset=lineOffset();
			for(y=0;y<h;++y){
				memcpy(pDst,pSrc,linesize);
				pDst+=dstLineOffset;
				pSrc+=srcLineOffset;
			}

			return;
	}
	else{
		T* pDst=pFirstPixel();
		const T* pSrc=rOther.pcFirstPixel();
		const unsigned h=height();
		const unsigned w=width();
		const unsigned preChans=preChannels();
		const unsigned chans=primChannels()+postChannels();
		const unsigned nbytes=primChannels()*sizeof(T);
		const unsigned srcPreChans=rOther.preChannels();
		const unsigned srcChans=rOther.primChannels()+rOther.postChannels();
		const unsigned wrap=lineWrap();
		const unsigned srcWrap=rOther.lineWrap();
		unsigned x,y;
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pDst+=preChans;
				pSrc+=srcPreChans;
				memcpy(pDst,pSrc,nbytes);
				pDst+=chans;
				pSrc+=srcChans;
			}
			pDst+=wrap;
			pSrc+=srcWrap;
		}
	}
}

// take() first destroys this image, copies all members
// from rOther and then destroys rOther
// use this instead of operator= if rOther is i.e. temporary
// this spares create() and memcpy()
template <class T> void imagev2<T>::take(imagev2<T>& rOther)
{
	// clear this image
	destroy();

	// overtake rOther
	if(rOther.created()){
		m_bCreated=rOther.created();
		m_uChannels=rOther.channels();
		// note do *not* call rOther.height() which may return the roi height etc.
		// but copy plain members
		m_uBorderWidth=rOther.m_uBorderWidth;

		// note: take the memory
		m_pAlloc=rOther.m_pAlloc;
		m_pFirstPixel=rOther.m_pFirstPixel;

		// new: alpha channel and based
		m_bBased=rOther.m_bBased;
		m_uPreChans=rOther.m_uPreChans;
		m_uPrimChans=rOther.m_uPrimChans;
		m_uPostChans=rOther.m_uPostChans;
		// copy roi members
		m_FullRoi=rOther.m_FullRoi;
		// copy stacks
		m_roiStack=rOther.m_roiStack;
		m_ChannelLayoutStack=rOther.m_ChannelLayoutStack;

		m_Roi=rOther.getRoi();
		m_pRoiFirstPixel=rOther.m_pRoiFirstPixel;
		m_uRoiLineWrap=rOther.m_uRoiLineWrap;

		// note: see destroy()
		rOther.m_pAlloc=NULL;
		rOther.destroy();
	}

	return;
}

template <class T> void imagev2<T>::extendToBorders()
{
	if(!created())
		throw fqImageError(imageError::imageNotCreated);

	if(!hasBorders())
		return;

	// note this works on all channels currently
	pushChannelLayout();
	setPreChannels(0);
	setPostChannels(0);

	// note we cannot use the roi here, because we cannot set the roi to the borders
	// thus work on non roi-ed members
	
	const unsigned bw=borderWidth();
	const unsigned lo=lineOffset();
	const unsigned h=m_FullRoi.height;
	const unsigned w=m_FullRoi.width;
	const unsigned ch=channels();
	const T* pSrc;
	T* pDst;
	unsigned i,j;

	// expand top line
	pDst=m_pAlloc+bw*ch;
	for(i=0;i<bw;++i){
		memcpy(pDst,m_pFirstPixel,w*ch*channelByteDepth());
		pDst+=lo;
	}

	// expand bootom line
	pDst=m_pFirstPixel+h*lo;
	pSrc=pDst-lo;
	for(i=0;i<bw;++i){
		memcpy(pDst,pSrc,w*ch*channelByteDepth());
		pDst+=lo;
	}

	// expand left column
	pDst=m_pAlloc+bw*lo;
	pSrc=m_pFirstPixel;
	for(i=0;i<h;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
		pSrc+=lo;
	}

	// expand right column
	pDst=m_pAlloc+bw*lo+bw*ch+w*ch;
	pSrc=pDst-ch;
	for(i=0;i<h;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
		pSrc+=lo;
	}

	// expand top left corner
	pSrc=m_pFirstPixel;
	pDst=m_pAlloc;
	for(i=0;i<bw;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
	}

	// expand top right corner
	pSrc=m_pFirstPixel+w*ch-ch;
	pDst=m_pAlloc+bw*ch+w*ch;
	for(i=0;i<bw;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
	}

	// expand bottom left corner
	pSrc=m_pFirstPixel+h*lo-lo;
	pDst=m_pAlloc+h*lo+bw*lo;
	for(i=0;i<bw;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
	}

	// expand bottom right corner
	pSrc=m_pFirstPixel+h*lo-2*bw*ch;
	pDst=m_pAlloc+bw*lo+h*lo+bw*ch+w*ch;
	for(i=0;i<bw;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pSrc,ch*channelByteDepth());
			pDst+=ch;
		}
		pDst+=lo-bw*ch;
	}

	// reset channel layout
	popChannelLayout();
}

template <class T> void imagev2<T>::fillBorders(const T* const pValues)
{
	if(!created())
		throw fqImageError(imageError::imageNotCreated);

	if(!hasBorders())
		return;

	// note this works on all channels currently
	pushChannelLayout();
	setPreChannels(0);
	setPostChannels(0);

	// note we cannot use the roi here, because we cannot set the roi to the borders
	// thus work on non roi-ed members
	
	const unsigned bw=borderWidth();
	const unsigned lo=lineOffset();
	const unsigned h=m_FullRoi.height;
	const unsigned w=m_FullRoi.width;
	const unsigned w2=w+2*bw;
	const unsigned ch=channels();
	const unsigned chn=ch*channelByteDepth();

	T* pDst;
	unsigned i,j;

	// set top lines and upper left/right corner
	pDst=m_pAlloc;
	for(i=0;i<bw;++i){
		for(j=0;j<w2;++j){
			memcpy(pDst,pValues,chn);
			pDst+=ch;
		}
	}

	// set left /right column
	// note pDst already in position...
	for(i=0;i<h;++i){
		for(j=0;j<bw;++j){
			memcpy(pDst,pValues,chn);
			pDst+=ch;
		}
		pDst+=w;
		for(j=0;j<bw;++j){
			memcpy(pDst,pValues,chn);
			pDst+=ch;
		}
	}

	// set bottom lines and lower left/right corner
	// note pDst already in position...
	for(i=0;i<bw;++i){
		for(j=0;j<w2;++j){
			memcpy(pDst,pValues,chn);
			pDst+=ch;
		}
	}

	// reset channel layout
	popChannelLayout();
}

template <class T> unsigned imagev2<T>::blueChannel() const
{
	if(m_uChannels<3)
		return 0xFFFFFFFF;

	switch(m_eColorSpace){
			case colorSpaceRGBA:
			case colorSpaceRGB:
				return 2;
				break;
			case colorSpaceBGR:
			case colorSpaceBGRA:		
				return 0;
				break;
			case colorSpaceARGB:
				return 3;
				break;
			case colorSpaceABGR:
				return 1;
				break;
			default:
				break;
	}
	return 0xFFFFFFFF;
}

template <class T> unsigned imagev2<T>::greenChannel() const
{
	if(m_uChannels<3)
		return 0xFFFFFFFF;

	switch(m_eColorSpace){
			case colorSpaceRGB:
			case colorSpaceBGR:
			case colorSpaceBGRA:
			case colorSpaceRGBA:
				return 1;
				break;
			case colorSpaceABGR:
			case colorSpaceARGB:
				return 2;
				break;
			default:
				break;
	}
	return 0xFFFFFFFF;
}

template <class T> unsigned imagev2<T>::redChannel() const
{
	if(m_uChannels<3)
		return 0xFFFFFFFF;

	switch(m_eColorSpace){
			case colorSpaceRGBA:
			case colorSpaceRGB:
				return 0;
				break;
			case colorSpaceBGR:
			case colorSpaceBGRA:		
				return 2;
				break;
			case colorSpaceARGB:
				return 1;
				break;
			case colorSpaceABGR:
				return 3;
				break;
			default:
				break;
	}
	return 0xFFFFFFFF;
}

template <class T> unsigned imagev2<T>::alphaChannel() const
{
	if(m_uChannels<4)
		return 0xFFFFFFFF;

	switch(m_eColorSpace){
			case colorSpaceARGB:
			case colorSpaceABGR:
				return 0;
				break;
			case colorSpaceRGBA:
			case colorSpaceBGRA:		
				return 3;
				break;
			default:
				break;
	}
	return 0xFFFFFFFF;
}



// force instantiation
// note imagev2<T> is used by different projects with different needs concerning type T
// thus have a simpe text file with your needs in the path
#include "imagev2.expl_templ_inst"
#pragma function(memcpy)
