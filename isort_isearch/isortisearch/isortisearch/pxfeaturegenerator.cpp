//#include "pximpldefs.h"
#include "pximageimpl.h"
#include "pxfeaturegenerator.h"
#include "dct8x8.h"
#include <assert.h>
//#include <math.h>

#define _USE_MATH_DEFINES
#include <cmath>


namespace isortisearch{
template <typename TBIG> const int featureGenerator<TBIG>::stepsL = 4;
template <typename TBIG> const int featureGenerator<TBIG>::stepsS = 2;  
template <typename TBIG> const int featureGenerator<TBIG>::stepsH = 8;


template <typename TBIG> const int featureGenerator<TBIG>::deltaL = 100;
template <typename TBIG> const int featureGenerator<TBIG>::deltaS = 120;
template <typename TBIG> const int featureGenerator<TBIG>::deltaH = 360/stepsH; 

template <typename TBIG> const int featureGenerator<TBIG>::rangeL = deltaL*stepsL;
template <typename TBIG> const int featureGenerator<TBIG>::rangeS = deltaS*stepsS;
template <typename TBIG> const int featureGenerator<TBIG>::rangeH = deltaH*stepsH;

template <typename TBIG> const int featureGenerator<TBIG>::rangeL2 = rangeL/2;

template <typename TBIG> const TBIG featureGenerator<TBIG>::T0 = (TBIG)250.;
template <typename TBIG> const TBIG featureGenerator<TBIG>::T1 = (TBIG)0.68; 
template <typename TBIG> const TBIG featureGenerator<TBIG>::T2 = (TBIG)0.98;

template <typename TBIG> bool featureGenerator<TBIG>::calculateFeatureData(const imageDescriptor& iDesc)
{
	assert(colorLayoutSize()+histoSize()+edgeSize()+textureSize()==FEATURESIZE);

	if(!iDesc.m_pImage||!iDesc.m_pFirstPixel||iDesc.m_Width<8||iDesc.m_Height<8)
		return false;

	// the longer side of the image has to be at least 40 pixels (for calculateColorTexture())
	if(__max(iDesc.m_Width,iDesc.m_Height)<40)
		return false;

	// invalidate feature data
	// note we have to be the images friend to do this
	iDesc.m_pImage->m_uFlags&=image::m_ValidFeatureDataMask;

	TBIG featureData[FEATURESIZE];
	// Note some of the functions below just add to the featuredata, thus set to 0 here
	for(unsigned i=0;i<FEATURESIZE;i++)
		featureData[i]=(TBIG)0.;

	// NOTE check if the following four functions can be openMP parallel sections

	// compute the color layout data
	if(!calculateColorLayout(featureData,iDesc))
		return false;
	// compute the color histogram
	if(!calculateColorHistogram(featureData+histoOffset(),iDesc))
		return false;
	// compute edge data
	if(!calculateEdgeFeature(featureData+edgeOffset(),iDesc))
		return false;
	// compute texture data
	if(!calculateColorTexture(featureData+textureOffset(),iDesc))
		return false;

	TBIG sum = (TBIG)1.;  // Ersatz fuer Laenge des Farbhistogrammvektors (und Vermeidung der Division durch 0)
	// den Featurevektor (bis auf die Farbhistogrammkomponente) normieren
	for (unsigned i = 0; i < FEATURESIZE; ++i) {
		// kai's java code: if (!(i>=colorLayoutSize && i < colorLayoutSize+colorHistoSize-2))
		if (i<=histoOffset()||i>=edgeOffset()-2)
			sum += abs(featureData[i]);
	}
	
	// Werte so verschieben und skalieren, dass sie den Bereich von -0.5 bis 0.5 "gut" ausnutzen
	sum = (TBIG)3.8/sum;
	for (unsigned i = 0; i < FEATURESIZE; ++i) {
		if (i<colorLayoutSize())
			featureData[i] = featureData[i] * sum;
		else if (i < colorLayoutSize()+histoSize()-2)
			featureData[i] = featureData[i] - (TBIG)0.5;
		else
			featureData[i] = featureData[i] * sum - (TBIG)0.5; 		
	}
	
	// write feature data through iDesc to image
	char* pCharData=iDesc.m_pImage->m_pImpl->m_FeatureData;

	// Quantisierung der Werte auf Byte (-127 bis 127)
	const TBIG g = (TBIG)0.5;
	for (unsigned i = 0; i < FEATURESIZE; ++i) {
		if (featureData[i] > g)
			featureData[i] = g;
		else if (featureData[i] < -g)
			featureData[i] = -g;	

		pCharData[i] = (char)(featureData[i]*2*127);
	}
	// validate feature data
	iDesc.m_pImage->m_uFlags|=image::m_ValidFeatureDataFlag;

	return true;
}

template <typename TBIG> bool featureGenerator<TBIG>::calculateColorLayout(TBIG *pFeatureData,const imageDescriptor& iDesc)
{
	assert(pFeatureData);

	//try{
		// NOTE: formerly, we used these classes to compute the color layout.
		// these used a better algorithm to compute the 8*8 mean values from the image by imagev5 ROIs
		// and were able to apply/skip/ignore alpha channel data
		// however, we now use Kai's new code...

		//byteImageV5 byteImg;
		//byteImageV5::colorSpace cs;

		//switch(iDesc.m_ChannelLayout){
		//	case ARGB:
		//		cs=byteImageV5::colorSpaceARGB;
		//		break;
		//	case RGBA:
		//		cs=byteImageV5::colorSpaceRGBA;
		//		break;
		//	case BGRA:
		//		cs=byteImageV5::colorSpaceBGRA;
		//		break;
		//	case ABGR:
		//		cs=byteImageV5::colorSpaceABGR;
		//		break;
		//	case UNDEFINED:
		//	default:
		//		return false;
		//		break;
		//}

		//byteImg.create(iDesc.m_Width,iDesc.m_Height,0,true,(unsigned __int32*)(iDesc.m_pFirstPixel));
		//byteImg.setColorSpace(cs);

		// ...to extract the color layout 
		// the color layout extracor
		// note byteColorLayoutExtractorV5<float> is not reentrant currently
		// thus create it here...
		//byteColorLayoutExtractorV5<float> ex;
		//ex.setAlphaMode((byteColorLayoutExtractorV5<float>::alpha_mode)iDesc.m_AlphaMode);
		//ex.setBgColor(rgb<unsigned char>(iDesc.m_Red,iDesc.m_Green,iDesc.m_Blue));
		//ex.init(byteImg);
		//ex.extractFeature(iDesc.m_pImage->m_pImpl->m_FeatureData);
		//byteImg.destroy();
		
		// Kai's new code:

		// compute an 8*8 YCbCr from the image
		const unsigned width = iDesc.m_Width;
		const unsigned height = iDesc.m_Height;
		
		const unsigned numBlocks = 8;
		const TBIG maxVal = (TBIG)160;
		const TBIG maxVal2 = maxVal*maxVal;
		
		// note these have to be numBlocks*numBlocks
		TBIG y8x8[numBlocks][numBlocks];
		TBIG cb8x8[numBlocks][numBlocks];
		TBIG cr8x8[numBlocks][numBlocks];

		const unsigned __int32* pFirstPixel=iDesc.m_pFirstPixel;
		unsigned __int32 pix;
		unsigned rShift,bShift,gShift;

		switch(iDesc.m_ChannelLayout){
				case RGBA:
					rShift=24;
					gShift=16;
					bShift=8;
					break;
				case BGRA:		
					rShift=8;
					gShift=16;
					bShift=24;
					break;
				case ARGB:
					rShift=16;
					gShift=8;
					bShift=0;
					break;
				case ABGR:
					rShift=0;
					gShift=8;
					bShift=16;
					break;
				case UNDEFINED:
				default:
					return false;
		}

		for(unsigned y_ = 0; y_ < numBlocks; ++y_) {
			const unsigned yStart = y_*height/numBlocks;
			const unsigned yEnd = (y_+1)*height/numBlocks;

			for (int x_ = 0; x_ < numBlocks; ++x_) {
				const unsigned xStart = x_*width/numBlocks;
				const unsigned xEnd = (x_+1)*width/numBlocks;

				// loop over the blocks
				TBIG r = (TBIG)0., g = (TBIG)0., b = (TBIG)0.;
				unsigned sum = 0;

				for (unsigned y = yStart; y < yEnd; ++y) {
					for (unsigned x = xStart; x < xEnd; ++x, ++sum) {

						pix=*(pFirstPixel+y*width+x);
						r += (TBIG)((pix >> rShift) & 255);
						g += (TBIG)((pix >> gShift) & 255);
						b += (TBIG)((pix >> bShift) & 255);
					}
				}
				assert(sum);
				// compute the mean color
				const TBIG sum_ = (TBIG)1./(TBIG)sum;
				r *= sum_;
				g *= sum_;
				b *= sum_;
				
				// convert to YCbCr (really?)
				y8x8[y_][x_] = (TBIG)0.25*(r+ 2*g + b) - (TBIG)128.;
				TBIG cb = (TBIG)1.25*(-r + 2*g -b); 
				TBIG cr = (TBIG)1.25*(r - b); 

				// clamp saturation
				TBIG len = cb*cb + cr*cr;
				if (len > maxVal2) {
					len = sqrt(len);
					cb *= maxVal/len;
					cr *= maxVal/len;
				}
					
				cb8x8[y_][x_] = cb;
				cr8x8[y_][x_] = cr;
			}
		}
		//byteImg.destroy();

		// dct's
		TBIG y_DCT[8][8];
		TBIG cbDCT[8][8];
		TBIG crDCT[8][8];
		dct8x8<TBIG>::transform(y8x8,y_DCT);
		dct8x8<TBIG>::transform(cb8x8,cbDCT);
		dct8x8<TBIG>::transform(cr8x8,crDCT);

		// generate feature
		const TBIG f = (TBIG)0.0008;
		pFeatureData[0]  = f*y_DCT[0][0]; 
		pFeatureData[1]  = f*crDCT[0][0];
		pFeatureData[2]  = f*cbDCT[0][0]; 
		pFeatureData[3]  = f*y_DCT[0][1];
		pFeatureData[4]  = f*crDCT[0][1];
		pFeatureData[5]  = f*cbDCT[0][1];
		pFeatureData[6]  = f*y_DCT[1][0];
		pFeatureData[7]  = f*crDCT[1][0];
		pFeatureData[8]  = f*cbDCT[1][0];
		pFeatureData[9]  = f*y_DCT[2][0];
		pFeatureData[10] = f*crDCT[2][0];
		pFeatureData[11] = f*cbDCT[2][0];				
		pFeatureData[12] = f*y_DCT[0][2];			
		pFeatureData[13] = f*crDCT[0][2];
		pFeatureData[14] = f*cbDCT[0][2];
	//}
	//catch(...){
	//	return false;
	//}
	return true;
}

template <typename TBIG> bool featureGenerator<TBIG>::calculateColorHistogram(TBIG *pFeatureData,const imageDescriptor& iDesc)
{
	assert(pFeatureData);

	const int width  = iDesc.m_Width;
	const int height = iDesc.m_Height;

	// cube to accumulate the color histogram
	array3DV5<TBIG> cube;
	try{
		cube.resize(stepsL,stepsS,stepsH);
	}
	catch(std::bad_alloc){
#ifndef NO_THROW_BADALLOC
		throw;
#endif
		return false;
	}
	cube.set((TBIG)0.);

	// inner part of the image
	const int w2 = width/2;
	const int w4 = width/4;
	const int h2 = height/2;
	const int h4 = height/4; 
																   	
	// mean of the color components
	TBIG ma = (TBIG)0.;
	TBIG mb = (TBIG)0.;
	// variances
	TBIG va = (TBIG)0.;
	TBIG vb = (TBIG)0.;

	// sum of weights (inner parts of the image are weighted higher)
	TBIG sumW = (TBIG)0.;			 		

	int r,g,b;
	//int rgb[3];
	TBIG lab[3];
	TBIG bsh[3];

	const unsigned __int32* pFirstPixel=iDesc.m_pFirstPixel;
	unsigned __int32 pix;
	unsigned rShift,bShift,gShift;

	switch(iDesc.m_ChannelLayout){
			case RGBA:
				rShift=24;
				gShift=16;
				bShift=8;
				break;
			case BGRA:		
				rShift=8;
				gShift=16;
				bShift=24;
				break;
			case ARGB:
				rShift=16;
				gShift=8;
				bShift=0;
				break;
			case ABGR:
				rShift=0;
				gShift=8;
				bShift=16;
				break;
			case UNDEFINED:
			default:
				return false;
	}

	for(int y=0; y < height; ++y) {
		bool isYcenter = abs(y-h2) < h4;
		
		for (int x=0 ; x<width ; ++x) {

			// weight for either inner or outer image part 
			TBIG w = (TBIG) ((abs(x-w2) < w4 && isYcenter) ? (TBIG)3. : (TBIG)1.);
			sumW += w;


			pix = *(pFirstPixel+y*width+x); 
			r = (int)((pix>>rShift)&255);
			g = (int)((pix>>gShift)&255);
			b = (int)((pix>>bShift)&255);
			
			// convert rgb to "lab"
			// rgb2lab(rgb,lab);

			lab[0] = (TBIG)0.25*(TBIG)(r + 2*g + b) - (TBIG)128.;
			lab[1] = (TBIG)(-r + 2*g -b);  
			lab[2] = (TBIG)(r - b); 


			// sum up mean and variance
			const TBIG ta = lab[1];
			const TBIG tb = lab[2];
			ma += w*ta;
			mb += w*tb;
			va += w*ta*ta;
			vb += w*tb*tb;

			// convert "lab" to bsh
			lab2bsh(lab,bsh);		

			fuzzyQuantization(bsh, w, cube); 
		}
	}
	sumW = (TBIG)1./sumW;

	// write nomalized and potentized histogramm to feature data
	// ... fillAndNormFeatureVector(cube,sumW,pFeatureData+histoOffset());
	// contains just:
	packCube(cube); // some histogram cells are merged

	normalizeHistogram(cube, sumW); 

	fillFeatureVector(pFeatureData, cube); 

	// compute variances
	ma *= sumW;
	mb *= sumW;
	va *= sumW;
	vb *= sumW;
	va -= ma*ma;
	vb -= mb*mb;

	// compute standard deviation
	// note the variances may be smaller than 0. due to unaccuracy of TBIG
	if (va < (TBIG)0.) 
		va = (TBIG)0.; 
	TBIG sa = (TBIG) sqrt(va);
	if (vb < (TBIG)0.) 
		vb = (TBIG)0.; 
	TBIG sb = (TBIG) sqrt(vb);

	const TBIG ks = (TBIG)0.0158;  
	*(pFeatureData+histoSize()-2) = ks*sa;
	*(pFeatureData+histoSize()-1) = ks*sb;

	return true;
}

template <typename TBIG> void featureGenerator<TBIG>::rgb2lab(byteRGB rgb,TBIG* lab)
{
	const int r = rgb.m_red;
	const int g = rgb.m_green;
	const int b = rgb.m_blue;

	lab[0] = (TBIG)0.25*(TBIG)(r + 2*g + b) - (TBIG)128.;
	lab[1] = (TBIG)(-r + 2*g -b);  
	lab[2] = (TBIG)(r - b); 
}

template <typename TBIG> void featureGenerator<TBIG>::lab2bsh(TBIG* lab,TBIG* bsh)
{
	//const TBIG l = lab[0];
	const TBIG a = lab[1];
	const TBIG b = lab[2];

	TBIG s = (TBIG) sqrt(a*a+b*b);
	TBIG h = (TBIG)180.*(TBIG)M_1_PI*(atan2(b,a)) + (TBIG)22.5; 

	if (h < (TBIG)0.)
		h += (TBIG)360.;
	else if (h >= (TBIG)360.)
		h -= (TBIG)360.;

	bsh[0] = lab[0];
	bsh[1] = s; 
	bsh[2] = h; 
}

/*
 * fuzzy quantization with triangle'zugehoerigkeitsfunktionen'
 */
template <typename TBIG> void featureGenerator<TBIG>::fuzzyQuantization(TBIG* lsh,TBIG w,array3DV5<TBIG>& cube)
{
	TBIG l = lsh[0];
	TBIG s = lsh[1];
	TBIG h = lsh[2];

	// shift l, because 128 was subtracted before (center l-range)
	l += rangeL2; 
	
	// clamp values to max or min values
	if (s>=rangeS)
		s = rangeS-(TBIG)1.;
	else if (s<(TBIG)0.)
		s = (TBIG)0.;

	if (h>=rangeH)
		h = rangeH-(TBIG)1.;
	else if (h<(TBIG)0.)
		h = (TBIG)0.;

	if (l>=rangeL)
		l = rangeL-(TBIG)1.;
	else if (l<(TBIG)0.)
		l = (TBIG)0.;
	
	// calculate individual histogram weigths

	// fuzzy quantization of l (luminance) 
	TBIG wl_m1, wl, wl_p1;
	// index as TBIG
	const TBIG ql = l/deltaL;
	// index as int
	const int il = (int)ql;
	// fraction in the interval
	const TBIG dl = ql - il; 

	if (dl >= (TBIG)0.5) { 	// larger than the center
		wl_m1 = (TBIG)0.;
		wl = (TBIG)1.5 - dl;
		wl_p1 = (TBIG)1 - wl;
	}
	else {
		wl = (TBIG)0.5 + dl;
		wl_m1 = (TBIG)1. - wl;
		wl_p1 = (TBIG)0.;
	}

	// quantization of s (saturation)(note s >= 0) 
	TBIG ws, ws_p1;
	int is, is_p1;
	
	// dead zone (ignore small values)
	s = std::max((TBIG)0., s-(TBIG)10.);	
	if (s > deltaS) {
		is = 0;
		ws    = (TBIG)0.;
		is_p1 = 1;
		ws_p1 = (TBIG)1.;	
	}
	else {
		is_p1 = 1;
		ws_p1 = s/deltaS;
		is    = 0;
		ws    = (TBIG)1. - ws_p1;
	}

	// quantization of h (hue)
	TBIG wh_m1, wh, wh_p1;
	// index as TBIG
	const TBIG qh = h/deltaH;
	// index as int	
	const int ih = (int)qh;
	// fraction in the interval
	const TBIG dh = qh - (TBIG)ih; 

	if (dh >= (TBIG)0.5) { 	// larger than the center
		wh_m1 = (TBIG)0.;
		wh = (TBIG)1.5 - dh;
		wh_p1 = (TBIG)1. - wh;
	}
	else {
		wh = (TBIG)0.5 + dh;
		wh_m1 = (TBIG)1. - wh;
		wh_p1 = (TBIG)0.;
	}
	// neighbour cells with wraping (!)
	const int ib_m1 = (ih > 0) ? ih-1 : stepsH-1;
	const int ib_p1 = (ih < stepsH-1) ? ih+1 : 0; 

	if (wl_m1 > (TBIG)0.) {
		wl_m1 *= w;	// scale with pixel weight
		const int il_m1 = (il > 0) ? il-1 : 0; // smaller neighbour cell
		cube.at(il_m1,is   ,ib_m1) += wl_m1 * ws    * wh_m1;
		cube.at(il_m1,is   ,ih   ) += wl_m1 * ws    * wh;
		cube.at(il_m1,is   ,ib_p1) += wl_m1 * ws    * wh_p1;
		cube.at(il_m1,is_p1,ib_m1) += wl_m1 * ws_p1 * wh_m1;
		cube.at(il_m1,is_p1,ih   ) += wl_m1 * ws_p1 * wh;
		cube.at(il_m1,is_p1,ib_p1) += wl_m1 * ws_p1 * wh_p1;
	}
	else {
		wl_p1 *= w; // scale with pixel weight
		int il_p1 = (il < stepsL-1) ? il+1 : stepsL-1; // bigger neighbour cell
		cube.at(il_p1,is   ,ib_m1) += wl_p1 * ws    * wh_m1;
		cube.at(il_p1,is   ,ih   ) += wl_p1 * ws    * wh;
		cube.at(il_p1,is   ,ib_p1) += wl_p1 * ws    * wh_p1;
		cube.at(il_p1,is_p1,ib_m1) += wl_p1 * ws_p1 * wh_m1;
		cube.at(il_p1,is_p1,ih   ) += wl_p1 * ws_p1 * wh;
		cube.at(il_p1,is_p1,ib_p1) += wl_p1 * ws_p1 * wh_p1;
	}
	wl *= w; // scale with pixel weight
	cube.at(il   ,is   ,ib_m1) += wl    * ws    * wh_m1;
	cube.at(il   ,is   ,ih   ) += wl    * ws    * wh;
	cube.at(il   ,is   ,ib_p1) += wl    * ws    * wh_p1;
	cube.at(il   ,is_p1,ib_m1) += wl    * ws_p1 * wh_m1;
	cube.at(il   ,is_p1,ih   ) += wl    * ws_p1 * wh;
	cube.at(il   ,is_p1,ib_p1) += wl    * ws_p1 * wh_p1;	
}

template <typename TBIG> void featureGenerator<TBIG>::packCube(array3DV5<TBIG>& cube)
{
	for (int li = 0; li < stepsL; ++li) {
		for (int hi = 1; hi < stepsH; ++hi) {
			cube.at(li,0,0) += cube.at(li,0,hi); // merge inner grey values
			cube.at(li,0,hi) = (TBIG)0.; // not really needed
		}

		cube.at(li,1,5) += cube.at(li,1,6); // merge blue values
		cube.at(li,1,6) = (TBIG)0.; // see above

		// dark colors and black
		if (li == 0) {
			for (int hi = 0; hi < stepsH; ++hi) {
				cube.at(1,1,hi) += cube.at(0,1,hi); // add next brighter color
				cube.at(0,1,hi) = (TBIG)0.; // see above
			} 
		}
		
		// bright colors and white
		if (li == stepsL-1) {
			for (int hi = 0; hi < stepsH; ++hi) {
				cube.at(stepsL-2,1,hi) += cube.at(stepsL-1,1,hi); // add next darker color
				cube.at(stepsL-1,1,hi) = (TBIG)0.; // see above
			} 
		}
	}
}

/*
 * noarmalize and potentalize histogram (gain smaller values)
 */
template <typename TBIG> void featureGenerator<TBIG>::normalizeHistogram(array3DV5<TBIG>& cube,TBIG w)
{	
	for (int i = 0; i < stepsL; ++i) {
		for (int j = 0; j < stepsS; ++j) {
			for (int k = 0; k < stepsH; ++k) {
				cube.at(i,j,k) = (TBIG)0.65*pow(cube.at(i,j,k)*w,(TBIG)0.34);	
			}
		}
	}
}

template <typename TBIG> void featureGenerator<TBIG>::fillFeatureVector(TBIG *pFeatureData,array3DV5<TBIG>& cube)
{
	unsigned m = 0;

	// copy just colors, grey values an b/w into the feature data
	for (int li = 0; li < stepsL; ++li) {
		if (li == 0) {
			pFeatureData[m++] = cube.at(0,0,0); 
		}
		else if (li != stepsL-1){
			for (int ai = 0; ai < stepsS; ++ai) {
				for (int bi = 0; bi < stepsH; ++bi) {
					if (ai == 0 && bi == 0) {
						pFeatureData[m++] = cube.at(li,ai,bi); 
					}
					else if (ai == 1 && bi != 6) {
						pFeatureData[m++] = cube.at(li,ai,bi); 
					}
				}
			}
		}
		else {
			pFeatureData[m++] = cube.at(li,0,0); 
		}

	}
}

template <typename TBIG> bool featureGenerator<TBIG>::calculateEdgeFeature(TBIG *pFeatureData,const imageDescriptor& iDesc)
{
	const int width  =iDesc.m_Width;
	const int height =iDesc.m_Height;

	// will div by zero below...
	assert(width!=2);
	assert(height!=2);

	// Kai's java code:
	// int borderx = (int) (width/10. + 1); 
	// int bordery = (int) (height/10. + 1); 
	const int borderx = (int)((TBIG)width/(TBIG)10. + (TBIG)1); 
	const int bordery = (int)((TBIG)height/(TBIG)10. +(TBIG) 1); 

	const unsigned __int32* pFirstPixel=iDesc.m_pFirstPixel;
	unsigned rShift,bShift,gShift;

	switch(iDesc.m_ChannelLayout){
			case RGBA:
				rShift=24;
				gShift=16;
				bShift=8;
				break;
			case BGRA:		
				rShift=8;
				gShift=16;
				bShift=24;
				break;
			case ARGB:
				rShift=16;
				gShift=8;
				bShift=0;
				break;
			case ABGR:
				rShift=0;
				gShift=8;
				bShift=16;
				break;
			case UNDEFINED:
			default:
				return false;
	}
	TBIG* lum;
	// we don't catch a std::bad_alloc here
	//try{
		 lum = new TBIG[width*height];
	//}
	//catch(std::bad_alloc){
	//	throw;
	//	return false;
	//}

	unsigned __int32 pix;
	int pos;
	unsigned r,g,b;
	for(int y=bordery; y < height-bordery; ++y) {
		for (int x=borderx ; x<width-borderx ; ++x) {
			pos=y*width+x;
			pix = *(pFirstPixel+pos); 
			r = (pix>>rShift)&255;
			g = (pix>>gShift)&255;
			b = (pix>>bShift)&255;

			lum[pos] = (TBIG)0.25*(TBIG)r + (TBIG)0.7*(TBIG)g + (TBIG)0.05*(TBIG)b;
		}	
	}

	TBIG mask1,mask2,mask3,mask4,mask5,max;
	for(int y=bordery; y < height-bordery; y+=2) {
		for (int x=borderx ; x<width-borderx ; x+=2) {

			int pos = y*width+x;
			const TBIG y1 = lum[pos];
			const TBIG y2 = lum[pos+1]; 
			const TBIG y3 = lum[pos+width]; 
			const TBIG y4 = lum[pos+width+1]; 

			mask1 = 2*(    y1 - y2 - y3 + y4);
			mask1 *= mask1;
			mask2 = (      y1 + y2 - y3 - y4);
			mask2 *= mask2;
			mask3 = (      y1 - y2 + y3 - y4);
			mask3 *= mask3;
			mask4 = (TBIG)1.414*(y1          - y4);
			mask4 *= mask4;
			mask5 = (TBIG)1.414*(    y2 - y3);
			mask5 *= mask5;
			
			max = mask1;
			if (mask2 > max) max = mask2;
			if (mask3 > max) max = mask3;
			if (mask4 > max) max = mask4;
			if (mask5 > max) max = mask5;

			if (max > 40) {
				if (max < T0) 
					pFeatureData[0]++; 
				else {
					TBIG max_ = (TBIG)1. / max;
					
					if (mask1 * max_ > T1) pFeatureData[1]++;
					if (mask2 * max_ > T2) pFeatureData[2]++;
					if (mask3 * max_ > T2) pFeatureData[3]++;
					if (mask4 * max_ > T2) pFeatureData[4]++;
					if (mask5 * max_ > T2) pFeatureData[5]++;
				}
			}		
		}	
	}

	delete[] lum;

	
	const TBIG _sum = (TBIG)12.73/(TBIG)((height-2*bordery) * (width-2*borderx));

	
	for(unsigned k = 0; k < edgeSize(); ++k) {
		pFeatureData[k] *= _sum;
	}

	return true;
}

template <typename TBIG> bool featureGenerator<TBIG>::calculateColorTexture(TBIG *pFeatureData,const imageDescriptor& iDesc)
{
	const unsigned __int32* pFirstPixel=iDesc.m_pFirstPixel;
	unsigned __int32 pix;
	unsigned rShift,bShift,gShift;

	switch(iDesc.m_ChannelLayout){
			case RGBA:
				rShift=24;
				gShift=16;
				bShift=8;
				break;
			case BGRA:		
				rShift=8;
				gShift=16;
				bShift=24;
				break;
			case ARGB:
				rShift=16;
				gShift=8;
				bShift=0;
				break;
			case ABGR:
				rShift=0;
				gShift=8;
				bShift=16;
				break;
			case UNDEFINED:
			default:
				return false;
	}

	const unsigned width  =iDesc.m_Width;
	const unsigned height =iDesc.m_Height;
	const TBIG maxSize=(TBIG)40.;
	const unsigned max = __max(width,height);
	//if(max<40)
	//	return false;

	// normalize maximum dimension to maxSize
	unsigned newWidth = (int) ((TBIG) width * maxSize / (TBIG)max + (TBIG)0.5);
	if(newWidth<8)
		newWidth=8;

	unsigned newHeight =  (int) ((TBIG) height * maxSize / (TBIG)max + (TBIG)0.5);
	if(newHeight<8)
		newHeight=8;
	
	// div by zero below...
	// NOTE we should find another way to handle this. here, an image of 160*8 pixels will return false
	//if(newHeight<=4)
	//	return false;
	//if(newWidth<=4)
	//	return false;
	
	array3DV5<TBIG> lab;
	try{
		lab.resize(3,newHeight,newWidth);
	}
	catch(std::bad_alloc){
#ifndef NO_THROW_BADALLOC
		throw;
#endif
		return false;
	}

	// shrink and color transforation
	TBIG r,g,b;
	unsigned sum;
	for (unsigned y_ = 0; y_ < newHeight; ++y_) {
		const unsigned yStart = y_*height/newHeight;
		const unsigned yEnd = (y_+1)*height/newHeight;

		for (unsigned x_ = 0; x_ < newWidth; ++x_) {
			const unsigned xStart = x_*width/newWidth;
			const unsigned xEnd = (x_+1)*width/newWidth;

			// loop over the blocks
			r = (TBIG)0.;
			g = (TBIG)0.;
			b = (TBIG)0.;
			sum = 0;

			for (unsigned y = yStart; y < yEnd; ++y) {
				for (unsigned x = xStart; x < xEnd; ++x, ++sum) {

					pix = *(pFirstPixel+y*width + x);
					r += (pix >> rShift) & 255;
					g += (pix >> gShift) & 255;
					b += (pix >> bShift) & 255;
				}
			}
			
			// compute the mean color
			const TBIG sum_ = (TBIG)1./(TBIG)sum;
			r *= sum_;
			g *= sum_;
			b *= sum_;
			
			// convert to lab
			lab.at(0,y_,x_) = (TBIG)0.25*(r+ (TBIG)2.*g + b) - (TBIG)128.;
			lab.at(1,y_,x_) = (TBIG)0.25*(-r + (TBIG)2.*g -b); // 1.25
			lab.at(2,y_,x_) = (TBIG)0.25*(r - b);       // 1.25
		}
	}

	//getFeatureDataFromFilters(lab,newWidth,newHeight,pFeatureData);
	const unsigned border = 2;
	const TBIG w = (TBIG)0.5 * sqrt((TBIG)2.);
		
	for (unsigned y = border; y < newHeight-border; ++y) {
		
		TBIG l0;
		TBIG l1 = lab.at(0,y-1,0);
		TBIG l2 = lab.at(0,y-1,1);
		TBIG l3;
		TBIG l4 = lab.at(0,y,0);
		TBIG l5 = lab.at(0,y,1);
		TBIG l6;
		TBIG l7 = lab.at(0,y+1,0);
		TBIG l8 = lab.at(0,y+1,1);
		
		TBIG a0;
		TBIG a1 = lab.at(1,y-1,0);
		TBIG a2 = lab.at(1,y-1,1);
		TBIG a3;
		TBIG a4 = lab.at(1,y  ,0);
		TBIG a5 = lab.at(1,y  ,1);
		TBIG a6;
		TBIG a7 = lab.at(1,y+1,0);
		TBIG a8 = lab.at(1,y+1,1);
		
		TBIG b0;
		TBIG b1 = lab.at(2,y-1,0);
		TBIG b2 = lab.at(2,y-1,1);
		TBIG b3;
		TBIG b4 = lab.at(2,y  ,0);
		TBIG b5 = lab.at(2,y  ,1);
		TBIG b6;
		TBIG b7 = lab.at(2,y+1,0);
		TBIG b8 = lab.at(2,y+1,1);

		assert(newWidth>border);
		for (unsigned x = border; x < newWidth-border; ++x) {

			l0 = l1;
			l1 = l2;
			l2 = lab.at(0,y-1,x+1);
			l3 = l4;
			l4 = l5;
			l5 = lab.at(0,y  ,x+1);
			l6 = l7;
			l7 = l8;
			l8 = lab.at(0,y+1,x+1);
			
			a0 = a1;
			a1 = a2;
			a2 = lab.at(1,y-1,x+1);
			a3 = a4;
			a4 = a5;
			a5 = lab.at(1,y  ,x+1);
			a6 = a7;
			a7 = a8;
			a8 = lab.at(1,y+1,x+1);
			
			b0 = b1;
			b1 = b2;
			b2 = lab.at(2,y-1,x+1);
			b3 = b4;
			b4 = b5;
			b5 = lab.at(2,y  ,x+1);
			b6 = b7;
			b7 = b8;
			b8 = lab.at(2,y+1,x+1);

			
			// filter
			//			 	{ -w,  0,  w } 
			//				{ -1,  0,  1 }  
			//				{ -w,  0,  w }
			TBIG lf0  = w*(l2 - l0 - l6 + l8) - l3 + l5;

			//				{ -w, -1, -w }  
			//				{  0,  0,  0 }  
			//				{  w,  1,  w } 
			TBIG lf1  = w*(l6 + l8 - l0 - l2) - l1 + l7; 

			//				{  0, -1,  0 }  
			//				{  1,  0,  1 }  
			//				{  0, -1,  0 }
			TBIG lf2  = l3 - l1 + l5 - l7;

			TBIG af0  = w*(a2 - a0 - a6 + a8) - a3 + a5;
			TBIG af1  = w*(a6 + a8 - a0 - a2) - a1 + a7; 
			TBIG af2  = a3 - a1 + a5 - a7;

			TBIG bf0  = w*(b2 - b0 - b6 + b8) - b3 + b5;
			TBIG bf1  = w*(b6 + b8 - b0 - b2) - b1 + b7; 
			TBIG bf2  = b3 - b1 + b5 - b7;

			// ??
			unsigned p=0;
			pFeatureData[p++] += lf0 * lf0; 
			pFeatureData[p++] += lf1 * lf1; 
			pFeatureData[p++] += lf2 * lf2; 

			pFeatureData[p++] += af0 * af0; 
			pFeatureData[p++] += af1 * af1; 
			pFeatureData[p++] += af2 * af2; 

			pFeatureData[p++] += bf0 * bf0; 
			pFeatureData[p++] += bf1 * bf1; 
			pFeatureData[p++] += bf2 * bf2; 
		}
	}

	TBIG sizeFactor = (TBIG)1. / (TBIG)((newHeight - 2*border) * (newWidth - 2*border));

	for (unsigned i = 0; i < textureSize(); ++i) 
		pFeatureData[i] = (TBIG)0.004*sqrt(sizeFactor*pFeatureData[i]);

	return true;
}

template <typename TBIG> bool featureGenerator<TBIG>::calculateFeatureData(std::vector<byteRGB>& colors,char* pColorFeatureData)
{	
	if(!pColorFeatureData)
		return false;

	const unsigned width = colors.size();

	if(!width)
		return false;

	// cube to accumulate the color histogram
	array3DV5<TBIG> cube;
	try{
		cube.resize(stepsL,stepsS,stepsH);
	}
	catch(std::bad_alloc){
#ifndef NO_THROW_BADALLOC
		throw;
#endif
		return false;
	}

	TBIG lab[3];
	TBIG bsh[3];

	for (unsigned x=0 ; x<width; ++x) {
		rgb2lab(colors[x],lab);
		lab2bsh(lab, bsh);
		fuzzyQuantization(bsh, 1, cube);
	}

	packCube(cube);

	normalizeHistogram(cube, (TBIG)2.);

	TBIG* featureData;
	//try{
		featureData=new TBIG[histoSize()];
//	}
//	catch(std::bad_alloc){
//#ifndef NO_THROW_BADALLOC
//		throw;
//#endif
//		return false;
//	}

	fillFeatureVector(featureData, cube); 
	
	// Quantisierung
	const TBIG g = (TBIG)0.5;
	for (unsigned i = 0; i < histoSize(); ++i) {
		featureData[i] -= (TBIG)0.5;
		if (featureData[i]>g)
			featureData[i]=g;
		else if (featureData[i]<-g)
			featureData[i]=-g;	

		pColorFeatureData[i] = (char)(featureData[i]*2*127);
	}

	delete[] featureData;
	
	return true;
}


#include "pxfeaturegenerator.expl_templ_inst"

}