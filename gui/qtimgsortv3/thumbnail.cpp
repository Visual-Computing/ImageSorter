#include "thumbnail.h"
#if defined(QT_NO_DATASTREAM)
# error does not compile with QT_NO_DATASTREAM defined....
#endif

#include <assert.h>
#include <float.h>
#include "array.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
// version history
// this is for image sorter version 2
// reset version history to 1
// image sorter version 1 had up to 4
// 1: the thumbnail itself, parts of the flags() and the feature data
// 2: the URL if isLoadedFromNet()
// 3: the featureData() is 24*double now, was 16*double in version 2
// 4: thumbnail is now a class derived from isortisearch::controllerImage
// 5: feature data of the base class isortisearch::image has changed
const unsigned thumbnail::m_Version=5;
const unsigned thumbnail::m_SelectFlag=4;
const unsigned thumbnail::m_SelectMask=~(4);
const unsigned thumbnail::m_PreSelectFlag=8;
const unsigned thumbnail::m_PreSelectMask=~(8);
const unsigned thumbnail::m_PreDeSelectFlag=16;
const unsigned thumbnail::m_PreDeSelectMask=~(16);

const unsigned thumbnail::m_ValidFlag=32;
const unsigned thumbnail::m_ValidMask=~(32);
const unsigned thumbnail::m_MovedOrDeletedFlag=64;
const unsigned thumbnail::m_MovedOrDeletedMask=~(64);
const unsigned thumbnail::m_NotLoadedFlag=(128);
const unsigned thumbnail::m_NotLoadedMask=~(128);
const unsigned thumbnail::m_INetFlag=(256);
const unsigned thumbnail::m_INetMask=~(256);
const unsigned thumbnail::m_IsOutVersionedFlag=(512);
const unsigned thumbnail::m_IsOutVersionedMask=~(512);
const unsigned thumbnail::m_IsFromYahooFlag=(1024);
const unsigned thumbnail::m_IsFromYahooMask=~(1024);

thumbnail::thumbnail(const thumbnail& rOther) : isortisearch::controllerImage(rOther)
{
	// qt implicit share
	m_Img=rOther.m_Img;

	// qt implicit share
	m_FileName=rOther.m_FileName;
	m_FileSize=rOther.m_FileSize;
	m_LastModified=rOther.m_LastModified;

	m_uFlags=rOther.m_uFlags;

	// qt implicit share
	//m_JpegBinaryData=rOther.m_JpegBinaryData;

	// qt implicit share
	m_highResImage=rOther.m_highResImage;

	m_YahooURLs=rOther.m_YahooURLs;
	m_flickrURLs=rOther.m_flickrURLs;

}



thumbnail& thumbnail::operator=(const thumbnail& rOther)
{
	if(&rOther==this)
		return *this;
	
	// assign base
	isortisearch::controllerImage::operator=(rOther);
	
	// qt implicit share
	m_Img=rOther.m_Img;

	// qt implicit share
	m_FileName=rOther.m_FileName;
	m_FileSize=rOther.m_FileSize;
	m_LastModified=rOther.m_LastModified;

	m_uFlags=rOther.m_uFlags;

	// qt implicit share
	//m_JpegBinaryData=rOther.m_JpegBinaryData;

	// qt implicit share
	m_highResImage=rOther.m_highResImage;

	m_YahooURLs=rOther.m_YahooURLs;
	m_flickrURLs=rOther.m_flickrURLs;


	return *this;

}

bool thumbnail::setBaseMembers()
{
	//setWidth((unsigned)m_Img.width());
	//setHeight((unsigned)m_Img.height());
	return setDimensions((unsigned)m_Img.width(),(unsigned)m_Img.height());;
}

QString thumbnail::nameToSort() {
	if(isLoadedFromNet())
		return isFromYahoo() ? m_YahooURLs.m_ClickUrl : m_flickrURLs.clickURL();
	else
		return m_FileName;
	return QString();
}

