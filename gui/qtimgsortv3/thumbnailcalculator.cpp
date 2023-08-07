#include "thumbnailcalculator.h"
#include "colorlayoutextractor.h"
#include <QImage>
#include <assert.h>
#include <float.h>
#include "pximage.h"
using namespace isortisearch;

void thumbnailCalculator::format(thumbnail *p,unsigned size)
{
	if(!p){
		assert(false);
		return;
	}
	// we change the image to format RGB if not in format ARGB or RGB
	if(p->m_Img.format()!=QImage::Format_RGB32&&
		p->m_Img.format()!=QImage::Format_ARGB32){
			p->m_Img=p->m_Img.convertToFormat(QImage::Format_RGB32);
	}

	QSize sz=p->m_Img.size();

	// BUG FIX V2.03
	// when scaling an image with extreme ratios of width/height or heigth/width
	// one of the dimensions can be 0 afterwards when using QSize, leading to a div by zero below.
	// NOTE that depending on the debugger exceptions settings (MSVC), this does not
	// neccesarily break in the debugger, but definitely crashes in the release mode :(
	// the bug fix is to use QSizeF AND to check for dimensions NEAR 0 afterwards
	QSizeF szf(sz);
	// scale, keep aspect ratio
	szf.scale(size,size,Qt::KeepAspectRatio);

	// NOTE are these tests for zero ok?
	if(szf.width()==0.0)
		szf.rwidth()=DBL_MIN;
	if(szf.height()==0.0)
		szf.rheight()=DBL_MIN;

	// make sure bigger side is at least 40 pixels wide
	if(szf.width()>=szf.height()){
		if(szf.width()<40.){
			double f=40./szf.width();
			szf.scale(40.,szf.height()*f,Qt::IgnoreAspectRatio);
		}
	}
	else{
		if(szf.height()<40.){
			double f=40./szf.height();
			szf.scale(szf.width()*f,40.,Qt::IgnoreAspectRatio);
		}
	}
	// make sure smaller side is at least 8 pixels wide
	if(szf.width()<=szf.height()){
		if(szf.width()<8.){
			double f=8./szf.width();
			szf.scale(8.,szf.height()*f,Qt::IgnoreAspectRatio);
		}
	}
	else{
		if(szf.height()<8.){
			double f=8./szf.height();
			szf.scale(szf.width()*f,8.,Qt::IgnoreAspectRatio);
		}
	}
	sz=szf.toSize();

	// we scale the image to our internal width/heigth (whatever is bigger)
	if(p->m_Img.width()<=p->m_Img.height()){
		// note in case of width/height equality, this is ok, too...
		if(p->m_Img.height()!=sz.height())
			p->m_Img=p->m_Img.scaledToHeight(sz.height());
	}
	else{
		if(p->m_Img.width()!=sz.width())
			p->m_Img=p->m_Img.scaledToWidth(sz.width());
	}

}
void thumbnailCalculator::calculateFeatureData(thumbnail *p)
{
	if(!p){
		assert(false);
		return;
	}
	//p->setBaseMembers();
	imageDescriptor iDesc;
	iDesc.m_Width=p->width();
	iDesc.m_Height=p->height();
	// take care to call the const version of QImage::bits(), the non-const 
	// does a deep copy of the image...
	iDesc.m_pFirstPixel=(const unsigned __int32*)((const uchar*)(p->m_Img.bits()));
	// note we may have to change this on dofferent byte order platforms
	iDesc.m_ChannelLayout=isortisearch::ARGB;
	iDesc.m_pImage=(controllerImage*)(p);
	// note this is alpha ignore mode
	isortisearch::image::calculateFeatureData(iDesc);
}