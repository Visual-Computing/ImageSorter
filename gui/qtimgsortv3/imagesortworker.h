#pragma once
#include "imagesortdirs.h"
#include "imagesortresult.h"
#include "thumbnail.h"
#include "imagesort.h"
#include "thumbscache.h"

// TODOV4: do not include a widget here...
#include "issearchwidget.h"

#include <QtCore>
class imageSortWidget;

// #define CACHEDOWNLOADS

class imageSortWorker : public QThread
{
	Q_OBJECT
public:
	enum MODE{
		NOMODE=0,
		SORT,
		LOAD_DIRS,
		LOAD_FROM_NET,
		LOAD_SIM_FROM_NET,
		LOAD_SIM_FROM_DISK,
		CHANGE_DIRS
		//,CLEAR_DIRS
	};
		
	imageSortWorker(QObject *parent=NULL);
	~imageSortWorker();
	/*!
	call once after ctor
	*/
	void init(imageSortWidget* p);
	/*!
	first removes removeDir, then adds addDir (with subdirectories if bSubDirs)
	*/
	void changeDirs(const QSet<QString>& dirsToRemove,const QSet<QString>& dirsToAdd,bool bSubDirs);
	///*!
	//clears all dirs
	//*/
	//void clearDirs();
	/*!
	loads the dirs()

	this calls run() which runs in the new thread

	load() runs in the caller thread
	*/
	void load(bool bSimilars=false);

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Methode zum Starten der Festplattensuche
	*/
	void search(bool bSimilars=false);

	/*!
	read/write access to the dirs to load
	*/
	imageSortDirs& dirs() {return m_Dirs;}
	/*!
	read access to the dirs to load
	*/
	const imageSortDirs& cdirs() const {return m_Dirs;}
	/*!
	sorts the sort items in main window

	this calls run() which runs in the new thread

	sort() runs in the caller thread
	*/
	void sort(unsigned cbInterval=5);
	/*!
	call this from the main thread to stop load() or sort()

	either loadAborted() or sortAborted() will be emitted

	note that save() cannot be aborted
	*/
	void abort();
	/*!
	same as abort(), but no signal will not be emitted
	*/
	void abortSilently(bool bWait=false);
	
	/*
	Loads images from the net.
	
	called by the caller thread

	if bFromYahoo==FALSE, flickr is used...
	*/
	void loadFromNet(bool bLoadSimilars=false,bool bFromYahoo=true);
	
	/*!
	call to stop gracefully
	*/
	void stop();
	/*!
	returns true if waiting (in run())
	*/
	bool isWaiting() {QMutexLocker(&(this->m_Mutex));return m_bWaiting;}
	/*!
	returns true if not waiting and loading (in run())
	*/
	bool isLoading() {QMutexLocker(&(this->m_Mutex));return !m_bWaiting&&loadMode();}
	/*!
	returns true if not waiting and sorting (in run())
	*/
	bool isSorting() {QMutexLocker(&(this->m_Mutex));return !m_bWaiting&&sortMode();}
	/*!
	returns true if not waiting and scanning (directories) (in run())
	*/
	bool isScanning() {QMutexLocker(&(this->m_Mutex));return !m_bWaiting&&scanMode();}
	/*!
	tells if load mode

	this is true while !isWaiting() if load() was called and if isWaiting(),
	in this case it is the last operation done
	*/
	bool loadMode() {return (m_RunMode==LOAD_DIRS || m_RunMode==LOAD_FROM_NET || m_RunMode==LOAD_SIM_FROM_NET|| m_RunMode==LOAD_SIM_FROM_DISK);}
	/*!
	tells if sort mode

	this is true while !isWaiting() if sort() was called and if isWaiting(),
	in this case it is the last operation done
	*/
	bool sortMode() {return m_RunMode==SORT;}
	/*!
	tells if scan mode

	this is true while !isWaiting() if changeDirs() was called and if isWaiting(),
	in this case it is the last operation done
	*/
	bool scanMode() {return m_RunMode==CHANGE_DIRS;}
	/*!
	locks
	*/
	void lock() {m_Mutex.lock();}
	/*!
	unlocks
	*/
	void unlock() {m_Mutex.unlock();}
	/*!
	get the name filter, i.e. the image files we can load
	*/
	const QStringList& nameFilter() const {return m_Dirs.nameFilter();}
	/*!
	return the current search query
	*/
	const QString& query() const {return m_Query;}
	/*!
	return the current number of queries
	*/
	unsigned numQueries() const {return m_uNumQueries;}
	/*!
	return the current number of similar queries
	*/
	unsigned numSimilarQueries() const {return m_uNumSimilarQueries;}
	/*!
	return the current number of similar queries to be shown
	*/
	unsigned numSimilarShown() const {return m_uNumSimilarShown;}

