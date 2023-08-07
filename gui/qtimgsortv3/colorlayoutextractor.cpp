#include "colorlayoutextractor.h"
#include "imageerror.h"
#include "features.h"
#include "dct8x8.h"
#include "leak.h"
/*
note we want the result(s per channel) to have the following zig zag order:

			 0   1   2   3   4   5   6   7
			 ------------------------------
		   0|0,  1,  5,  6,  14, 15, 27, 28,
	       1|2,  4,  7,  13, 16, 26, 29, 42,
	       2|3,  8,  12, 17, 25, 30, 41, 43,
	       3|9,  11, 18, 24, 31, 40, 44, 53,
	       4|10, 19, 23, 32, 39, 45, 52, 54,
	       5|20, 22, 33, 38, 46, 51, 55, 60,
	       6|21, 34, 37, 47, 50, 56, 59, 61,
	       7|35, 36, 48, 49, 57, 58, 62, 63

that is, if the above is the resulting 8*8 dct in a channel, the first element in the output is [0][0], 
the second [0][1], the third [1][0] and so on...

thus, we adress the 8x8 dct as an consecutive (!!!) array of 64 values and copy from it to the outputs
by the following index values:
*/
#pragma message("test the zigzag scan index...")

template <typename TIMG,typename TDATA> const unsigned colorLayoutExtractor<TIMG,TDATA>::m_ZigZagIndex[64]={
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	48, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
	};


template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::init(imagev2<TIMG>& rImg)
{
	if(!rImg.created())
		throw fqImageError(imageError::imageNotCreated);

	// note this test for 3 or 4  primChannels(), too...
	if(!(rImg.isRGB()||rImg.isBGR()||rImg.isRGBA()
		 ||rImg.isABGR()||rImg.isBGRA()||rImg.isARGB()))
		throw fqImageError(imageError::badColorSpace);

	m_bHasAlpha=!(rImg.isRGB()||rImg.isBGR());

	// check if roi's have to be recalculated
	bool bKeepRois=m_bInitialized?(m_uWidth==rImg.width()&&m_uHeight==rImg.height()):false;

	m_bInitialized=false;
	m_pImg=&rImg;	

	m_uWidth=m_pImg->width();
	m_uHeight=m_pImg->height();

	// bug fix: when passing in images smaller than 8 pixels, 
	// bad rois are generated
	if(m_uWidth<8||m_uHeight<8)
		throw fqImageError(imageError::imageBadSize);

	if(!bKeepRois){
		// compute averaging roi's
		// we partition the image in 64 cells with approx. size
		// width()/8 and height()/8 
		// note depending on the remainders for x and y direction
		// we have to distribute bigger cells on each of the axes
		unsigned x,y;
		unsigned w=m_uWidth/8;
		unsigned wRem=m_uWidth%8;
		bool bIncW=true;
		if(wRem>4){
			++w;
			wRem=8-wRem;
			bIncW=false;
		}
		unsigned h=m_uHeight/8;
		unsigned hRem=m_uHeight%8;
		bool bIncH=true;
		if(hRem>4){
			++h;
			hRem=8-hRem;
			bIncH=false;
		}

		unsigned top=0,left,width,height=h,_wRem;

		for(y=0;y<8;++y){
			left=0;
			width=w;
			_wRem=wRem;
			for(x=0;x<8;++x){
				m_Rois[y][x].top=top;
				m_Rois[y][x].left=left;
				m_Rois[y][x].width=width;
				m_Rois[y][x].height=height;
				left+=width;
				width=w;
				if(_wRem&&!(x%2)){
					bIncW?++width:--width;
					--_wRem;
				}
			}
			top+=height;
			height=h;
			if(hRem&&!(y%2)){
				bIncH?++height:--height;
				--hRem;
			}
		}
	}
#if defined(IMIDEBUG)
	// try to set the rois
	unsigned x,y;
	rImg.pushRoi();
	for(y=0;y<8;++y)
		for(x=0;x<8;++x)
			rImg.setRoi(m_Rois[y][x]);
	rImg.popRoi();
#endif
	m_bInitialized=true;
}

