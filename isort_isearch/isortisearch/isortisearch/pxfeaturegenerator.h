#if !defined(PXFEATUREGENERATOR_H)
#define PXFEATUREGENERATOR_H

//#include "pximage.h"
#include "arrayv5.h"
#include "color.h"
#include <vector>

namespace isortisearch{

// forward

template <typename TBIG> class ISORT_API featureGenerator
{
public:
	featureGenerator() {;}
	~featureGenerator() {;}
	/*!
	calculates the feature data of the image passed in iDesc

	Note the feature data is stored in the isortisearch::image encapsulated in iDesc

	note throws std::bad_alloc if allocation fails
	*/
	bool calculateFeatureData(const imageDescriptor& iDesc);
	/*!
	calculates the feature data of the plain colors passes in colors
	
	note throws std::bad_alloc if allocation fails
	*/
	bool calculateFeatureData(std::vector<byteRGB>& colors,char *pColorFeatureData);

	// note not really needed, but make sure:
	// assert(colorLayoutSize()+histoEdgeFeatureSize()==FEATURESIZE)
	static unsigned __int32 colorLayoutSize() {return 15;}
	static unsigned __int32 colorLayoutOffset() {return 0;}
	// (8-1 colors + grey) +  black + white + 2x color variance (?) (20)
	static unsigned __int32 histoSize() {return stepsH*(stepsL-2) + 2 + 2;}
	static unsigned __int32 histoOffset() {return colorLayoutSize();}
	static unsigned __int32 edgeSize() {return 6;}
	static unsigned __int32 edgeOffset() {return histoSize()+histoOffset();}
	static unsigned __int32 textureSize() {return 9;} 
	static unsigned __int32 textureOffset() {return edgeSize()+edgeOffset();}

private:
	// note no copy ctor, no assignment
	featureGenerator(const featureGenerator& rOther);
	const featureGenerator& operator=(const featureGenerator& rOther);

	bool calculateColorLayout(TBIG* pFeatureData,const imageDescriptor& iDesc);
	bool calculateColorHistogram(TBIG* pFeatureData,const imageDescriptor& iDesc);
	// note throws std::bad_alloc if allocation fails
	bool calculateEdgeFeature(TBIG* pFeatureData,const imageDescriptor& iDesc);
	bool calculateColorTexture(TBIG* pFeatureData,const imageDescriptor& iDesc);
	// helpers
	void rgb2lab(byteRGB rgb,TBIG* pLAB);
	void lab2bsh(TBIG* lab,TBIG* bsh);
	void fuzzyQuantization(TBIG* lsh,TBIG w,array3DV5<TBIG>& cube);
	void packCube(array3DV5<TBIG>& cube);
	void normalizeHistogram(array3DV5<TBIG>& cube,TBIG w);
	void fillFeatureVector(TBIG *pFeatureData,array3DV5<TBIG>& cube);


	// steps for luminance in histogram
	static const int stepsL;
	// steps for saturation in histogram
	static const int stepsS;
	// steps for hue in histogram
	static const int stepsH;  

	// bin size for luminance in histogram
	static const int deltaL;
	// bin size for saturation in histogram
	static const int deltaS; 
	// bin size for hue in histogram
	static const int deltaH; 

	static const int rangeL;
	static const int rangeS;
	static const int rangeH;

	static const int rangeL2;

	static const TBIG T0;
	static const TBIG T1; 
	static const TBIG T2;


	};
}

#endif