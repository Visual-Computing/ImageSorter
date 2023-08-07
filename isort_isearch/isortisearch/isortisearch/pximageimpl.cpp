#include "pximageimpl.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif

namespace isortisearch {
/*
Version history:

1: Initial, used in ImageSorter V4.1
2: feature data is 99 float, used in ImageSorter V4.2
3: feature data is 50 char, used in ImageSorter V4.3
*/
const unsigned __int32 imageImpl::m_uMajorVersion=3;
const unsigned __int32 imageImpl::m_uMinorVersion=0;
featureGenerator<float> imageImpl::m_FeatureGenerator;

imageImpl::imageImpl(const imageImpl& rOther)
{
	memcpy(m_FeatureData,rOther.m_FeatureData,sizeof(m_FeatureData));
}

imageImpl& imageImpl::operator=(const imageImpl& rOther)
{
	if(&rOther!=this){
		memcpy(m_FeatureData,rOther.m_FeatureData,sizeof(m_FeatureData));
	}
	return *this;
}

bool imageImpl::setFeatureData(unsigned __int32 version,void *data)
{
	if(version!=m_uMajorVersion)
		return false;

	if(!data)
		return false;

	assert(sizeof(m_FeatureData)==featureDataByteSize());
	memcpy(m_FeatureData,data,sizeof(m_FeatureData));
	return true;
}


}

#if defined(_LEAK_H)
	#define new new
#endif