#if defined(_MSC_VER)
template <typename TIMG,typename TDATA> __forceinline array<TDATA>* colorLayoutExtractor<TIMG,TDATA>::extractColorLayout()
#elif defined(__GNUC__)
template <typename TIMG,typename TDATA> array<TDATA>* colorLayoutExtractor<TIMG,TDATA>::extractColorLayout() // GNU does not know about __forceinline
#else 
#error neither GNU nor MSC compiler 
#endif
{
	// if(!m_bInitialized)
	// 	throw fqImageError(imageError::instanceNotInitialized);
	
	// create the data, caller has to delete this...
	array<TDATA>* pData=new array<TDATA>(featureLength());
	if(!pData)
		throw fqImageError(imageError::badAlloc);

	extractColorLayout(pData->pVals());

	return pData;
}

template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::extractColorLayout(TDATA* pData,double chromaFactor)
{
	if(!m_bInitialized)
		throw fqImageError(imageError::instanceNotInitialized);

	if(!pData)
		throw fqImageError(imageError::badParameter);
	
	// this does most of the work:
	extractRawData();

	unsigned j=0,i;
	
	// copy the requested feature length zig zag scanned to the output
	// multiply the chroma values with the given factor
	// note here we have the following channel order: 
	// YDC
	// CbDC
	// CrDC
	// (m_uYCoeffs-1) YAC Coeffs (zig zag scanned)
	// (m_uCbCrCoeffs-1) CbAC Coeffs (zig zag scanned)
	// (m_uCbCrCoeffs-1) CrAC Coeffs (zig zag scanned)

	// note we adress the output of the dct as a consecutive array of 64 values
	assert(sizeof(m_YCoeff)==64*sizeof(TDATA));
	assert(sizeof(m_CbCoeff)==64*sizeof(TDATA));
	assert(sizeof(m_CrCoeff)==64*sizeof(TDATA));
	TDATA* pY=&m_YCoeff[0][0];
	TDATA* pCb=&m_CbCoeff[0][0];
	TDATA* pCr=&m_CrCoeff[0][0];

	// dc components
	(pData)[j++]=*pY;		// a.k.a. *(pY+m_ZigZagIndex[0]);
	(pData)[j++]=*pCb*chromaFactor;	// a.k.a. *(pCb+m_ZigZagIndex[0]);
	(pData)[j++]=*pCr*chromaFactor;	// a.k.a. *(pCr+m_ZigZagIndex[0]);

	// y ac components
	for(i=1;i<m_uYCoeffs;++i){
		(pData)[j++]=*(pY+m_ZigZagIndex[i]);
	}
	// cb ac components
	for(i=1;i<m_uCbCrCoeffs;++i){
		(pData)[j++]=*(pCb+m_ZigZagIndex[i])*chromaFactor;
	}
	// cr ac components
	for(i=1;i<m_uCbCrCoeffs;++i){
		(pData)[j++]=*(pCr+m_ZigZagIndex[i])*chromaFactor;
	}
	return;
}
template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::setNumYCoeffs(unsigned num)
{
	if(!num)
		num=1;
	else if(num>64)
		num=64;
	m_uYCoeffs=num;
}

template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::setNumCbCrCoeffs(unsigned num)
{
	if(!num)
		num=1;
	else if(num>64)
		num=64;
	m_uCbCrCoeffs=num;
}

