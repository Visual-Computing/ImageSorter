#if !defined(PX_IMAGE_IMPL_H)
#define PX_IMAGE_IMPL_H
#include "pximage.h"
#include "pximpldefs.h"
#include "pxfeaturegenerator.h"

namespace isortisearch{

class ISORT_API imageImpl
{
public:
	imageImpl(){;}
	~imageImpl(){;}

	imageImpl(const imageImpl&);
	imageImpl& operator=(const imageImpl&);
	/*
	calculates the feature data of the image passed in iDesc
	*/
	static bool calculateFeatureData(const imageDescriptor& iDesc){return m_FeatureGenerator.calculateFeatureData(iDesc);}
	/*
	returns the size (in bytes) of the feature data
	*/
	static unsigned __int32 featureDataByteSize() {return FEATURESIZE*sizeof(char);}
	/*
	returns the size (in float) of the feature data
	*/
	static unsigned __int32 featureDataSize() {return FEATURESIZE;}

	bool setFeatureData(unsigned __int32 version,void *data);
	const char *featureData() const {return m_FeatureData;}

	static unsigned __int32 majorVersion() {return m_uMajorVersion;}
	static unsigned __int32 minorVersion() {return m_uMinorVersion;}

	friend class isortisearch::image;
	template<typename TIMG> friend class searchBase;
	template<typename TIMG> friend class searcherImpl;
	template<typename TIMG> friend class dynamicSearcherImpl;
	template<typename TBIG> friend class featureGenerator;

protected:
	static const unsigned __int32 m_uMajorVersion;
	static const unsigned __int32 m_uMinorVersion;

	char m_FeatureData[FEATURESIZE];

	// the feature generator
	static featureGenerator<float> m_FeatureGenerator;
};

}

#endif