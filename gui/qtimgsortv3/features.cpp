#include "features.h"
#include "imageerror.h"
#include "leak.h"
template <typename T,typename TBIG> array<T>* meanExtractor<T,TBIG>::extract(imagev2<T>& rImg)
{
	if(!rImg.created())
		throw fqImageError(imageError::imageNotCreated);

	const unsigned chans=rImg.primChannels();
	assert(chans);

	array<T>* pData=new array<T>(chans);
	if(!pData)
		throw fqImageError(imageError::badAlloc);

	extract(rImg,*pData);

	return pData;
}

template <typename T,typename TBIG> void meanExtractor<T,TBIG>::extract(imagev2<T>& rImg,array<T>& rArr)
{
	if(!rImg.created())
		throw fqImageError(imageError::imageNotCreated);

	const unsigned chans=rImg.primChannels();
	assert(chans);
	if(rArr.size()!=chans)
		throw fqImageError(imageError::badParameter);

	array<TBIG> acc(chans,(TBIG)0);

	unsigned x,y,c;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned postChans=rImg.postChannels();
	const T *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				for(c=0;c<chans;++c){
					acc[c]+=*pPix++;
				}
				pPix+=postChans;
			}
			pPix+=wrap;
		}
	}
	else{
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				for(c=0;c<chans;++c){
					acc[c]+=*pPix++;
				}
			}
			pPix+=wrap;
		}
	}
	
	TBIG denom=(TBIG)w*(TBIG)h;
	for(c=0;c<chans;++c)
		rArr[c]=(T)(acc[c]/denom);

	return;
}

template <typename T,typename TBIG> bool meanExtractor<T,TBIG>::extractSkipAlpha(imagev2<T>& rImg,array<T>& rArr)
{
	if(!rImg.created()){
		throw fqImageError(imageError::imageNotCreated);
		return true;
	}

	// has to be an alpha rgb image format
	if(!(rImg.isABGR()||rImg.isARGB()||rImg.isBGRA()||rImg.isRGBA())){
		throw fqImageError(imageError::badChannelLayout);
		return true;
	}

	// this is for png curently
	// 0: fully transparent
	// maxChannelValue: fully opaque
	// const T alphaTreshhold=rImg.maxChannelValue();
	// const T alphaTreshhold=0;
	// assert(alphaTreshhold);

	// image is alpha RGB, see above
	const unsigned colchans=3;

	if(rArr.size()!=colchans){
		throw fqImageError(imageError::badParameter);
		return true;
	}

	const unsigned alphaChan=rImg.alphaChannel();
	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();
	// if ARGB or ABGR, adjust acc channel
	const unsigned accRedChan=(alphaChan)?redChan:redChan-1;
	const unsigned accGreenChan=(alphaChan)?greenChan:greenChan-1;
	const unsigned accBlueChan=(alphaChan)?blueChan:blueChan-1;
	assert(alphaChan!=0xFFFFFFFF);
	assert(redChan!=0xFFFFFFFF);
	assert(greenChan!=0xFFFFFFFF);
	assert(blueChan!=0xFFFFFFFF);

	array<TBIG> acc(colchans,(TBIG)0);

	unsigned x,y,c;
	unsigned nums=0;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned chans=rImg.channels();
	const unsigned postChans=rImg.postChannels();
	const T *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				//if(*(pPix+alphaChan)>alphaTreshhold){
				if(*(pPix+alphaChan)){
					acc[accRedChan]+=*(pPix+redChan);
					acc[accGreenChan]+=*(pPix+greenChan);
					acc[accBlueChan]+=*(pPix+blueChan);
					++nums;
					}
				pPix+=postChans;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{ 
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				//if(*(pPix+alphaChan)>alphaTreshhold){
				if(*(pPix+alphaChan)){
					acc[accRedChan]+=*(pPix+redChan);
					acc[accGreenChan]+=*(pPix+greenChan);
					acc[accBlueChan]+=*(pPix+blueChan);
					++nums;
				}
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}

	if(!nums)
		return false;

	for(c=0;c<colchans;++c)
		rArr[c]=(T)(acc[c]/(TBIG)(nums));

	return true;
}