template <typename TIMG,typename TDATA> mpeg7ColorLayoutDescriptor* colorLayoutExtractor<TIMG,TDATA>::extractMpeg7ColorLayout()
{
	// not yet implemented
	throw fqImageError(imageError::notImplemented);
	return NULL;
}
template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::extractOptimizedColorLayout(TDATA* pData,unsigned numValues,double chromafactor)
{
	if(!m_bInitialized)
		throw fqImageError(imageError::instanceNotInitialized);

	if(!pData||!numValues)
		throw fqImageError(imageError::badParameter);

	// clamp 
	if(numValues>64)
		numValues=64;
	
	// this does most of the work:
	extractRawData();

	// note we adress the output of the dct as a consecutive array of 64 values
	assert(sizeof(m_YCoeff)==64*sizeof(TDATA));
	assert(sizeof(m_CbCoeff)==64*sizeof(TDATA));
	assert(sizeof(m_CrCoeff)==64*sizeof(TDATA));
	TDATA* pY=&m_YCoeff[0][0];
	TDATA* pCb=&m_CbCoeff[0][0];
	TDATA* pCr=&m_CrCoeff[0][0];

	array<TDATA> opt(64);
	// note this looks strange, but there seems to be no
	// algorithm to compute this 'optimal' order (really?)
	//opt[63]=*(pY+m_ZigZagIndex[28+1]);	
	//opt[62]=*(pY+m_ZigZagIndex[29+1]);
	//opt[61]=*(pY+m_ZigZagIndex[30+1]);
	//opt[60]=*(pY+m_ZigZagIndex[23+1]);
	//opt[59]=*(pY+m_ZigZagIndex[27+1]);
	//opt[58]=*(pY+m_ZigZagIndex[25+1]);
	//opt[57]=*(pY+m_ZigZagIndex[21+1]);
	//opt[56]=*(pCb+m_ZigZagIndex[14+1])*chromafactor;			
	//opt[55]=*(pCr+m_ZigZagIndex[14+1])*chromafactor;
	//opt[54]=*(pCr+m_ZigZagIndex[13+1])*chromafactor;
	//opt[53]=*(pY+m_ZigZagIndex[26+1]); 
	//opt[52]=*(pY+m_ZigZagIndex[18+1]);
	//opt[51]=*(pY+m_ZigZagIndex[14+1]);
	//opt[50]=*(pY+m_ZigZagIndex[24+1]);
	//opt[49]=*(pY+m_ZigZagIndex[22+1]);
	//opt[48]=*(pY+m_ZigZagIndex[16+1]);			
	//opt[47]=*(pY+m_ZigZagIndex[15+1]);
	//opt[46]=*(pCb+m_ZigZagIndex[12+1])*chromafactor;
	//opt[45]=*(pY+m_ZigZagIndex[12+1]);
	//opt[44]=*(pY+m_ZigZagIndex[20+1]);
	//opt[43]=*(pCb+m_ZigZagIndex[13+1])*chromafactor;
	//opt[42]=*(pCr+m_ZigZagIndex[12+1])*chromafactor;
	//opt[41]=*(pCr+m_ZigZagIndex[6+1])*chromafactor;
	//opt[40]=*(pY+m_ZigZagIndex[10+1]);
	//opt[39]=*(pY+m_ZigZagIndex[17+1]);
	//opt[38]=*(pCb+m_ZigZagIndex[10+1])*chromafactor;
	//opt[37]=*(pCb+m_ZigZagIndex[5+1])*chromafactor;
	//opt[36]=*(pY+m_ZigZagIndex[5+1]);
	//opt[35]=*(pY+m_ZigZagIndex[19+1]);
	//opt[34]=*(pCr+m_ZigZagIndex[10+1])*chromafactor;
	//opt[33]=*(pY+m_ZigZagIndex[13+1]);
	//opt[32]=*(pCb+m_ZigZagIndex[7+1])*chromafactor;			
	//opt[31]=*(pCr+m_ZigZagIndex[7+1])*chromafactor;	
	//opt[30]=*(pCb+m_ZigZagIndex[6+1])*chromafactor;
	//opt[29]=*(pCb+m_ZigZagIndex[11+1])*chromafactor;
	//opt[28]=*(pY+m_ZigZagIndex[7+1]);
	//opt[27]=*(pY+m_ZigZagIndex[6+1]);
	//opt[26]=*(pY+m_ZigZagIndex[11+1]);
	//opt[25]=*(pCr+m_ZigZagIndex[5+1])*chromafactor;
	//opt[24]=*(pY+m_ZigZagIndex[9+1]);			
	//opt[23]=*(pCb+m_ZigZagIndex[3+1])*chromafactor;
	//opt[22]=*(pCb+m_ZigZagIndex[9+1])*chromafactor;
	//opt[21]=*(pY+m_ZigZagIndex[3+1]);
	//opt[20]=*(pCr+m_ZigZagIndex[11+1])*chromafactor;
	//opt[19]=*(pCr+m_ZigZagIndex[3+1])*chromafactor;
	//opt[18]=*(pCr+m_ZigZagIndex[9+1])*chromafactor;
	//opt[17]=*(pCb+m_ZigZagIndex[4+1])*chromafactor;
	//opt[16]=*(pCb+m_ZigZagIndex[8+1])*chromafactor;
	//opt[15]=*(pY+m_ZigZagIndex[8+1]);
	//opt[14]=*(pCr+m_ZigZagIndex[4+1])*chromafactor;
	//opt[13]=*(pCb+m_ZigZagIndex[0+1])*chromafactor;
	//opt[12]=*(pY+m_ZigZagIndex[4+1]);
	//opt[11]=*(pCr+m_ZigZagIndex[8+1])*chromafactor;
	//opt[10]=*(pCr+m_ZigZagIndex[0+1])*chromafactor;
	//opt[9]=*(pY+m_ZigZagIndex[0+1]);
	//opt[8]=*(pCb+m_ZigZagIndex[2+1])*chromafactor;
	//opt[7]=*(pY+m_ZigZagIndex[2+1]);
	//opt[6]=*(pCr+m_ZigZagIndex[2+1])*chromafactor;
	//opt[5]=*(pY+m_ZigZagIndex[1+1]);
	//opt[4]=*(pCr+m_ZigZagIndex[1+1])*chromafactor;
	//opt[3]=*(pCb+m_ZigZagIndex[1+1])*chromafactor;
	//// DC components
	//opt[2]=*(pY+m_ZigZagIndex[0]);
	//opt[1]=*(pCb+m_ZigZagIndex[0])*chromafactor;
	//opt[0]=*(pCr+m_ZigZagIndex[0])*chromafactor;

	opt[63]=*(pY+ m_ZigZagIndex[28]);

	opt[62]=*(pCr+m_ZigZagIndex[19]);
	opt[61]=*(pCb+m_ZigZagIndex[19]);
	opt[60]=*(pY+ m_ZigZagIndex[19]);
		
	opt[59]=*(pCr+m_ZigZagIndex[12]);
	opt[58]=*(pCb+m_ZigZagIndex[12]);
	opt[57]=*(pY+ m_ZigZagIndex[12]);
		
	opt[56]=*(pCr+m_ZigZagIndex[21]);
	opt[55]=*(pCb+m_ZigZagIndex[21]);
	opt[54]=*(pY+ m_ZigZagIndex[21]);
		
	opt[53]=*(pCr+m_ZigZagIndex[27]);
	opt[52]=*(pCb+m_ZigZagIndex[27]);
	opt[51]=*(pY+ m_ZigZagIndex[27]);
		
	opt[50]=*(pCr+m_ZigZagIndex[16]);
	opt[49]=*(pCb+m_ZigZagIndex[16]);
	opt[48]=*(pY+ m_ZigZagIndex[16]);
	
	opt[47]=*(pCr+m_ZigZagIndex[13]);
	opt[46]=*(pCb+m_ZigZagIndex[13]);
	opt[45]=*(pY+ m_ZigZagIndex[13]);
	
	opt[44]=*(pCr+m_ZigZagIndex[11]);
	opt[43]=*(pCb+m_ZigZagIndex[11]);
	opt[42]=*(pY+ m_ZigZagIndex[11]);
	
	opt[41]=*(pCr+m_ZigZagIndex[8]);
	opt[40]=*(pCb+m_ZigZagIndex[8]);
	opt[39]=*(pY+ m_ZigZagIndex[8]);
	
	opt[38]=*(pCr+m_ZigZagIndex[7]);
	opt[37]=*(pCb+m_ZigZagIndex[7]);
	opt[36]=*(pY+ m_ZigZagIndex[7]);

	opt[35]=*(pCr+m_ZigZagIndex[20]);
	opt[34]=*(pCb+m_ZigZagIndex[20]);
	opt[33]=*(pY+ m_ZigZagIndex[20]);
	
	opt[32]=*(pCr+m_ZigZagIndex[15]);
	opt[31]=*(pCb+m_ZigZagIndex[15]);
	opt[30]=*(pY+ m_ZigZagIndex[15]);
	
	opt[29]=*(pCr+m_ZigZagIndex[6]);
	opt[28]=*(pCb+m_ZigZagIndex[6]);
	opt[27]=*(pY+ m_ZigZagIndex[6]);
	
	opt[26]=*(pCr+m_ZigZagIndex[9]);
	opt[25]=*(pCb+m_ZigZagIndex[9]);
	opt[24]=*(pY+ m_ZigZagIndex[9]);
	
	opt[23]=*(pCr+m_ZigZagIndex[14]);
	opt[22]=*(pCb+m_ZigZagIndex[14]);
	opt[21]=*(pY+ m_ZigZagIndex[14]);
		
	opt[20]=*(pCr+m_ZigZagIndex[10]);
	opt[19]=*(pCb+m_ZigZagIndex[10]);
	opt[18]=*(pY+ m_ZigZagIndex[10]);
	
	opt[17]=*(pCr+m_ZigZagIndex[5]);
	opt[16]=*(pCb+m_ZigZagIndex[5]);
	opt[15]=*(pY+ m_ZigZagIndex[5]);
	
	opt[14]=*(pCr+m_ZigZagIndex[4]);
	opt[13]=*(pCb+m_ZigZagIndex[4]);
	opt[12]=*(pY+ m_ZigZagIndex[4]);
	
	opt[11]=*(pCr+m_ZigZagIndex[3]);
	opt[10]=*(pCb+m_ZigZagIndex[3]);
	opt[9] =*(pY+ m_ZigZagIndex[3]);
	
	opt[8] =*(pCr+m_ZigZagIndex[2]);
	opt[7] =*(pCb+m_ZigZagIndex[2]);
	opt[6] =*(pY+ m_ZigZagIndex[2]);
	
	opt[5] =*(pCr+m_ZigZagIndex[1]);
	opt[4] =*(pCb+m_ZigZagIndex[1]);
	opt[3] =*(pY+ m_ZigZagIndex[1]);
	// DC components
	opt[2] =*(pCr+m_ZigZagIndex[0]);
	opt[1] =*(pCb+m_ZigZagIndex[0]);
	opt[0] =*(pY +m_ZigZagIndex[0]);
	
	// now copy as much as the caller wants
	unsigned i;
	for(i=0;i<numValues;++i)
		pData[i]=opt[i];

	return;
}

