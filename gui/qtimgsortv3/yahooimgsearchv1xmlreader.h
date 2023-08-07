#pragma once

#include <QXmlStreamReader>
#include <QQueue>
#include "yahooimgsearchv1result.h"
// TODOV4: don't include a widget here
#include "issearchwidget.h"
class QMutex;

class yahooImgSearchV1XmlReader : public QXmlStreamReader
{
public:
	yahooImgSearchV1XmlReader();
	~yahooImgSearchV1XmlReader();
	// read the xml file and append (!) to the list
	// if pTotalResults!=NULL and pbOk!=NULL, the total results available (as reported in the xml file) are writen to pTotalResults
	// note you have to check wether *pBOk is true after the call...
	// if a QMutex* is passed, read() locks on this mutex when accessing the QQueue
	bool read(QIODevice *device,QQueue<yahooImgSearchV1Result>* pList,bool* pbOk=NULL,unsigned* pTotalResults=NULL,QMutex* pMutex=NULL);

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzen der Suchoptionen für diesen Threads
	*/
	void setSearchOptions(searchOptions options) { m_searchOptions=options; }

private:
	QQueue<yahooImgSearchV1Result>* m_pResults;
	QMutex *m_pMutex;

	yahooImgSearchV1Result m_CurrentResult;

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Variable für die Suchoptionen
	*/
	searchOptions m_searchOptions;

	void readResultSet();
	void readResult();
	void readThumbnail();
	void readOtherElement();

	// these are trivial:
	void readUrl() {m_CurrentResult.m_Url=readElementText();}
	void readClickUrl() {m_CurrentResult.m_ClickUrl=readElementText();}
	void readRefererUrl() {m_CurrentResult.m_RefererUrl=readElementText();}
	void readThumbnailUrl() {m_CurrentResult.m_ThumbnailUrl=readElementText();}
	void readHeight();
	void readWidth();
};