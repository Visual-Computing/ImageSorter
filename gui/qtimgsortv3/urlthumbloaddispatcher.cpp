#include "urlthumbloaddispatcher.h"
#include "urlloader.h"
#include "imagesortwidget.h"
#include "imagesortworker.h"
// by widget and worker: #include "thumbnail.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
urlThumbLoadDispatcher::urlThumbLoadDispatcher()
{
	m_uFilesLoaded=0;
	m_uNumThreads=2;
	m_uNumRepaints=10;
	m_bAbort=m_bSearchSimilar=false;
	// note this is essential...
	moveToThread(this);
}

urlThumbLoadDispatcher::~urlThumbLoadDispatcher()
{
	unsigned i;
	assert(m_Thumbs.size()==m_Loaders.size());
	for(i=0;i<m_Loaders.size();++i){
		delete m_Loaders[i];
		delete m_Thumbs[i];
	}

	m_Loaders.clear();
	m_Thumbs.clear();
}

void urlThumbLoadDispatcher::setWorker(imageSortWorker* pWorker)
{
	m_pWorker=pWorker;
}

void urlThumbLoadDispatcher::run()
{
	if(!m_uNumThreads||!m_pWorker){
		assert(false);
		return;
	}

	m_bAbort=false;

	m_ExitIDs.clear();
	// this is just a helper for the updateStatusBar2() signal...
	m_uFilesLoaded=0;

	bool bYahooURLs=m_pWorker->m_bYahooImgSearch;
	// note don't have more threads than urls
	unsigned sz=(bYahooURLs)?(unsigned)(m_pWorker->m_YahooImgSearchV1Results.size()):(unsigned)(m_pWorker->m_flickrImgSearchResults.size());
	unsigned numThreads=sz<m_uNumThreads?sz:m_uNumThreads;

	// create the threads we need
	while(m_Loaders.size()<numThreads){
		urlThumbLoader* p=new urlThumbLoader;
		m_Loaders.append(p);
		thumbnail* pt=new thumbnail;
		if(bYahooURLs)
			pt->setFromYahoo();
		else
			pt->setFromFlickr();
		m_Thumbs.append(pt);
	}

	assert(m_Thumbs.size()==m_Loaders.size());
	assert(m_Loaders.size()==numThreads);

	// note we connect the loaded() signal with the repaint request of the worker
	// if loading is really fast, we may block the app by flooding it with paint requests...
	// note if we search for similars, make sure then dynamicSearcher does not issue it's callbacks, too
	if(m_bSearchSimilar)
		m_pWorker->m_pImageSortWidget->m_DynamicSearcher.registerSearchAdvancedCallback(NULL,NULL,0,false);

	connect(this,SIGNAL(loaded()),m_pWorker->m_pImageSortWidget,SLOT(repaintRequested()),Qt::QueuedConnection);
	connect(this,SIGNAL(updateStatusBar()),m_pWorker->m_pImageSortWidget,SLOT(updateStatusBarRequested()),Qt::QueuedConnection);
	connect(this,SIGNAL(updateStatusBar2(unsigned,unsigned)),m_pWorker->m_pImageSortWidget,SLOT(updateStatusBar2Requested(unsigned,unsigned)),Qt::QueuedConnection);

	// set the id's and the first urls, then start the threads
	for(unsigned i=0;i<numThreads;++i){
		m_Loaders[i]->setId(i);
		m_Loaders[i]->setSize(m_pWorker->m_pImageSortWidget->thumbSize());
		if(bYahooURLs)
			m_Thumbs[i]->m_YahooURLs=m_pWorker->m_YahooImgSearchV1Results.dequeue();
		else
			m_Thumbs[i]->m_flickrURLs=m_pWorker->m_flickrImgSearchResults.dequeue();
		connect(m_Loaders[i],SIGNAL(finished()),this,SLOT(childFinished()),Qt::QueuedConnection);
		m_Loaders[i]->setRunArgs(m_Thumbs[i]);
		m_Loaders[i]->start();
	}

	// run the event loop
	exec();

}

