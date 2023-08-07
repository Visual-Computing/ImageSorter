#include "yahooimagesearchv1querydispatcher.h"
#include "yahooimgsearchv1querythread.h"
#include "imagesortworker.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
yahooImgSearchV1QueryDispatcher::yahooImgSearchV1QueryDispatcher()
{
	m_uNumThreads=2;
	m_bAbort=false;
	m_bHttpError=false;
	// note this is essential...
	moveToThread(this);
}

yahooImgSearchV1QueryDispatcher::~yahooImgSearchV1QueryDispatcher()
{
	unsigned i;
	for(i=0;i<m_QueryThreads.size();++i){
		delete m_QueryThreads[i];
	}

	m_QueryThreads.clear();
}

void yahooImgSearchV1QueryDispatcher::run()
{
	if(!m_uNumThreads||!m_pResults||!m_uNumQueries||!m_uNumQueriesPerThread||m_Query.isEmpty()){
		assert(false);
		return;
	}

	m_bAbort=m_bHttpError=false;

	// note don't have more threads than queries
	unsigned numThreads=qMin(m_uNumThreads,m_uNumQueries);
	// create the threads we need
	while(m_QueryThreads.size()<numThreads){
		yahooImgSearchV1QueryThread* p=new yahooImgSearchV1QueryThread;
		p->setQuery(m_Query);
		p->setSearchOptions(m_searchOptions);
		p->setResults(m_pResults);
		p->setMutex(&m_Mutex);
		p->setNumQueries(m_uNumQueriesPerThread);
		p->setTimeOut(5000);
		m_QueryThreads.append(p);
	}

	assert(m_QueryThreads.size()==numThreads);

	// note the result queue already contains results...
	m_uStart=m_pResults->size()+1;

	// set the id's and the first run args, then start the threads
	for(unsigned i=0;i<numThreads;++i){
		m_QueryThreads[i]->setId(i);
		connect(m_QueryThreads[i],SIGNAL(finished()),this,SLOT(childFinished()),Qt::QueuedConnection);
		m_QueryThreads[i]->setStart(m_uStart);
		m_uStart+=m_uNumQueriesPerThread;
		m_QueryThreads[i]->start();
	}

	// run the event loop
	exec();

	// we may have still child threads running, wait for them
	bool bDone=false;
	while(!bDone){
		bDone=true;
		for(unsigned i=0;i<numThreads;++i){
			if(!m_QueryThreads[i]->isFinished()){
				bDone=false;
				//break;
			}
		}
	}

}

void yahooImgSearchV1QueryDispatcher::childFinished()
{
	// mutual exclusive...
	m_Mutex.lock();

	// check (only once) if to abort
	if(!m_bAbort){
		m_pWorker->lock();
		m_bAbort=m_pWorker->m_bAbort;
		m_pWorker->unlock();
	}

	bool bExit=false;
	// get the first loader which is finished
	unsigned i,id;
	bool bFound=false;
	bool bActive=false;
	for(i=0;i<m_QueryThreads.size();++i){
		if(m_QueryThreads[i]->isFinished()){
			if(!bFound){
				id=i;
				bFound=true;
			}
		}
		else
			bActive=true;
		}
	if(m_QueryThreads[id]->hasHttpError()){
		// one of the threads has an error. it is most likely that
		// other threads will have an error also. so don't start any new threads any more.
		// we may build in better handling later, i.e. based on the QHttp::Error...
		m_bHttpError=true;
		m_HttpErrorString=m_QueryThreads[id]->httpErrorString();
	}
	unsigned sz=m_pResults->size();
	// stop if we have enough results
	if(sz>=m_uNumQueries)
		bExit=true;
	//else /*if(!bActive)*/{
	else if(!m_bAbort&&!m_bHttpError){
		m_QueryThreads[id]->setStart(m_uStart);
		m_uStart+=m_uNumQueriesPerThread;
		m_QueryThreads[id]->start();
	}
	m_Mutex.unlock();

	if(m_bAbort||m_bHttpError)
		bExit=true;
	
	if(bExit&&!bActive)
		exit(0);
}