// central processing used by the extract...() members above
template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::extractRawData()
{
	if(m_bHasAlpha&&m_AlphaMode==SKIP_ALPHA){
		extractRawDataSkipAlpha();
		return;
	}

	// calulate the 8x8 thumbnail and convert to YCbCr
	unsigned x,y;
	imagev2<TIMG>& rImg=*m_pImg;

	// TODO: when the meaner has LUTs, we need to keep it...
	meanExtractor<TIMG,TDATA> meaner;
	// array<TIMG> meanVals(rImg.primChannels());
	array<TIMG> meanVals(3);

	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();

	const double maxVal = 220;

	rImg.pushRoi();
	for(y=0;y<8;++y){
		for(x=0;x<8;++x){
			// set roi and get mean
			rImg.setRoi(m_Rois[y][x]);

			// no or ignore alpha
			if(!m_bHasAlpha||m_AlphaMode==IGNORE_ALPHA)
				meaner.extract(rImg,meanVals);
			else if(m_AlphaMode==APPLY_ALPHA)
				meaner.extractApplyAlpha(rImg,meanVals);

			// convert to YCbCr
			m_AvgY[y][x]=(TDATA)(0.299*meanVals[redChan]+0.587*meanVals[greenChan]+0.114*meanVals[blueChan]);
			// note add 128 if TDATA is unsigned !!!
			m_AvgCb[y][x]=(TDATA)((-0.168736*meanVals[redChan]-0.331264*meanVals[greenChan]+0.500*meanVals[blueChan]));
			m_AvgCr[y][x]=(TDATA)((0.500*meanVals[redChan]-0.418688*meanVals[greenChan]-0.081312*meanVals[blueChan]));

			m_AvgCb[y][x] *= 5;
			m_AvgCr[y][x] *= 5;

			// Werte begrenzen
			double len = sqrt(m_AvgCb[y][x]*m_AvgCb[y][x] + m_AvgCr[y][x]*m_AvgCr[y][x]);

			if (len > maxVal) {
				m_AvgCb[y][x] *= maxVal/len;
				m_AvgCr[y][x] *= maxVal/len;
			}
		}
	}
	rImg.popRoi();
	
	// dct transforms
	dct8x8<TDATA>::transform(m_AvgY,m_YCoeff);
	dct8x8<TDATA>::transform(m_AvgCb,m_CbCoeff);
	dct8x8<TDATA>::transform(m_AvgCr,m_CrCoeff);

}

