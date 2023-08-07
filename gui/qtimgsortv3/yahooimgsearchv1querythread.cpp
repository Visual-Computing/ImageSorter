#include "yahooimgsearchv1querythread.h"
#include "yahooimgsearchv1xmlreader.h"
#include <QTimer>
#include <QBuffer>
#include <assert.h>

yahooImgSearchV1QueryThread::yahooImgSearchV1QueryThread()
{
	m_uStart=0;
}

yahooImgSearchV1QueryThread::~yahooImgSearchV1QueryThread()
{
	assert(isFinished());
}

void yahooImgSearchV1QueryThread::run()
{
	// set the error flag, so timedOut() can check
	m_bError=true;
	if(!m_pMutex||!m_uNumQueries||!m_pResults||m_Query.isEmpty()){
		return;
	}
	unsigned id=m_Id;
	m_bTimedOut=false;


	// as with the QHttp instance, also the QTimer has to live in the run() thread...
	QTimer timer;
	timer.setSingleShot(true);
	connect(&timer,SIGNAL(timeout()),this,SLOT(timedOut())); 

	QBuffer buf;
	// NOTE: we cannot use a member variable like QHttp m_Http here
	// because our destructor crashes then. the reason is quite tricky:
	// the member Variable lives in another thread (the tread where our constructor is called, 
	// which is different of the thread where run() is called)
	// It seems that the calls to the QHttp instance we issue *here* (like setHost() and get())
	// create several other internal QObjects, the live in this thread (the tread which calls run(), again...)
	// the destructor of the QHttp member variable tries to send signals to these internal QObjects, 
	// and this crashes because Qt cannot send signal to receivers which live in different threads...
	QHttp http;
	http.setHost("search.yahooapis.com");
	// let the QHttp instance exit the event loop on done()
	connect(&http,SIGNAL(done(bool)),this,SLOT(isDone(bool)));
	if(m_uTimeOut)
		timer.start(m_uTimeOut);

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Auslesen der gesetzten Suchoptionen und Setzen der Optionen
		*   in der Anfrage (URL)
		*/
	QString options;
	if(!m_searchOptions.m_Format.isEmpty())
		options += "&format=" + m_searchOptions.m_Format;
	switch(m_searchOptions.m_Coloration) {
		case 1:
			options += "&coloration=color";
			break;
		case 2:
			options += "&coloration=bw";
			break;
	}
	http.get("/ImageSearchService/V1/imageSearch?appid=MISSING&query="+m_Query.toUtf8()+"&results="+QString::number(m_uNumQueries)+"&start="+QString::number(m_uStart)+options,&buf);
	//http.get("/ImageSearchService/V1/imageSearch?appid=MISSING&query="+m_Query.toUtf8()+"&results="+QString::number(m_uNumQueries)+"&start="+QString::number(m_uStart),&buf);
	//http.get("/ImageSearchService/V1/imageSearch?appid=MISSING&query="+m_Query+"&results="+QString::number(m_uNumQueries)+"&start="+QString::number(m_uStart),&buf);


	// run the event loop
	// this returns when the request is finished, either with or w/o error, or when the timer timed out
	exec();

	http.disconnect(this);
	timer.disconnect(this);

	if(m_bTimedOut)
		http.abort();

	timer.stop();

	m_HttpError=http.error();
	m_HttpErrorString=http.errorString();

	if(m_bError){
		return;
	}

	yahooImgSearchV1XmlReader rd;
	buf.open(QIODevice::ReadOnly);
	rd.setSearchOptions(m_searchOptions);
	rd.read(&buf,m_pResults,NULL,NULL,m_pMutex);
	buf.close();
	return;
}

void yahooImgSearchV1QueryThread::isDone(bool bError)
{
	//// if the timer still running, stop it...
	//if(m_pTimer->isActive())
	//	m_pTimer->stop();

	m_bError=bError;
	m_bError?exit(1):exit(0);
}

void yahooImgSearchV1QueryThread::timedOut()
{
	//if(!m_bError){
	//	// something gone wrong, the request has finished,
	//	// but the timer was still active...
	//	assert(false);
	//}
	//m_pHttp->abort();
	m_bTimedOut=true;
	exit(1);
}