QDataStream &operator>>(QDataStream &in,thumbnail &t)
{
	quint32 ver;
	in >> ver;
	// note initial version is/was 1
	// current version is 5
	// since we have a new filecache, we don't load older versions any longer
	if(ver<1||ver>thumbnail::m_Version){
		in.setStatus(QDataStream::ReadCorruptData);
		return in;
	}

	// load version 5 data

	// the thumbnail image
	// note this does not work, the data stream id corrupt after this...
	// but it's status() is OK
	// t.m_Img.load(in.device(),"JPG");
	// thus:
	quint32 nullMarker;
	in >> nullMarker;
	assert(!nullMarker);
	if(nullMarker)
		t.m_Img=QImage();
	else{
		// load into the thumbnails byte array
		// V4.3: the jpeg binary data is no longer a thumbnail member
		// t.m_JpegBinaryData.clear();
		// in >> t.m_JpegBinaryData;
		// QBuffer buffer(&(t.m_JpegBinaryData));
		QByteArray jpegBinaryData;
		in >> jpegBinaryData;
		QBuffer buffer(&(jpegBinaryData));
		buffer.open(QIODevice::ReadOnly);
		t.m_Img.load(&buffer,"JPG");
		buffer.close();
		bool b=t.setBaseMembers();
		assert(b);
	}
	

	// the flags
	unsigned __int32 f;
	in >> f;
	t.setFlags(f);

	// this is essential: as long as the loading code has not confirmed that there is an image
	// in the current folder which is 'like' this thumbnail, the thumb is invalid.
	// 'is like' means: delivers same hash value...
	t.invalidate();
	assert(!t.isMovedOrDeleted());

	// note version 5 uses the vBase V2
	// first load the image class major version
	bool bFeatureDataLoaded=false;
	unsigned __int32 major;
	in >> major;
	// compare it to the actual version
	if(major==isortisearch::image::majorVersion()){
		// next read the size of the stored feature data
		unsigned __int32 sz;
		in >> sz;
		// and compare it to the actual size
		// note this is the number of floats!!!
		if(sz==isortisearch::image::featureDataSize()){
			// now load and set the data
			array<char> data(sz);
			// funny enough, there is no operator>> for 'char':
			__int32 val;
			for(unsigned i=0;i<sz;++i){
				in >> val;
				data[i] = (char) val;
			}
			//in.readRawData((char*)(data.pVals()),sz*sizeof(float));
			bFeatureDataLoaded=t.setFeatureData(major,data.pVals());
			assert(bFeatureDataLoaded);
		}
	}
	if(!bFeatureDataLoaded)
		t.setOutVersioned();

	// load 'file info'

	in >> t.m_FileName;
	in >> t.m_FileSize;
	in >> t.m_LastModified;

	t.setBaseMembers();

	return in;
}

QDataStream &operator<<(QDataStream &out, const thumbnail &t)
{
	// do not save thumbs from the net
	if(t.isLoadedFromNet())
		return out;

	// save version
	quint32 ver=thumbnail::m_Version;
	out << ver;

	// save version 5 data

	// the thumbnail image

	// this leaves the QDataStream in a mess, although status() is OK:
	// t.m_Img.save(out.device(),"JPG",100);

	const quint32 nullMarker=t.m_Img.isNull()?1:0;
	out << nullMarker;
	if(!nullMarker){
		// if the thumbnails byte array is empty,
		// the thumb wasn't loaded from the database...

		// V4.3: the jpeg binary data is no longer a thumbnail member,
		// it is just written to the cahe file (see below)
		// we never write thumbnails twice to the cache, 
		// thus there is no degradation im image quality...

		//if(t.m_JpegBinaryData.isEmpty()){
		//	QByteArray barray;
		//	QBuffer buffer(&barray);
		//	buffer.open(QIODevice::WriteOnly);
		//	t.m_Img.save(&buffer,"JPG",30);
		//	buffer.close();
		//	out << barray;
		//}
		//else
		//	// write back the byte array...
		//	out << t.m_JpegBinaryData;

		QByteArray jpegBinaryData;
		QBuffer buffer(&jpegBinaryData);
		buffer.open(QIODevice::WriteOnly);
		t.m_Img.save(&buffer,"JPG",30);
		buffer.close();
		out << jpegBinaryData;
	}

	// the flags
	out << t.flags();

	// the feature data
	// first the major version of the library image
	out << isortisearch::image::majorVersion();
	// then the site (number of floats) of the data
	out << isortisearch::image::featureDataSize();
	// then the data as floats
	unsigned sz=isortisearch::image::featureDataSize();
	__int32 val;
	for(unsigned i=0;i<sz;++i){
		val=(__int32) *(t.featureData()+i);
		out << val;
	}

	// the file info data
	out << t.m_FileName;
	out << t.m_FileSize;
	out << t.m_LastModified;

	return out;
}

//QDataStream &operator>>(QDataStream &in,fixedArray<double,FEATURESIZE> &a)
//{
//	assert(a.size()==FEATURESIZE);
//	unsigned i;
//	for(i=0;i<FEATURESIZE;++i)
//		in >> a[i];
//
//	return in;
//}
//
//QDataStream &operator<<(QDataStream &out, const fixedArray<double,FEATURESIZE> &a)
//{
//	assert(a.size()==FEATURESIZE);
//	unsigned i;
//	for(i=0;i<FEATURESIZE;++i)
//		out << a[i];
//
//	return out;
//}
//
