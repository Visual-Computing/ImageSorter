#include "yahooimgsearchv1xmlreader.h"
#include "yahooimgsearchv1result.h"
#include <assert.h>
#include <QMutex>

yahooImgSearchV1XmlReader::yahooImgSearchV1XmlReader()
{
	;
}

yahooImgSearchV1XmlReader::~yahooImgSearchV1XmlReader()
{
	;
}

bool yahooImgSearchV1XmlReader::read(QIODevice *device,QQueue<yahooImgSearchV1Result>* pList,bool* pbOk,unsigned* pTotalResults,QMutex* pMutex)
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
			// a very ignorant test if this is the right xml file
			//if(name()=="ResultSet"&&attributes().value("","xmlns")=="urn:yahoo:srchmi")
			//	readResultSet();
			if(name()=="ResultSet"){
				if(pTotalResults&&pbOk){
					QString str=attributes().value("totalResultsAvailable").toString();
					*pTotalResults=str.toUInt(pbOk);
				}
				//QXmlStreamAttributes attr=attributes();
				////attributes().value("","xmlns")=="urn:yahoo:srchmi")
				readResultSet();
			}
			else
				raiseError(QObject::tr("The file is no yahoo Image Search API V1 file."));
			}
	}

	return !error();
}

void yahooImgSearchV1XmlReader::readResultSet()
{
	Q_ASSERT(isStartElement()&&name()=="ResultSet");

	while (!atEnd()) {
		readNext();

		if(isEndElement())
			break;

		if(isStartElement()){
			if(name()=="Result")
				readResult();
			else
				readOtherElement();
		}
	}

}

void yahooImgSearchV1XmlReader::readResult()
{
	Q_ASSERT(isStartElement()&&name()=="Result");

	while (!atEnd()) {
		readNext();

		if(isEndElement())
			break;

		if(isStartElement()){
			if(name()=="Url")
				readUrl();
			else if(name()=="ClickUrl")
				readClickUrl();
			else if(name()=="RefererUrl")
				readRefererUrl();
			else if(name()=="Thumbnail")
				readThumbnail();
			else if(name()=="Height")
				readHeight();
			else if(name()=="Width")
				readWidth();
			else
				readOtherElement();
		}
	}

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Ausfiltern der Internetbilder aus der XML-Datei
	*/
	// Prüfen ob ein Bild im Hoch- oder Querformat ist
	bool addElement=true;
	if(m_searchOptions.m_Orientation==1 && m_CurrentResult.m_Width >= m_CurrentResult.m_Height)
		addElement=false;
	else if(m_searchOptions.m_Orientation==2 && m_CurrentResult.m_Width <= m_CurrentResult.m_Height)
		addElement=false;

	// Festlegen einer Bildgröße für small und medium, large sind alle Bilder größer medium
	unsigned int small=400;
	unsigned int medium=1400;
	unsigned int imageSize = m_CurrentResult.m_Width + m_CurrentResult.m_Height;
	// Prüfen ob ein Bild die entsprechende Größe hat
	if(m_searchOptions.m_Size==1 && imageSize > small )
		addElement=false;
	else if(m_searchOptions.m_Size==2 && (imageSize <= small || imageSize > medium) )
		addElement=false;
	else if(m_searchOptions.m_Size==3 && imageSize < medium )
		addElement=false;


	// Nur wenn das Bild den Suchoptionen entspricht, wird es hinzugefügt
	if(addElement) {
		// add current result to queue, lock/unlock if requested
		if(m_pMutex)
			m_pMutex->lock();

		m_pResults->enqueue(m_CurrentResult);

		if(m_pMutex)
			m_pMutex->unlock();
	}
}

void yahooImgSearchV1XmlReader::readThumbnail()
{
	Q_ASSERT(isStartElement()&&name()=="Thumbnail");

	while (!atEnd()) {
		readNext();

		if(isEndElement())
			break;

		if(isStartElement()){
			if(name()=="Url")
				readThumbnailUrl();
			else
				readOtherElement();
		}
	}
}



void yahooImgSearchV1XmlReader::readOtherElement()
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

void yahooImgSearchV1XmlReader::readHeight()
{
	bool bOk;

	m_CurrentResult.m_Height=readElementText().toUInt(&bOk);
	if(!bOk)
		m_CurrentResult.m_Height=0;
}

void yahooImgSearchV1XmlReader::readWidth()
{
	bool bOk;

	m_CurrentResult.m_Width=readElementText().toUInt(&bOk);
	if(!bOk)
		m_CurrentResult.m_Width=0;
}