template <typename T,typename TBIG> void meanExtractor<T,TBIG>::extractApplyAlpha(imagev2<T>& rImg,array<T>& rArr)
{
	// if the bg color is black, use the faster method
	if(m_BgColor.m_red==m_BgColor.m_blue==m_BgColor.m_green==(T)(0)){
		extractApplyAlphaBlackBG(rImg,rArr);
		return;
	}

	if(!rImg.created()){
		throw fqImageError(imageError::imageNotCreated);
	}

	// has to be an alpha rgb image format
	if(!(rImg.isABGR()||rImg.isARGB()||rImg.isBGRA()||rImg.isRGBA())){
		throw fqImageError(imageError::badChannelLayout);
	}

	// image is alpha RGB, see above
	const unsigned colchans=3;

	if(rArr.size()!=colchans){
		throw fqImageError(imageError::badParameter);
	}

	const unsigned alphaChan=rImg.alphaChannel();
	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();
	// if ARGB or ABGR, adjust acc channel
	const unsigned accRedChan=(alphaChan)?redChan:redChan-1;
	const unsigned accGreenChan=(alphaChan)?greenChan:greenChan-1;
	const unsigned accBlueChan=(alphaChan)?blueChan:blueChan-1;
	assert(alphaChan!=0xFFFFFFFF);
	assert(redChan!=0xFFFFFFFF);
	assert(greenChan!=0xFFFFFFFF);
	assert(blueChan!=0xFFFFFFFF);

	array<TBIG> acc(colchans,(TBIG)0);
	TBIG fBG,fFG;
	const TBIG maxval=rImg.maxChannelValue();
	assert(maxval);
	const TBIG redBG=m_BgColor.m_red;
	const TBIG greenBG=m_BgColor.m_green;
	const TBIG blueBG=m_BgColor.m_blue;

	unsigned x,y,c;
	unsigned nums=0;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned chans=rImg.channels();
	const unsigned postChans=rImg.postChannels();
	const T *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				// note for small types T, we can precalculate these to LUTs
				fBG=(maxval-(TBIG)(*(pPix+alphaChan)))/maxval;
				fFG=(TBIG)(1)-fBG;
				acc[accRedChan]+=(TBIG)*(pPix+redChan)*fFG+redBG*fBG;
				acc[accGreenChan]+=(TBIG)*(pPix+greenChan)*fFG+greenBG*fBG;
				acc[accBlueChan]+=(TBIG)*(pPix+blueChan)*fFG+blueBG*fBG;
				pPix+=postChans;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{ 
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				fBG=(maxval-(TBIG)(*(pPix+alphaChan)))/maxval;
				fFG=(TBIG)(1)-fBG;
				acc[accRedChan]+=(TBIG)*(pPix+redChan)*fFG+redBG*fBG;
				acc[accGreenChan]+=(TBIG)*(pPix+greenChan)*fFG+greenBG*fBG;
				acc[accBlueChan]+=(TBIG)*(pPix+blueChan)*fFG+blueBG*fBG;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}

	const TBIG denom=(TBIG)w*(TBIG)h;
	for(c=0;c<colchans;++c)
		rArr[c]=(T)(acc[c]/denom);

	return;
}

