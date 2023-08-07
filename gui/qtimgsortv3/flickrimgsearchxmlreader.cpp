#include "flickrimgsearchxmlreader.h"
#include "flickrimgsearchresult.h"
#include <assert.h>
#include <QMutex>

flickrImgSearchXmlReader::flickrImgSearchXmlReader()
{
	;
}

flickrImgSearchXmlReader::~flickrImgSearchXmlReader()
{
	;
}

bool flickrImgSearchXmlReader::read(QIODevice *device,QQueue<flickrImgSearchResult>* pList,bool* pbOk,unsigned* pTotalResults,QMutex* pMutex)
{
	if(!pList||!device){
		assert(false);
		return false;
	}

	m_pResults=pList;
	// may be NULL:
	m_pMutex=pMutex;
	setDevice(device);

	while(!atEnd()){
		readNext();

		if(isStartElement()){
			if(name()=="rsp"){
				if(attributes().value("stat").toString()=="ok")
					readRsp(pbOk,pTotalResults);
				else
					raiseError(QObject::tr("The file is no flickr photo search result xml file."));
			}
			else
				raiseError(QObject::tr("The file is no flickr photo search result xml file."));
			}
	}

	return !error();
}

void flickrImgSearchXmlReader::readRsp(bool* pbOk,unsigned* pTotalResults)
{
	Q_ASSERT(isStartElement()&&name()=="rsp");

	while (!atEnd()) {
		readNext();

		if(isEndElement())
			break;

		if(isStartElement()){
			if(name()=="photos"){
				if(pTotalResults&&pbOk){
					QString str=attributes().value("total").toString();
					*pTotalResults=str.toUInt(pbOk);
					QString str2=attributes().value("page").toString();
					QString str3=attributes().value("pages").toString();
					QString str4=attributes().value("perpage").toString();
				}
				readPhotos();
			}
			else
				readOtherElement();
		}
	}

}

void flickrImgSearchXmlReader::readPhotos()
{
	Q_ASSERT(isStartElement()&&name()=="photos");

	while (!atEnd()) {
		readNext();

		if(isEndElement())
			break;

		if(isStartElement()){
			if(name()=="photo")
				readPhoto();
			else
				readOtherElement();
		}
	}
}

void flickrImgSearchXmlReader::readPhoto()
{
	// note in flickr, photo is an *empty* xml element with attributes like
	// 
	// <photo id="2651870716" owner="23642613@N03" secret="1d5b213abd" server="3056" farm="4" title="tessie." ispublic="1" isfriend="0" isfamily="0"/>
	Q_ASSERT(isStartElement()&&name()=="photo");

	m_CurrentResult.m_Id=attributes().value("id").toString();
	m_CurrentResult.m_Owner=attributes().value("owner").toString();
	m_CurrentResult.m_Secret=attributes().value("secret").toString();
	m_CurrentResult.m_Server=attributes().value("server").toString();
	m_CurrentResult.m_Farm=attributes().value("farm").toString();
	m_CurrentResult.m_Title=attributes().value("title").toString();
	QString str=attributes().value("ispublic").toString();
	m_CurrentResult.m_bPublic=(str=="1")?1:0;
	str=attributes().value("isfriend").toString();
	m_CurrentResult.m_bFriend=(str=="1")?1:0;
	str=attributes().value("isfamily").toString();
	m_CurrentResult.m_bFamily=(str=="1")?1:0;

	// add current result to queue, lock/unlock if requested
	if(m_pMutex)
		m_pMutex->lock();

	m_pResults->enqueue(m_CurrentResult);

	if(m_pMutex)
		m_pMutex->unlock();

	// read the end element of the empty element
	readNext();

}



void flickrImgSearchXmlReader::readOtherElement()
{
	Q_ASSERT(isStartElement());

	while(!atEnd()){
		readNext();

		if(isEndElement())
			break;

		if(isStartElement())
			readOtherElement();
	}
}