template <typename TIMG,typename TDATA> void colorLayoutExtractor<TIMG,TDATA>::extractRawDataSkipAlpha()
{
	// calulate the 8x8 thumbnail and convert to YCbCr
	int x,y;
	imagev2<TIMG>& rImg=*m_pImg;

	// TODO: when the meaner has LUTs, we need to keep it...
	meanExtractor<TIMG,TDATA> meaner;
	//array<TIMG> meanVals(rImg.primChannels());
	array<TIMG> meanVals(3);

	assert(m_bHasAlpha);
	assert(m_AlphaMode==SKIP_ALPHA);

	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();

	// keep track of ROIs which are fully transparent
	bool alphaROIs[8][8];
	unsigned numAlphaROIs=0;
	const TDATA maxVal = 220;

	rImg.pushRoi();
	for(y=0;y<8;++y){
		for(x=0;x<8;++x){
			// set roi and get mean
			rImg.setRoi(m_Rois[y][x]);

			// convert to YCbCr
			if(meaner.extractSkipAlpha(rImg,meanVals)){
				alphaROIs[y][x]=false;

				m_AvgY[y][x]=(TDATA)(0.299*meanVals[redChan]+0.587*meanVals[greenChan]+0.114*meanVals[blueChan]);
				// note add 128 if TDATA is unsigned !!!
				m_AvgCb[y][x]=(TDATA)((-0.168736*meanVals[redChan]-0.331264*meanVals[greenChan]+0.500*meanVals[blueChan]));
				m_AvgCr[y][x]=(TDATA)((0.500*meanVals[redChan]-0.418688*meanVals[greenChan]-0.081312*meanVals[blueChan]));

				m_AvgCb[y][x] *= 5;
				m_AvgCr[y][x] *= 5;

				// Werte begrenzen
				TDATA len = (TDATA)(sqrt(m_AvgCb[y][x]*m_AvgCb[y][x] + m_AvgCr[y][x]*m_AvgCr[y][x]));

				if (len > maxVal) {
					m_AvgCb[y][x] *= maxVal/len;
					m_AvgCr[y][x] *= maxVal/len;
				}
			}
			else{
				alphaROIs[y][x]=true;
				++numAlphaROIs;
			}
		}
	}
	rImg.popRoi();

	// if there where some (not: all) rois with full alpha,
	// set the mean of the neighbour roi's
	int xs,xe,xi,ys,ye,yi;
	unsigned numFound=0;
	if(numAlphaROIs&&numAlphaROIs!=64){
		// forward
		for(y=0;y<8;++y){
			for(x=0;x<8;++x){
				if(alphaROIs[y][x]){
					TDATA Y=0,Cb=0,Cr=0;
					xs=(x==0)?0:x-1;
					xe=(x==7)?7:x+1;
					ys=(y==0)?0:y-1;
					ye=(y==7)?7:y+1;
					unsigned nums=0;
					for(yi=ys;yi<=ye;++yi){
						for(xi=xs;xi<=xe;++xi){
							if(!alphaROIs[yi][xi]){
								Y+=m_AvgY[yi][xi];
								Cb+=m_AvgCb[yi][xi];
								Cr+=m_AvgCr[yi][xi];
								++nums;
							}
						}
					}
					if(nums){
						alphaROIs[y][x]=false;
						m_AvgY[y][x]=Y/(TDATA)nums;
						m_AvgCb[y][x]=Cb/(TDATA)nums;
						m_AvgCr[y][x]=Cr/(TDATA)nums;
						++numFound;
					}
				}
			}
		}
		// backward, if needed
		if(numFound!=numAlphaROIs){
			for(y=7;y>=0;--y){
				for(x=7;x>=0;--x){
					if(alphaROIs[y][x]){
						TDATA Y=0,Cb=0,Cr=0;
						xs=(x==0)?0:x-1;
						xe=(x==7)?7:x+1;
						ys=(y==0)?0:y-1;
						ye=(y==7)?7:y+1;
						unsigned nums=0;
						for(yi=ys;yi<=ye;++yi){
							for(xi=xs;xi<=xe;++xi){
								if(!alphaROIs[yi][xi]){
									Y+=m_AvgY[yi][xi];
									Cb+=m_AvgCb[yi][xi];
									Cr+=m_AvgCr[yi][xi];
									++nums;
								}
							}
						}
						if(nums){
							alphaROIs[y][x]=false;
							m_AvgY[y][x]=Y/(TDATA)nums;
							m_AvgCb[y][x]=Cb/(TDATA)nums;
							m_AvgCr[y][x]=Cr/(TDATA)nums;
							++numFound;
						}
					}
				}
			}
		}
	}
	else if(numAlphaROIs==64){
		// fully transparent
		// handle as 'white'
		//const TDATA whiteVal=(TDATA)(rImg.maxChannelValue());
		//const TDATA whiteY=(TDATA)(0.299*whiteVal+0.587*whiteVal+0.114*whiteVal);
		//// note add 128 if TDATA is unsigned !!!
		//TDATA whiteCb=((TDATA)(-0.168736*whiteVal-0.331264*whiteVal+0.500*whiteVal))*(TDATA)5;
		//TDATA whiteCr=((TDATA)(0.500*whiteVal-0.418688*whiteVal-0.081312*whiteVal))*(TDATA)5;
		//const TDATA len = (TDATA)(sqrt(whiteCb*whiteCb + whiteCr*whiteCr));
		//if(len>maxVal){
		//	whiteCb *= maxVal/len;
		//	whiteCr *= maxVal/len;
		//}
		const TDATA whiteY=maxVal;
		// note add 128 if TDATA is unsigned !!!
		TDATA whiteCb=(TDATA)0;
		TDATA whiteCr=(TDATA)0;
		for(y=0;y<8;++y){
			for(x=0;x<8;++x){
				m_AvgY[y][x]=whiteY;
				m_AvgCb[y][x]=whiteCb;
				m_AvgCr[y][x]=whiteCr;
				}
			}
		}

	assert(numFound==numAlphaROIs);

	// dct transforms
	dct8x8<TDATA>::transform(m_AvgY,m_YCoeff);
	dct8x8<TDATA>::transform(m_AvgCb,m_CbCoeff);
	dct8x8<TDATA>::transform(m_AvgCr,m_CrCoeff);


}