	unsigned numNetSearchThreads() const {return m_uNumNetSearchThreads;}

	void setQuery(const QString query) {m_Query=query;}
	void setNumQueries(unsigned n) {m_uNumQueries=n;}
	void setNumSimilarQueries(unsigned n) {m_uNumSimilarQueries=n;}
	void setNumSimilarShown(unsigned n) {m_uNumSimilarShown=n;}
	void setNumNetSearchThreads(unsigned n) {m_uNumNetSearchThreads=n;}
	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzt die Suchoptionen im Worker
	*/
	void setSearchOptions(const searchOptions& options) {m_searchOptions=options;}
	void useSearchOptions(bool val) {m_bUseSearchOptions=val;}
	// TODOV4 return a const reference
	searchOptions getSearchOptions() { return m_searchOptions;}

signals:
	/*!
	emit to let the widget repaint itself
	*/
	void repaintWidget();
	/*!
	emit to update the status bar while loading images from disk
	*/
	void updateStatusBar();
	/*!
	emit to update the status bar while searching for similar images on disk
	*/
	void updateStatusBar2(unsigned dirs,unsigned files);
	/*!
	emitted on start of loading
	*/
	void loadingStarted();
	/*!
	emitted when loading is aborted
	*/
	void loadAborted();
	/*!
	emitted when loading is completed
	*/
	void loadCompleted();
	/*!
	emitted when loading from internet failed
	*/
	void inetLoadFailed(QString errStr);
	/*!
	emitted when loading from internet is aborted
	*/
	void inetLoadAborted();
	/*!
	emitted when loading from net is completed
	*/
	void inetLoadCompleted();
	/*!
	emitted when loading similar (from disk) is aborted
	*/
	void loadSimilarAborted();
	/*!
	emitted when loading similar (from disk) is completed
	*/
	void loadSimilarCompleted();
	/*!
	emitted when loading similar from internet is aborted
	*/
	void inetLoadSimilarAborted();
	/*!
	emitted when loading similar from internet is completed
	*/
	void inetLoadSimilarCompleted();
	/*!
	emitted when sorting starts
	*/
	void sortingStarted();
	/*!
	emitted when a new sort map is available
	*/
	void newSort();
	/*!
	emitted when sort aborted
	*/
	void sortAborted();
	/*!
	emitted when sort complete
	*/
	void sortCompleted();
	/*!
	emitted when a dir is created
	*/
	void dirCreated(const QString& dir);
	/*!
	emitted when dirs changed
	*/
	void dirsChanged();
	/*!
	emitted when dir changed
	*/
	void dirsChangeAborted();
	/*!
	emitted when images searched
	*/
	void inserted(unsigned int nums);
protected:
	virtual void run();
private:
	// run mode
	MODE m_RunMode;

	// called by run() based on m_RunMode
	void _sort();
	void _load_dirs();
	void _load_from_net();
	void _change_dirs();
	QSet<QString> m_DirsToRemove;
	QSet<QString> m_DirsToAdd;
	bool m_bSubDirs;

