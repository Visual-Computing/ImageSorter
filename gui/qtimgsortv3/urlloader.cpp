#include "urlloader.h"
#include <QBuffer>
#include "thumbnail.h"
#include "thumbnailcalculator.h"
#include <assert.h>
#include "pximage.h"
using namespace isortisearch;

urlThumbLoader::urlThumbLoader():m_bCanRun(false),m_bImageLoadError(false)
{
}

urlThumbLoader::~urlThumbLoader()
{
	assert(isFinished());
}

bool urlThumbLoader::setRunArgs(thumbnail* pThumb,unsigned timeOut)
{
	// check input parameters
	// NOTE we have may to parse the thumbnail type from the url path (***)
	if(!pThumb)
		return false;
	
	if(pThumb->isFromYahoo())
		m_Url.setUrl(pThumb->m_YahooURLs.m_ThumbnailUrl);
	else
		m_Url.setUrl(pThumb->m_flickrURLs.thumbnailURL());

	if(!m_Url.isValid())
		return false;

	m_pThumb=pThumb;
	m_uTimeOut=timeOut;
	m_bCanRun=true;
	return true;
}

void urlThumbLoader::run()
{
	// set the error flag, so timedOut() can check
	m_bError=true;
	if(!m_bCanRun){
		return;
	}

	// a caller has to call setRunArgs() again before issuing next start():
	m_bCanRun=false;
	m_bTimedOut=false;

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
	http.setHost(m_Url.host());
	http.get(m_Url.path(),&buf);

	// let the QHttp instance exit the event loop on done()
	connect(&http,SIGNAL(done(bool)),this,SLOT(isDone(bool)));

	// as with the QHttp instance, also the QTimer has to live in the run() thread...
	QTimer timer;
	timer.setSingleShot(true);
	connect(&timer,SIGNAL(timeout()),this,SLOT(timedOut())); 

	if(m_uTimeOut)
		timer.start(m_uTimeOut);

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

	// read the contents of the buffer to the thumbnail QImage member
	// NOTE if the imageformats plugin are not loaded, this fails!
	buf.open(QIODevice::ReadOnly);
	// TODO: check if QImage::load(QIODevice*) really needs a format QString. Other overloads can ommit that...
	m_bImageLoadError=!(m_pThumb->m_Img.load(&buf,"JPG"));
	buf.close();

	if(m_bImageLoadError){
		return;
	}

	// do whatever we need to do with the thumb, then:
	thumbnailCalculator::format(m_pThumb,m_uSize);
	bool b=m_pThumb->setBaseMembers();
	assert(b);

	m_pThumb->setLoadedFromNet();
	m_pThumb->validate();

	thumbnailCalculator::calculateFeatureData(m_pThumb);

	return;
}

void urlThumbLoader::isDone(bool bError)
{
	//// if the timer still running, stop it...
	//if(m_pTimer->isActive())
	//	m_pTimer->stop();

	m_bError=bError;
	m_bError?exit(1):exit(0);
}

void urlThumbLoader::timedOut()
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