// implermentation of byteColorLayoutExtractor
double byteColorLayoutExtractor::m_Y_red[256];
double byteColorLayoutExtractor::m_Y_green[256];
double byteColorLayoutExtractor::m_Y_blue[256];
double byteColorLayoutExtractor::m_Cb_red[256];
double byteColorLayoutExtractor::m_Cb_green[256];
double byteColorLayoutExtractor::m_Cb_blue[256];
double byteColorLayoutExtractor::m_Cr_red[256];
double byteColorLayoutExtractor::m_Cr_green[256];
double byteColorLayoutExtractor::m_Cr_blue[256];
const bool byteColorLayoutExtractor::m_bTablesInitialized=initializeTables();

bool byteColorLayoutExtractor::initializeTables()
{
	unsigned i;
	for(i=0;i<256;++i){
		m_Y_red[i]=0.299*(double)i;
		m_Y_green[i]=0.587*(double)i;
		m_Y_blue[i]=0.114*(double)i;

		m_Cb_red[i]=-0.168736*(double)i;
		m_Cb_green[i]=-0.331264*(double)i;
		m_Cb_blue[i]=0.5*(double)i;

		m_Cr_red[i]=0.5*(double)i;
		m_Cr_green[i]=-0.418688*(double)i;
		m_Cr_blue[i]=-0.081312*(double)i;
	}

	return true;

}

