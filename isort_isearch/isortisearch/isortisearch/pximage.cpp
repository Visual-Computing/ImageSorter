#include "pximage.h"
#include "pximageimpl.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
#include <assert.h>

namespace isortisearch{

// implementaion of image
const unsigned __int32 image::m_ValidFeatureDataFlag=1;
const unsigned __int32 image::m_ValidFeatureDataMask=~(1);

image::image() : m_uFlags(0)
{
	m_pImpl=new imageImpl;
}

ISORT_INLINE image::~image()
{
	if(m_pImpl)
		delete m_pImpl;
}

image::image(const image& rOther) : m_uFlags(rOther.m_uFlags)
{
	assert(rOther.m_pImpl);
	m_pImpl=new imageImpl(*(rOther.m_pImpl));
}

image& image::operator=(const image& rOther)
{
	assert(m_pImpl);
	assert(rOther.m_pImpl);
	if(&rOther!=this){
		m_uFlags=rOther.m_uFlags;
		m_pImpl->operator=(*(rOther.m_pImpl));
	}
	return *this;
}

ISORT_INLINE unsigned __int32 image::majorVersion() 
{
	return imageImpl::majorVersion();
}

ISORT_INLINE unsigned __int32 image::minorVersion() 
{
	return imageImpl::minorVersion();
}

ISORT_INLINE bool image::featureDataValid() const
{
	return m_uFlags&m_ValidFeatureDataFlag;
}

bool image::setFeatureData(unsigned __int32 version,void *data)
{
	assert(m_pImpl);
	m_uFlags&=m_ValidFeatureDataMask;

	bool bRet=m_pImpl->setFeatureData(version,data);

	if(bRet)
		m_uFlags|=m_ValidFeatureDataFlag;
	
	return bRet;
}

bool image::calculateFeatureData(const imageDescriptor& iDesc)
{
	return imageImpl::calculateFeatureData(iDesc);
}

const char* image::featureData() const
{
	assert(m_pImpl);
	return m_pImpl->featureData();
}

unsigned __int32 image::featureDataSize()
{
	return imageImpl::featureDataSize();
}

unsigned __int32 image::featureDataByteSize()
{
	return imageImpl::featureDataByteSize();
}

// implementation of sortImage

sortImage::sortImage() : m_uMapXPos(0), m_uMapYPos(0)
{
	;
}

sortImage::sortImage(const sortImage& rOther) :
				 image(rOther),
				 m_uMapXPos(rOther.m_uMapXPos),
				 m_uMapYPos(rOther.m_uMapYPos)
{
	;
}

ISORT_INLINE sortImage::~sortImage()
{
	;
}

sortImage& sortImage::operator=(const sortImage& rOther)
{
	if(&rOther!=this){
		image::operator=(rOther);
		m_uMapXPos=rOther.m_uMapXPos;
		m_uMapYPos=rOther.m_uMapYPos;
	}
	return *this;
}


ISORT_INLINE unsigned __int32 sortImage::mapXPos() const
{
	return m_uMapXPos;
}

ISORT_INLINE unsigned __int32 sortImage::mapYPos() const
{
	return m_uMapYPos;
}
// implementation of controllerImage
const unsigned __int32 controllerImage::m_VisibleFlag=2;
const unsigned __int32 controllerImage::m_VisibleMask=~(2);

controllerImage::controllerImage() : m_uWidth(0),
				 m_uHeight(0),
				 m_UpperLeftX(0),
				 m_UpperLeftY(0),
				 m_LowerRightX(0),
				 m_LowerRightY(0)
{
	m_uFlags|=isortisearch::controllerImage::m_VisibleFlag;
}

controllerImage::controllerImage(const controllerImage& rOther) :
				 sortImage(rOther),
				 m_uWidth(rOther.m_uWidth),
				 m_uHeight(rOther.m_uHeight),
				 m_UpperLeftX(rOther.m_UpperLeftX),
				 m_UpperLeftY(rOther.m_UpperLeftY),
				 m_LowerRightX(rOther.m_LowerRightX),
				 m_LowerRightY(rOther.m_LowerRightY)
{
	;
}

ISORT_INLINE controllerImage::~controllerImage()
{
	;
}

controllerImage& controllerImage::operator=(const controllerImage& rOther)
{
	if(&rOther!=this){
		sortImage::operator=(rOther);
		m_uWidth=rOther.m_uWidth;
		m_uHeight=rOther.m_uHeight;
		m_UpperLeftX=rOther.m_UpperLeftX;
		m_UpperLeftY=rOther.m_UpperLeftY;
		m_LowerRightX=rOther.m_LowerRightX;
		m_LowerRightY=rOther.m_LowerRightY;
	}
	return *this;
}
bool controllerImage::setDimensions(unsigned __int32 width,unsigned __int32 height) 
{
	//if(width<8||height<8)
	//	return false;
	//else if(__max(width,height)<40)
	//	return false;


	//m_uWidth=width;
	//m_uHeight=height;
	if(!setWidth(width))
		return false;

	if(!setHeight(height))
		return false;

	return true;	
}

bool controllerImage::setWidth(unsigned __int32 width) 
{
	if(width<8)
		return false;

	m_uWidth=width;

	return true;	
}

bool controllerImage::setHeight(unsigned __int32 height) 
{
	if(height<8)
		return false;

	m_uHeight=height;

	return true;	
}

ISORT_INLINE unsigned __int32 controllerImage::width() const
{
	return m_uWidth;
}


ISORT_INLINE unsigned __int32 controllerImage::height() const 
{
	return m_uHeight;
}

ISORT_INLINE bool controllerImage::visible() const
{
	return m_uFlags&m_VisibleFlag;
}


}
#if defined(_LEAK_H)
	#define new new
#endif