template <typename T,typename TBIG> void meanExtractor<T,TBIG>::extractApplyAlphaBlackBG(imagev2<T>& rImg,array<T>& rArr)
{
	if(!rImg.created()){
		throw fqImageError(imageError::imageNotCreated);
	}

	// has to be an alpha rgb image format
	if(!(rImg.isABGR()||rImg.isARGB()||rImg.isBGRA()||rImg.isRGBA())){
		throw fqImageError(imageError::badChannelLayout);
	}

	// image is alpha RGB, see above
	const unsigned colchans=3;

	if(rArr.size()!=colchans){
		throw fqImageError(imageError::badParameter);
	}

	const unsigned alphaChan=rImg.alphaChannel();
	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();
	// if ARGB or ABGR, adjust acc channel
	const unsigned accRedChan=(alphaChan)?redChan:redChan-1;
	const unsigned accGreenChan=(alphaChan)?greenChan:greenChan-1;
	const unsigned accBlueChan=(alphaChan)?blueChan:blueChan-1;
	assert(alphaChan!=0xFFFFFFFF);
	assert(redChan!=0xFFFFFFFF);
	assert(greenChan!=0xFFFFFFFF);
	assert(blueChan!=0xFFFFFFFF);

	array<TBIG> acc(colchans,(TBIG)0);
	TBIG fFG;
	const TBIG maxval=rImg.maxChannelValue();
	assert(maxval);

	unsigned x,y,c;
	unsigned nums=0;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned chans=rImg.channels();
	const unsigned postChans=rImg.postChannels();
	const T *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				// note for small types T, we can precalculate these to LUTs
				fFG=(TBIG)(1)-((maxval-(TBIG)(*(pPix+alphaChan)))/maxval);
				acc[accRedChan]+=(TBIG)*(pPix+redChan)*fFG;
				acc[accGreenChan]+=(TBIG)*(pPix+greenChan)*fFG;
				acc[accBlueChan]+=(TBIG)*(pPix+blueChan)*fFG;
				pPix+=postChans;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{ 
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				fFG=(TBIG)(1)-((maxval-(TBIG)(*(pPix+alphaChan)))/maxval);
				acc[accRedChan]+=(TBIG)*(pPix+redChan)*fFG;
				acc[accGreenChan]+=(TBIG)*(pPix+greenChan)*fFG;
				acc[accBlueChan]+=(TBIG)*(pPix+blueChan)*fFG;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}

	const TBIG denom=(TBIG)w*(TBIG)h;
	for(c=0;c<colchans;++c)
		rArr[c]=(T)(acc[c]/denom);

	return;
}

// implementation of byteMeanExtractor
byteMeanExtractor::byteMeanExtractor()
{
	calculateLUTs();
}

void byteMeanExtractor::setBgColor(const byteRGB& color)
{
	if(m_BgColor!=color){
		m_BgColor=color;
		// calculate BG FG factor LUTs
		calculateLUTs();
	}
}

void byteMeanExtractor::calculateLUTs()
{
	unsigned i;
	const double maxval=255.;

	for(i=0;i<256;i++){
		double bg=(maxval-(double)(i))/maxval;
		m_BgRedLUT[i]=(double)m_BgColor.m_red*bg;
		m_BgGreenLUT[i]=(double)m_BgColor.m_green*bg;
		m_BgBlueLUT[i]=(double)m_BgColor.m_blue*bg;
		m_FgLUT[i]=1.0-bg;
	}
}

void byteMeanExtractor::extractApplyAlpha(imagev2<unsigned char>& rImg,array<unsigned char>& rArr)
{
	// if the bg color is black, use the faster method
	if(m_BgColor.m_red==m_BgColor.m_blue==m_BgColor.m_green==(unsigned char)(0)){
		extractApplyAlphaBlackBG(rImg,rArr);
		return;
	}

	if(!rImg.created()){
		throw fqImageError(imageError::imageNotCreated);
	}

	// has to be an alpha rgb image format
	if(!(rImg.isABGR()||rImg.isARGB()||rImg.isBGRA()||rImg.isRGBA())){
		throw fqImageError(imageError::badChannelLayout);
	}

	// image is alpha RGB, see above
	const unsigned colchans=3;

	if(rArr.size()!=colchans){
		throw fqImageError(imageError::badParameter);
	}

	const unsigned alphaChan=rImg.alphaChannel();
	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();
	// if ARGB or ABGR, adjust acc channel
	const unsigned accRedChan=(alphaChan)?redChan:redChan-1;
	const unsigned accGreenChan=(alphaChan)?greenChan:greenChan-1;
	const unsigned accBlueChan=(alphaChan)?blueChan:blueChan-1;
	assert(alphaChan!=0xFFFFFFFF);
	assert(redChan!=0xFFFFFFFF);
	assert(greenChan!=0xFFFFFFFF);
	assert(blueChan!=0xFFFFFFFF);

	array<double> acc(colchans,(double)0);

	double fFG;
	unsigned x,y,c;
	unsigned nums=0;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned chans=rImg.channels();
	const unsigned postChans=rImg.postChannels();
	const unsigned char *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				fFG=m_FgLUT[*(pPix+alphaChan)];
				acc[accRedChan]+=(double)*(pPix+redChan)*fFG+m_BgRedLUT[*(pPix+alphaChan)];
				acc[accGreenChan]+=(double)*(pPix+greenChan)*fFG+m_BgGreenLUT[*(pPix+alphaChan)];
				acc[accBlueChan]+=(double)*(pPix+blueChan)*fFG+m_BgBlueLUT[*(pPix+alphaChan)];
				pPix+=postChans;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{ 
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				fFG=m_FgLUT[*(pPix+alphaChan)];
				acc[accRedChan]+=(double)*(pPix+redChan)*fFG+m_BgRedLUT[*(pPix+alphaChan)];
				acc[accGreenChan]+=(double)*(pPix+greenChan)*fFG+m_BgGreenLUT[*(pPix+alphaChan)];
				acc[accBlueChan]+=(double)*(pPix+blueChan)*fFG+m_BgBlueLUT[*(pPix+alphaChan)];
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}

	const double denom=(double)w*(double)h;
	for(c=0;c<colchans;++c)
		rArr[c]=(unsigned char)(acc[c]/denom);

	return;
}

