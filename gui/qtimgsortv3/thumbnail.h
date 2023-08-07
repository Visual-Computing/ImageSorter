#pragma once
#include "pximage.h"
#include "imagesort.h"
#include <qtgui>

#include "yahooimgsearchv1result.h"
#include "flickrimgsearchresult.h"

/*!
a thumbnail along with some properties
*/
class thumbnail : public isortisearch::controllerImage
{
public:
	thumbnail() {;}
	virtual ~thumbnail(){;}
	
	thumbnail(const thumbnail& rOther);
	thumbnail& operator=(const thumbnail& rOther);

	// the 'name', either the yahoo url, flickr url or the file info
	// Note only one of these will contain meaningful values
	yahooImgSearchV1Result m_YahooURLs;
	flickrImgSearchResult m_flickrURLs;
	// V5: we need to store the file size and last modified date for the new thumbscache
	// however, there is noc way to save a QFileInfo, thus
	// QFileInfo m_FileInfo;
	QString m_FileName;
	quint64 m_FileSize;
	QDateTime m_LastModified;

	// the image to sort
	// call this after the QImage is set....
	bool setBaseMembers();
	QImage m_Img;

	// the high resolution version of this image 
	// (only loaded when needed)
	QImage m_highResImage;

	// flag handling
	void resetFlags() {m_uFlags=0;}

	// the valid flag is for handling the thumbnail cache
	// QDataStream &operator>>(QDataStream &, thumbnail &) invalidates the thumbnail on load
	// imageSortWorker::_load_dir() validates the thumbnail when loaded from the cache 
	// if the corresponding image file is still available and still unchanged
	// invalid thumbs are deleted from the cache
	// thus, after loading all loaded thumbnails are valid...
	bool isValid() const {return m_uFlags&m_ValidFlag;}
	void validate() {m_uFlags|=m_ValidFlag;}
	void invalidate() {m_uFlags&=m_ValidMask;}

	bool isMovedOrDeleted() const {return m_uFlags&m_MovedOrDeletedFlag;} 
	void setMovedOrDeleted() {m_uFlags|=m_MovedOrDeletedFlag;deSelect();} 
	void unsetMovedOrDeleted() {m_uFlags&=m_MovedOrDeletedMask;}

	bool isNotLoaded() const {return m_uFlags&m_NotLoadedFlag;}  
	void setNotLoaded() {m_uFlags|=m_NotLoadedFlag;} 
	void setLoaded() {m_uFlags&=m_NotLoadedMask;}

	bool selected() const {return m_uFlags&m_SelectFlag;}
	// note a isSelected() is never isPreSelected()...
	void select() {m_uFlags|=m_SelectFlag;dePreSelect();} 
	void deSelect() {m_uFlags&=m_SelectMask;dePreDeSelect();}

	bool preSelected() const {return m_uFlags&m_PreSelectFlag;} 
	void preSelect() {m_uFlags|=m_PreSelectFlag;} 
	void dePreSelect() {m_uFlags&=m_PreSelectMask;}

	bool preDeSelected() const {return m_uFlags&m_PreDeSelectFlag;} 
	void preDeSelect() {m_uFlags|=m_PreDeSelectFlag;} 
	void dePreDeSelect() {m_uFlags&=m_PreDeSelectMask;}

	bool isLoadedFromNet() const {return m_uFlags&m_INetFlag;}
	void setLoadedFromNet() {m_uFlags|=m_INetFlag;} 
	void setLoadedFromDisk() {m_uFlags&=m_INetMask;}

	bool isOutVersioned() const {return m_uFlags&m_IsOutVersionedFlag;}  
	void setOutVersioned() {m_uFlags|=m_IsOutVersionedFlag;} 
	void setNotOutVersioned() {m_uFlags&=m_IsOutVersionedMask;}

	// valid only if isLoadedFromNet()...
	bool isFromYahoo() const {return m_uFlags&m_IsFromYahooFlag;}
	bool isFromFlickr() const {return !isFromYahoo();}
	void setFromYahoo() {m_uFlags|=m_IsFromYahooFlag;} 
	void setFromFlickr() {m_uFlags&=m_IsFromYahooMask;}

	void setUpperLeftX(unsigned __int32 val) {m_UpperLeftX=val;}
	void setUpperLeftY(unsigned __int32 val) {m_UpperLeftY=val;}
	void setLowerRightX(unsigned __int32 val) {m_LowerRightX=val;}
	void setLowerRightY(unsigned __int32 val) {m_LowerRightY=val;}

	// a version member for de/serializing
	static const unsigned m_Version;
	// if loaded from the thumbs database, the binary jpeg data, else isEmpty() is true...
	// if not empty, this is written back to the thumbs data base. thus we don't have
	// no loss in quality over thumbnail database generations...
	// V4.3. we never write thumbnails back to the cahe if once loaded from the cache, thus we don't 
	// need this any more. Of course, the thumbnail cache file contains this, but m_Img is set by this in the loader...
	// QByteArray m_JpegBinaryData;

	QString nameToSort();
	qint64 sizeToSort() const {return isLoadedFromNet()?(isFromYahoo()?((qint64)m_YahooURLs.m_Height*(qint64)m_YahooURLs.m_Width):0):m_FileSize;}

	QRect drawnRect() const {QRect r(upperLeftX(),upperLeftY(),drawnWidth(),drawnHeight()); return r;}
	int drawnHeight() const {return lowerRightY()-upperLeftY();}
	int drawnWidth() const {return lowerRightX()-upperLeftX();}

	// note sorting by the original image dims in case of local disk image does *not* work, because we have the thumbs size...
	//qint64 sizeToSort() const {return isLoadedFromNet()?(isFromYahoo()?((qint64)m_YahooURLs.m_Height*(qint64)m_YahooURLs.m_Width):0):(qint64)(m_Img.width())*(qint64)(m_Img.height());}

	static const unsigned m_ValidFlag;
	static const unsigned m_ValidMask;
	static const unsigned m_SelectFlag;
	static const unsigned m_SelectMask;
	static const unsigned m_PreSelectFlag;
	static const unsigned m_PreSelectMask;
	static const unsigned m_PreDeSelectFlag;
	static const unsigned m_PreDeSelectMask;
	static const unsigned m_MovedOrDeletedFlag;
	static const unsigned m_MovedOrDeletedMask;
	static const unsigned m_NotLoadedFlag;
	static const unsigned m_NotLoadedMask;
	static const unsigned m_INetFlag;
	static const unsigned m_INetMask;
	static const unsigned m_IsOutVersionedFlag;
	static const unsigned m_IsOutVersionedMask;
	static const unsigned m_IsFromYahooFlag;
	static const unsigned m_IsFromYahooMask;
};

#if !defined(QT_NO_DATASTREAM)
	#include <qtcore>
	QDataStream &operator<<(QDataStream &, const thumbnail &);
	QDataStream &operator>>(QDataStream &, thumbnail &);
#endif
