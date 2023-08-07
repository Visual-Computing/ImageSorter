#include "flickrimgsearchquerythread.h"
#include "flickrimgsearchxmlreader.h"
#include <QTimer>
#include <QBuffer>
#include <assert.h>

flickrImgSearchQueryThread::flickrImgSearchQueryThread()
{
	m_uStart=0;
}

flickrImgSearchQueryThread::~flickrImgSearchQueryThread()
{
	assert(isFinished());
}

void flickrImgSearchQueryThread::run()
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
	http.setHost("api.flickr.com");
	// let the QHttp instance exit the event loop on done()
	connect(&http,SIGNAL(done(bool)),this,SLOT(isDone(bool)));
	if(m_uTimeOut)
		timer.start(m_uTimeOut);

	// adapt m_uStart (from yahoo image search) to flickr page number
	unsigned page=m_uStart;
	page--;
	page/=m_uNumQueries;
	page++;
	// sort = relevance
	http.get("/services/rest/?method=flickr.photos.search&api_key=MISSING&text="+m_Query.toUtf8()+"&per_page="+QString::number(m_uNumQueries)+"&page="+QString::number(page)+"&sort=relevance",&buf);
	//http.get("/services/rest/?method=flickr.photos.search&api_key=MISSING&text="+m_Query.toUtf8()+"&per_page="+QString::number(m_uNumQueries)+"&page="+QString::number(page),&buf);
	

	
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

	flickrImgSearchXmlReader rd;
	buf.open(QIODevice::ReadOnly);
	rd.read(&buf,m_pResults,NULL,NULL,m_pMutex);
	buf.close();
	return;
}

void flickrImgSearchQueryThread::isDone(bool bError)
{
	//// if the timer still running, stop it...
	//if(m_pTimer->isActive())
	//	m_pTimer->stop();

	m_bError=bError;
	m_bError?exit(1):exit(0);
}

void flickrImgSearchQueryThread::timedOut()
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
