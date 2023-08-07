#pragma once
#include "imagev2.h"
#include "array.h"

/*!
a very simple feature extractor which gets the mean values of an image
*/
template <typename T,typename TBIG=double> class meanExtractor
{
public:
	meanExtractor() : m_BgColor(0,0,0) {;}
	~meanExtractor() {;}
	/*!
	extracts the mean values from image rImg and returns an array<> of these

	note the caller has to take the array, i.e. has to delete it
	*/
	array<T>* extract(imagev2<T>& rImg);
	/*!
	extracts the mean values from image rImg to the array passed
	*/
	void extract(imagev2<T>& rImg,array<T>& rArr);
	/*!
	extracts the mean values from image rImg to the array passed

	pixels with alpha channel values of 100% transparency are ignored, 
	all other pixels contribute fully. 

	if all pixels are ignored, false is returned, otherwise true

	note that rImg has to have colorSpace ARGB, RGBA, ABGR or BGRA otherwise this throws

	note that rArr.size() has to be 3, otherwise this throws
	*/
	bool extractSkipAlpha(imagev2<T>& rImg,array<T>& rArr);
	/*!
	sets the background color to be used in exctractApplyAlpha()

	the default is black
	*/
	void setBgColor(const rgb<T>& color) {m_BgColor=color;}
	/*!
	extracts the mean values from image rImg to the array passed

	alpha channel values are applied, i.e. rgb images are blended with the 
	color set by setBGColor().

	note that rImg has to have colorSpace ARGB, RGBA, ABGR or BGRA.
	*/
	void extractApplyAlpha(imagev2<T>& rImg,array<T>& rArr);
protected:
	meanExtractor(const meanExtractor& rOther);
	meanExtractor& operator=(const meanExtractor& rOther);
	rgb<T> m_BgColor;
	// a faster version for black bg
	void extractApplyAlphaBlackBG(imagev2<T>& rImg,array<T>& rArr);
};

/*!
an implementation for unsigned char,double

we can do alpha calculations through LUTs here

note this is useful only if you have to extract the mean value by applying alpha channel
values from a big set of images without changing the bg color.
*/

class byteMeanExtractor : public meanExtractor<unsigned char,double>
{
public:
	byteMeanExtractor();
	~byteMeanExtractor() {;}
	/*!
	calculates the color LUTs if the bg color passed is not the current.

	note that black BG color is the default
	*/
	void setBgColor(const byteRGB& color);
	/*!
	extracts the mean values from image rImg to the array passed

	alpha channel values are applied, i.e. rgb images are blended with the 
	color set by setBGColor().

	note that rImg has to have colorSpace ARGB, RGBA, ABGR or BGRA.
	*/
	void extractApplyAlpha(imagev2<unsigned char>& rImg,array<unsigned char>& rArr);
protected:
	byteMeanExtractor(const byteMeanExtractor& rOther);
	byteMeanExtractor& operator=(const byteMeanExtractor& rOther);
	// the LUTs for BG and FG alpha factors
	double m_BgRedLUT[256];
	double m_BgGreenLUT[256];
	double m_BgBlueLUT[256];
	double m_FgLUT[256];
	void calculateLUTs();
	// a faster version for black bg
	void extractApplyAlphaBlackBG(imagev2<unsigned char>& rImg,array<unsigned char>& rArr);

};