void byteColorLayoutExtractor::setBgColor(const rgb<unsigned char>& color)
{
	m_BgColor=color;
	m_Meaner.setBgColor(color);
}
// central processing used by the extract...() members above
void byteColorLayoutExtractor::extractRawData()
{
	if(m_bHasAlpha&&m_AlphaMode==SKIP_ALPHA){
		extractRawDataSkipAlpha();
		return;
	}

	// calulate the 8x8 thumbnail and convert to YCbCr
	unsigned x,y;
	imagev2<unsigned char>& rImg=*m_pImg;

	// array<unsigned char> meanVals(rImg.primChannels());
	array<unsigned char> meanVals(3);

	const unsigned redChan=rImg.redChannel();
	const unsigned greenChan=rImg.greenChannel();
	const unsigned blueChan=rImg.blueChannel();

	const double maxVal = 220;

	rImg.pushRoi();
	for(y=0;y<8;++y){
		for(x=0;x<8;++x){
			// set roi and get mean
			rImg.setRoi(m_Rois[y][x]);

			// no or ignore alpha
			if(!m_bHasAlpha||m_AlphaMode==IGNORE_ALPHA)
				m_Meaner.extract(rImg,meanVals);
			else if(m_AlphaMode==APPLY_ALPHA)
				m_Meaner.extractApplyAlpha(rImg,meanVals);

			// convert to YCbCr
			m_AvgY[y][x]=(double)(0.299*meanVals[redChan]+0.587*meanVals[greenChan]+0.114*meanVals[blueChan]);
			// note add 128 if double is unsigned !!!
			m_AvgCb[y][x]=(double)((-0.168736*meanVals[redChan]-0.331264*meanVals[greenChan]+0.500*meanVals[blueChan]));
			m_AvgCr[y][x]=(double)((0.500*meanVals[redChan]-0.418688*meanVals[greenChan]-0.081312*meanVals[blueChan]));

			// use LUTs...
			//m_AvgY[y][x]=m_Y_red[meanVals[redChan]]+m_Y_green[meanVals[greenChan]]+m_Y_blue[meanVals[blueChan]];
			//m_AvgCb[y][x]=m_Cb_red[meanVals[redChan]]+m_Cb_green[meanVals[greenChan]]+m_Cb_blue[meanVals[blueChan]];
			//m_AvgCr[y][x]=m_Cr_red[meanVals[redChan]]+m_Cr_green[meanVals[greenChan]]+m_Cr_blue[meanVals[blueChan]];

			m_AvgCb[y][x] *= 5;
			m_AvgCr[y][x] *= 5;

			// Werte begrenzen
			double len = sqrt(m_AvgCb[y][x]*m_AvgCb[y][x] + m_AvgCr[y][x]*m_AvgCr[y][x]);

			if (len > maxVal) {
				m_AvgCb[y][x] *= maxVal/len;
				m_AvgCr[y][x] *= maxVal/len;
			}
		}
	}
	rImg.popRoi();

	// dct transforms
	dct8x8<double>::transform(m_AvgY,m_YCoeff);
	dct8x8<double>::transform(m_AvgCb,m_CbCoeff);
	dct8x8<double>::transform(m_AvgCr,m_CrCoeff);

}



// excplicit instantiation
template class colorLayoutExtractor<unsigned char,double>;