void urlThumbLoadDispatcher::childFinished()
{
	// mutual exclusive...
	m_Mutex.lock();

	++m_uFilesLoaded;

	bool bYahooURLs=m_pWorker->m_bYahooImgSearch;

	// check (only once) if to abort
	if(!m_bAbort){
		m_pWorker->lock();
		m_bAbort=m_pWorker->m_bAbort;
		m_pWorker->unlock();
	}

	// note the elegant way to abort here:
	if(m_bAbort){
		if(bYahooURLs)
			m_pWorker->m_YahooImgSearchV1Results.clear();
		else
			m_pWorker->m_flickrImgSearchResults.clear();

	}

	bool bExit=false;
	if(bYahooURLs){
		if(m_pWorker->m_YahooImgSearchV1Results.isEmpty())
			bExit=true;
	}
	else
		if(m_pWorker->m_flickrImgSearchResults.isEmpty())
			bExit=true;

	// get the first loader which is finished
	// note when bExit==true, we need to keep track of the IDs, otherwise we might get the same ID up to
	// "number of threads" times...
	unsigned i,id;
	bool bFound=false;
	bool bActive=false;
	for(i=0;i<m_Loaders.size();++i){
		if(m_Loaders[i]->isFinished()){
			if(!bFound){
				if(!bExit){
					bFound=true;
					id=i;
				}
				else if(!m_ExitIDs.contains(i)){
					m_ExitIDs.insert(i);
					bFound=true;
					id=i;
				}
			}
		}
		else
			bActive=true;
	}
	const bool bHttpError=m_Loaders[id]->hasHttpError();
	const bool bImageLoadError=m_Loaders[id]->hasImageLoadError();
	if(!bHttpError&&!bImageLoadError){
		// V5 create a *new* thumbnail here and wrap it in a refPtr
		// NOTE: do not use the thumbnail* in m_Thumbs, these will be re-used by the next loader thread !!!
		thumbnail* pt= new thumbnail(*m_Thumbs[id]);
		controllerImage *pi=(controllerImage*)pt;
		refPtr<controllerImage> rfp(pi);

		if(!m_bSearchSimilar){
			// lock the widget
			m_pWorker->m_pImageSortWidget->lock();
			//m_pWorker->m_pImageSortWidget->m_SortItems.append(*m_Thumbs[id]);
			//thumbnail *p=&(m_pWorker->m_pImageSortWidget->m_SortItems.last());
			//m_pWorker->m_pImageSortWidget->m_SortedByName.append(p);
			m_pWorker->m_pImageSortWidget->m_ControllerImages.push_back(rfp);
			//m_pWorker->m_pImageSortWidget->m_SortedByName.push_back(rfp);
			m_pWorker->m_pImageSortWidget->unlock();
			if(!(m_uFilesLoaded%m_uNumRepaints)){
				emit loaded();
				emit updateStatusBar();
			}
		}
		else{
			//	m_pWorker->m_pImageSortWidget->lock();
			// locks:
			m_pWorker->m_pImageSortWidget->m_DynamicSearcher.dynamicSearch(rfp);
			if(!(m_uFilesLoaded%m_uNumRepaints)){
				emit loaded();
				emit updateStatusBar2(0,m_uFilesLoaded);
			}
			//m_pWorker->m_pImageSortWidget->unlock();

		}
	}
	else{
		// we have a problem here:
		// we download thumbs from many different urls
		// if one fails, the other may work
		// thus, there is no reason to stop here
		// we simply gather the errors and continue...
		if(bYahooURLs){
			if(bHttpError)
				m_HttpErrorStrings.insert(m_Thumbs[id]->m_YahooURLs.m_ThumbnailUrl,m_Loaders[id]->httpErrorString());
			else
				m_HttpErrorStrings.insert(m_Thumbs[id]->m_YahooURLs.m_ThumbnailUrl,tr("JPEG load failed"));
		}
		else{
			if(bHttpError)
				m_HttpErrorStrings.insert(m_Thumbs[id]->m_flickrURLs.thumbnailURL(),m_Loaders[id]->httpErrorString());
			else
				m_HttpErrorStrings.insert(m_Thumbs[id]->m_flickrURLs.thumbnailURL(),tr("JPEG load failed"));
		}
	}
	// start the loader again, if there are any more urls...
	if(!bExit){
		if(bYahooURLs){
			m_Thumbs[id]->m_YahooURLs=m_pWorker->m_YahooImgSearchV1Results.dequeue();
			m_Loaders[id]->setRunArgs(m_Thumbs[id]);
			m_Loaders[id]->start();
		}
		else{
			m_Thumbs[id]->m_flickrURLs=m_pWorker->m_flickrImgSearchResults.dequeue();
			m_Loaders[id]->setRunArgs(m_Thumbs[id]);
			m_Loaders[id]->start();
		}
	}
	m_Mutex.unlock();
	if(bExit&&!bActive)
		exit(0);
}
