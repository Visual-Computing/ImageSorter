#pragma once

#include <QXmlStreamReader>
#include <QQueue>
#include "flickrimgsearchresult.h"
class QMutex;

class flickrImgSearchXmlReader : public QXmlStreamReader
{
public:
	flickrImgSearchXmlReader();
	~flickrImgSearchXmlReader();
	// read the xml file and append (!) to the list
	// if pTotalResults!=NULL and pbOk!=NULL, the total results available (as reported in the xml file) are writen to pTotalResults
	// note you have to check wether *pBOk is true after the call...
	// if a QMutex* is passed, read() locks on this mutex when accessing the QQueue
	bool read(QIODevice *device,QQueue<flickrImgSearchResult>* pList,bool* pbOk=NULL,unsigned* pTotalResults=NULL,QMutex* pMutex=NULL);
private:
	QQueue<flickrImgSearchResult>* m_pResults;
	QMutex *m_pMutex;

	flickrImgSearchResult m_CurrentResult;

	void readRsp(bool* pbOk,unsigned* pTotalResults);
	void readPhotos();
	void readPhoto();
	void readOtherElement();

};