void byteMeanExtractor::extractApplyAlphaBlackBG(imagev2<unsigned char>& rImg,array<unsigned char>& rArr)
{
	if(!rImg.created()){
		throw fqImageError(imageError::imageNotCreated);
	}

	// has to be an alpha rgb image format
	if(!(rImg.isABGR()||rImg.isARGB()||rImg.isBGRA()||rImg.isRGBA())){
		throw fqImageError(imageError::badChannelLayout);
	}

	// image is alpha RGB, see above
	const unsigned colchans=3;

	if(rArr.size()!=colchans){
		throw fqImageError(imageError::badParameter);
	}

	const unsigned alphaChan=rImg.alphaChannel();
	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();
	// if ARGB or ABGR, adjust acc channel
	const unsigned accRedChan=(alphaChan)?redChan:redChan-1;
	const unsigned accGreenChan=(alphaChan)?greenChan:greenChan-1;
	const unsigned accBlueChan=(alphaChan)?blueChan:blueChan-1;
	assert(alphaChan!=0xFFFFFFFF);
	assert(redChan!=0xFFFFFFFF);
	assert(greenChan!=0xFFFFFFFF);
	assert(blueChan!=0xFFFFFFFF);

	array<double> acc(colchans,(double)0);
	double fFG;

	unsigned x,y,c;
	unsigned nums=0;
	const unsigned w=rImg.width();
	const unsigned h=rImg.height();
	const unsigned wrap=rImg.lineWrap();
	const unsigned preChans=rImg.preChannels();
	const unsigned chans=rImg.channels();
	const unsigned postChans=rImg.postChannels();
	const unsigned char *pPix=rImg.pcFirstPixel();
	if(preChans||postChans){
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				pPix+=preChans;
				fFG=m_FgLUT[*(pPix+alphaChan)];
				acc[accRedChan]+=(double)*(pPix+redChan)*fFG;
				acc[accGreenChan]+=(double)*(pPix+greenChan)*fFG;
				acc[accBlueChan]+=(double)*(pPix+blueChan)*fFG;
				pPix+=postChans;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}
	else{ 
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				fFG=m_FgLUT[*(pPix+alphaChan)];
				acc[accRedChan]+=(double)*(pPix+redChan)*fFG;
				acc[accGreenChan]+=(double)*(pPix+greenChan)*fFG;
				acc[accBlueChan]+=(double)*(pPix+blueChan)*fFG;
				pPix+=chans;
			}
			pPix+=wrap;
		}
	}

	const double denom=(double)w*(double)h;
	for(c=0;c<colchans;++c)
		rArr[c]=(unsigned char)(acc[c]/denom);

	return;
}



// excplicit instantiation
#include "features.expl_templ_inst"