	// returns true if the thumbs database was clean
	// called by _load_dirs()
	// bool _load_dir(const QString& dir,const QFileInfoList& files,imageSortResult* p,bool bEmit);
	// V5: no chached sort
	bool _load_dir(const QString& dir,const QFileInfoList& files);


	// loads and scales a jpeg bypassing the slow QImageReader
	// returns true on success
	bool _load_jpeg(const QString& str,thumbnail* p);

	// loads the exdif thumbnail from a jpeg
	bool _load_jpeg_exif_thumbnail(const QString& str,thumbnail* p);

	// loads and scales an image by the QImageReader
	// returns true on success
	bool _load_qimage(const QString& str,thumbnail* p);

	// the directories
	imageSortDirs m_Dirs;

	// used during _load_dir()
	thumbscache m_ThumbsCache;

	/*!
	a ptr to our caller

	note this is not our parent in the sense of QThread::QThread(QObject *pParent)
	*/
	imageSortWidget* m_pImageSortWidget;
	// we need to access the mutex from the widget...
	friend class imageSortWidget;

	// a static to be used as a callback for the som<T>
	// note this is ugly, but not our fault: Qt4 cannot handle signals in template classes,
	// otherwise we'd better use the signal/slot mechanism
	static void m_cbNewColorSort(void *pUserData) {((imageSortWorker*)pUserData)->cbNewSort();}
	// called by the static above
	void cbNewSort();
	unsigned m_cbInterval;

	// true if color sorting shall be aborted
	bool m_bAbort;
	// true: emit no signal on abort
	bool m_bAbortSilently;
	// flag to restart
	bool m_bRestart;
	// flag to destroy
	bool m_bDestroy;

	// flag signaling this is waiting (in run())
	bool m_bWaiting;

	// mutex to guard the flags
	QMutex m_Mutex;
	// wait condition to wake up
	QWaitCondition m_WaitCondition;

	// just to prohibit:
	imageSortWorker(const imageSortWorker&);
	imageSortWorker& operator=(const imageSortWorker&);

	// flag telling the main window is waiting for the silent abort
	bool m_bMainWaits;

	// an image as a placeholder for load failures
	QImage m_LoadFailureImage;

	// V3BETA1
	// we load inet images from yahoo image search v1 api
	QQueue<yahooImgSearchV1Result> m_YahooImgSearchV1Results;
	QQueue<flickrImgSearchResult> m_flickrImgSearchResults;

	// true: yahoo image search, false: flickr image search
	bool m_bYahooImgSearch;

	// an error string from QHttp in case of error...
	QString m_HttpErrorString;

	// a flag telling if we have any QHttp error...
	bool m_bHttpError;

	// if we search for similars and the query is the same, we can keep the last search...
	QString m_LastSearchQuery;

	// a set to keep the urls already downloaded
	// used to avoid downloads of duplicates 
	QSet<QString> m_DownLoadedURLs;

	// a bool flag which tells if we search similars 
	bool m_bLoadSimilars;

	// friends:
	friend class urlThumbLoadDispatcher;
	friend class yahooImgSearchV1QueryDispatcher;
	friend class flickrImgSearchQueryDispatcher;

	QString m_Query;
	unsigned m_uNumQueries;
	unsigned m_uNumSimilarQueries;
	unsigned m_uNumSimilarShown;
	unsigned m_uNumNetSearchThreads;

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Variable für die Suchoptionen
	*/
	searchOptions m_searchOptions;
	bool m_bUseSearchOptions;

	// V4 alpha handling
	public:
	void pasteOnCheckerBoard(QImage* img);

	// static callbacks the dynamic searcher calls
	static void repaintRequestedStaticCB(void *pUserData);
	// mebers called by the statics above
	void repaintRequestedByDynamicSearcher(void);
	// for reporting the number of files and dirs scanned during similarity search:
	unsigned m_uDirsScannedForSimilars;
	unsigned m_uFilesScannedForSimilars;
};
