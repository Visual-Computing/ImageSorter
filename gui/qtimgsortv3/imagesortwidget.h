#pragma once

#include <QtGui>
#include "thumbnail.h"
#include "imagesortworker.h"
#include "issearchwidget.h"
#include "isextensionwidget.h"
#include "pxdefs.h"
#include "pximage.h"
#include "pxsorter.h"
#include "pxsearcher.h"
#include "pxguicontroller.h"
#include "ismutex.h"
#include <algorithm>
using namespace isortisearch;
/*
*	Geändert von Claudius Brämer und David Piegza
*
*   imageSortWidget wird von QFrame abgeleitet, nötig für
*   StyleSheet Angaben
*/
class imageSortWidget : public QFrame {

	Q_OBJECT

	public:
		/*!
		the supported sort modes
		*/
		enum sortMode{
			BY_NAME=0,
			BY_DATE,
			BY_SIZE,
			BY_COLOR,
			BY_SIMILARITY_LIST,
			BY_SIMILARITY_PYRAMID,
			BY_DISK_LOAD,
			BY_NET_LOAD
		};

		/*!
		the states (of the worker)
		*/
		enum state{
			IDLE=0,
			CHANGING_DIRS,
			LOADING,
			SORTING,
			DIRSDIRTY,
			HARDDISK_SEARCH,
			// V4.2
			PREPARE_HARDDISK_SEARCH
		};

		/*!
		the inet search services
		*/
		enum InetSearchService{
			YAHOO=0,
			FLICKR=1
		};

		/*!
		ctor
		*/
		imageSortWidget(QFrame *p=0, Qt::WFlags f=0);
		/*!
		dtor
		*/
		~imageSortWidget();

		/*!
		returns the current bg color
		
		note the bg color is also used for the image viewer opened by double click
		*/
		const QColor& getBackgroundColor() const {return bgColor;}
		/*!
		resets the widget

		TODO: this may be a slot...
		*/
		void reset();
		/*!
		/*!
		returns the number of map places in x/y direction
		*/
		// unsigned mapPlacesX() const {return m_Sorter.mapPlacesX();}
		/*!
		returns the number of map places in x/y direction
		*/
		// unsigned mapPlacesY() const {return m_Sorter.mapPlacesY();}
		/*!
		returns the current sort mode
		*/
		sortMode getSortMode() const {return m_SortMode;}
		/*!
		returns the (bigger side) of the thumbnails loaded (by the worker)

		aspect ratio is kept

		V5: needed?
		*/
		unsigned thumbSize() const {return m_uThumbSize;}
		/*!
		returns the number of thumbs expeted to be loaded 

		note this should always be bigger or equal to numThumbs()

		note new: since we have placeholders for images which failed to load now,
		this is equal to numThumbs() after loading. anyway, during loading numThumbs()
		is smaller and grows towards numThumbsExpected()...
		*/
		unsigned numThumbsExpected() const {return m_uNumThumbsExpected;}
		/*!
		returns the number of thumbs currently loaded 

		note this grows while loading, thus this locks
		*/
		// V5: note prior versions had a mutex in te widget *and* a mutex in the worker.
		// thus the app was de facto not locking at all :()
		unsigned numThumbs() {QMutexLocker(&(m_Mutex.rmutex()));return m_ControllerImages.size();}
		/*!
		returns true if any thumbnail images loaded

		note this locks, cause it calls numThumbs()
		*/
		bool hasImages() {return numThumbs()!=0;}
		/*!
		returns the name filter, i.e. the image files we can load
		*/
		const QStringList& nameFilter()  const {return m_Worker.nameFilter();}
		/*!
		true if we have a color sort
		*/
		bool hasColorSort() const {return m_bHasColorSort;}
		/*!
		true if worker thread is loading

		note this costs a lock()/unlock()
		*/
		bool isLoading() {return m_Worker.isLoading();}
		/*!
		true if worker thread is sorting

		note this costs a lock()/unlock()
		*/
		bool isSorting() {return m_Worker.isSorting();}
		/*!
		the current state
		*/
		state getState() const {return m_State;}
		/*!
		a convenience abbreviation
		*/
		bool isIdle() const {return m_State==IDLE;}
		/*!
		a convenience abbreviation
		*/
		// V4.2: new state...
		bool isPreparingHardDiskSearch()const {return m_State==PREPARE_HARDDISK_SEARCH;}		
		/*!
		a convenience abbreviation
		*/
		bool isScanning() const {return m_State==CHANGING_DIRS;}
		/*!
		a convenience abbreviation
		*/
		bool hasDirtyDirs() const {return m_State==DIRSDIRTY;}
		/*!
		return true if the worker ha any dir
		*/
		bool hasDir() const {return !m_Worker.cdirs().empty();}
		/*
		a string describing the current state

		this can be polled in response to the busy() signal
		
		V4.2: no longer used
		*/
		//QString stateString() const {return m_StateString;}
		/*!
		returns all the dirs loaded
		*/
		QList<QString> dirsLoaded() const {return m_Worker.cdirs().cleanDirs();}
		/*!
		returns the top dirs loaded
		*/
		QList<QString> topDirsLoaded() const {return m_Worker.cdirs().topDirs();}
		/*!
		returns the the sym link name of a loaded dir (or it's name if the dir wasn't
		added by a sym link name)
		*/
		QString symLinkName(const QString& dir) const {return m_Worker.cdirs().symLinkName(dir);}
		/*!
		returns true if we have any dir loaded by a sym link
		*/
		bool hasSymLinkDirs() const {return m_Worker.cdirs().hasCleanSymLinks();}
		/*!
		return the current search query
		*/
		const QString& query() const {return m_Worker.query();}
		/*!
		return the current number of queries
		*/
		unsigned numQueries() const {return m_Worker.numQueries();}
		/*!
		return the current number of similar queries
		*/
		unsigned numSimilarQueries() const {return m_Worker.numSimilarQueries();}
		/*!
		return the current number of similar queries to be shown
		*/
		unsigned numSimilarShown() const {return m_Worker.numSimilarShown();}
		/*!
		return the current number of net search threads
		*/
		unsigned numNetSearchThreads() const {return m_Worker.numNetSearchThreads();}
		
		/*!
		returns true if we have a selelection
		*/
		bool hasSelection() const {return m_uNumSelected!=0;}
		/*!
		returns true if the query is not empty
		*/
		bool hasQuery() const {return !m_Worker.query().isEmpty();}
		/*!
		returns true if the current sort displayed is from the net
		*/
		bool isNetSort() const {return m_bIsNetSort;}
		/*!
		returns true if yahoo search
		*/
		bool isYahooSearch() const {return m_inetSearchService==YAHOO;}
		/*!
		returns true if flickr search
		*/
		bool isFlickrSearch() const {return m_inetSearchService==FLICKR;}
		/*!
		returns the search service as an int
		*/
		int inetSearchService() const {return (int)m_inetSearchService;}
		/*!
		return true if valid name sort available
		*/
		bool canDoNameSort() const {return !isNetSort()||isYahooSearch();}
		/*!
		return true if valid size sort available
		*/
		bool canDoSizeSort() const {return !isNetSort()||isYahooSearch();}
		/*!
		return true if valid date sort available
		*/
		bool canDoDateSort() const {return !isNetSort();}
		/*!
		return true if valid similarity sort available
		*/
		bool hasSimilaritySort() const {return m_bIsSimilarSort;}


		void searchDisk(const searchOptions& options);
		//void enableMsgWidgets();


public slots:
		void setQuery(const QString&);
		// called when the 'get from internet' btn clicked
		// V5: needed?
		void download();

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Methode für Bildsuche mit Suchoptionen
		*/
		void download(searchOptions&);

		// called when the 'get similar from net' btn clicked
		// void downloadSimilars();
		// called when the 'search similar on disk' btn clicked
		// void searchSimilars();

		/*!
		adds the dir with the passed absolute pathname to the list
		of dirs to be sorted

		if the dir does not exist, nothing happens
		*/
		void changeDirs(const QSet<QString>& dirsToRemove,const QSet<QString>& dirsToAdd);
		void dirsChanged();
		void dirsChangeAborted();
		/*!
		loads all directories added by addDir()

		if bRefresh=true, the dirs are loaded even if the dir set hasn't changed...

		return false if the workes dirs are not dirty, otherwise true (wether or not the dirs are empty...)
		*/
		bool load(bool disksearch=false);
		/*!
		aborts loading if currently loading

		aborts sorting if currently sorting

		otherwise NOP
		*/
		void abortLoadOrSort();
		/*!
		called when ESC pressed
		*/
		void escPressed();
		/*!
		resets the visualization

		this calls resetZoom() and enableSphere(false)

		if bForceRedraw=true, the widget is redrawn

		TODO: check if to merge with reset()
		*/
		void resetVisualization(bool bRedraw=true);
		/*!
		zooms in
		*/
		void zoomIn();
		/*!
		zooms out
		*/
		void zoomOut();
		/*!
		resets zoom

		if bForceRedraw=true, the widget is redrawn
		*/
		void resetZoom(bool bForceRedraw=true);
		/*!
		sets the background color
		*/
		void setBackgroundColor(const QColor& color) {bgColor = color;}
		/*!
		sets the sort mode

		if bForceRedraw=true, the widget is redrawn

		if mode is different from getSortMode(), the mode is set 
		and sortModeChanged() is emitted.
		*/
		void setSortMode(sortMode mode,bool bForceRedraw=true);

		/*!
		enables/disables preview on Mouse Over
		*/
		void enablePreviewOnMouseOver(bool bEnable);

		/*!
		enable/disables automatic resort on hide, move or delete
		*/
		void enableResortOnMoveDelete(bool bEnable);

		//V3BETA2
		/*!
		enables/disables recursive load
		*/
		void enableRecursiveLoad(bool bEnable) {m_bEnableRecursiveLoad=bEnable;}
		bool recursiveLoadEnabled() const {return m_bEnableRecursiveLoad;}

		void enableAutoLoad(bool b) {m_bDoAutoLoad=b;}

		void setSearchOptions(const QString& query,unsigned nQueries,unsigned nSimilarQueries,unsigned nShowSimilars,unsigned numNetSearchThreads);
		void setDimFactor(double factor) {m_Sorter.setDimFactor(factor);}
		double dimFactor() const {return m_Sorter.dimFactor();}
		// V3 BETA 3
		void setInetSearchService(int i);

		void disableWarnings(bool b);
private slots:

		void loadAndSortBtnClicked();
		void updateProgressBar(unsigned int);
		void hideMsgWidgets();

		// worker slots
		void loadingStarted();
		void loadCompleted();
		void loadAborted();
		void loadSimilarCompleted();
		void loadSimilarAborted();
		void inetLoadFailed(QString);
		void inetLoadCompleted();
		void inetLoadAborted();
		void inetLoadSimilarCompleted();
		void inetLoadSimilarAborted();

		void colorSortStarted();
		void colorSortCompleted();
		void colorSortAborted();

		// called when the worker has loaded a 'line'
		void repaintRequested();
		// called when the worker has a new color sort
		void newColorSort();

		// called when the worker has loaded a line or a dir
		void updateStatusBarRequested();
		// called during similarity search on disk
		void updateStatusBar2Requested(unsigned dirs,unsigned files);

		void resort();

		void dirCreatedByWorker(const QString& dir);

		void undoSelection();
		void openSelected();
		void moveSelected();
		void deleteSelected();
		void copySelected();
		void invertSelection();
		void selectAll();
		void copySelectedToClipboard();

		void splashWindowShown();

		void searchFolder();
		void searchDiskAborted();
		void stopSearchDisk();

signals:
		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Signal für Anzahl ausgewählter Ordner und Dateien für 
		*   Status-Widget-Anzeige
		*/		
		void setStatusWidget(const QString& strNumDirs, const QString& strNumFiles);
		void setNumSelected(const QString& strSelected);
		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Signal für Dateipfad des ausgewählten Bildes für Einzelbildansicht
		*/
		void startDrag();

		/*!
		emitted when the (worker) state has changed 

		may be used by the gui to call state() and/or stateString()
		and to display messages, show wait cursors and so on...
		*/
		void stateChanged();
		/*!
		emitted when thumbnail clicked
		*/
		void imageClicked(const thumbnail*);
		/*!
		emitted when image hovered
		*/
		void imageHovered(const thumbnail*);
		/*!
		emitted when the image set has to be refreshed for any reason,
		i.e. when the user has deleted selected images via the context menu
		*/
		//void refresh();
		/*!
		emitted when the sort mode is changed

		slots connected to this can call getSortMode() to get the mode
		*/
		void sortModeChanged();
		/*!
		emitted when selected images have been moved, deleted or copied
		*/
		void refreshFileSystem();
		void refreshFileSystemPath(const QString& path);
		/*!
		may be use to display wait cursor

		note this is for lengthy ops which are *not*
		in the worker, but in this thread...
		*/
		void startWorking();
		/*!
		may be use to hide wait cursor

		note this is for lengthy ops which are *not*
		in the worker, but in this thread...
		*/
		void stoppedWorking();
		/*!
		emitted when (the worker) created a new directory on the filesystem
		*/
		void dirCreated(const QString& dir);
		/*!
		emitted when change dirs aborted
		*/
		void lastLoadedDirs(QStringList dirs);
		/*!
		emitted when selection changed
		*/
		void selectionChanged();
		/*!
		emitted when an inet search is about to be done
		*/
		void deSelectDirs();
		/*!
		emitted when a thumbnail has been downloaded from the net *or* 
		is inserted in the similarity container
		*/
		void showProgress(unsigned int num);
protected:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void paintEvent(QPaintEvent *event);
		void mouseDoubleClickEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);
		virtual void contextMenuEvent(QContextMenuEvent *p);
		virtual void keyPressEvent(QKeyEvent *pe);
		virtual void keyReleaseEvent(QKeyEvent *pe);

#ifndef QT_NO_WHEELEVENT
		/*!
			zoom in/out by the wheel mouse
		*/
		virtual void wheelEvent(QWheelEvent *pe);
#endif
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		/*
		stops dragging the thumbs by the left mouse button or changing
		the selection rect if shift or ctrl pressed
		may emit selectionRectChanged() or itemClicked()
		*/
		virtual void mouseReleaseEvent(QMouseEvent *event);
		// overridden methods
		QSize minimumSizeHint() const;

		// binary predicates for the name, date and size sort
		// TODO: these should be in the worker...
		static bool smallerName(refPtr<controllerImage> p1,refPtr<controllerImage> p2) {return QString::localeAwareCompare(((thumbnail&)(*p1)).nameToSort(),((thumbnail&)(*p2)).nameToSort())<0;}
		static bool smallerDate(refPtr<controllerImage> p1,refPtr<controllerImage> p2) {return ((thumbnail&)(*p1)).m_LastModified<((thumbnail&)(*p2)).m_LastModified;}
		static bool biggerSize(refPtr<controllerImage> p1,refPtr<controllerImage> p2) {return ((thumbnail&)(*p1)).sizeToSort()>((thumbnail&)(*p2)).sizeToSort();}
		// name, date and size sorts
		void sortByName() {std::sort(m_SortedByName.begin(),m_SortedByName.end(),&imageSortWidget::smallerName);}
		void sortByDate() {std::sort(m_SortedByDate.begin(),m_SortedByDate.end(),&imageSortWidget::smallerDate);}
		void sortBySize() {std::sort(m_SortedBySize.begin(),m_SortedBySize.end(),&imageSortWidget::biggerSize);}


	private:
		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Variable für Button zum Starten des Festplatten-Ladevorgangs
		*/
		ISMsgWidget *m_pMsgWidget;
		ISMsgWidget *m_pMsgStart;
		QPushButton *loadImgButton;

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Variablen für Anzahl ausgewählter Ordnder und Dateien
		*/		
		QString strNumDirs;
		QString strNumFiles;
		QString strSelected;

		// delivers the thumbnail at the given position or NULL if none
		thumbnail* getPictureAt(const QPoint &pos);
		
		// true by ctor, false after timer in repaint() down
		bool m_bShowSplash;
		// set by load()...

		sortMode m_SortMode;

		// a context menu
		QMenu m_ContextMenu;
		// it's actions
		QAction* m_pResortAction;
		QAction* m_pOpenSelected;
		QAction* m_pCopySelectedToClipboard;
		QAction* m_pDeleteSelected;
		QAction* m_pMoveSelected;
		QAction* m_pCopySelected;
		QAction* m_pSelectAll;
		QAction* m_pUndoSelection;
		QAction* m_pInvertSelection;
		void createContextMenu();

		state m_State;
		void setState(state s);
		// V4.2: no longer used
		// QString m_StateString;
		// a mutex to lock() and unlock()
		ISMutex m_Mutex;
		// locks
		void lock() {m_Mutex.lock();}
		// unlocks
		void unlock(){m_Mutex.unlock();}

		// a mutex and a wait condition for waiting
		QMutex m_WaitMutex;
		QWaitCondition m_WaitCondition;

		/*
		sorts by color
		*/
		void sortByColor();

		/*
		the search and ui classes
		*/
		guiController<controllerImage> m_GuiController;
		sorter<controllerImage> m_Sorter;
		dynamicSearcher<controllerImage> m_DynamicSearcher;

		// static callbacks the controller calls
		static void repaintRequestedStaticCB(void *pUserData);
		// mebers called by the statics above
		void repaintRequestedBySortController(void);


		// the number of thumbs ecxpected to be loaded
		// this is used for kai's new draw code for non color sort modes
		// as well as for the state string when loading
		unsigned m_uNumThumbsExpected;

		// tells if we have a valid color sort
		// either form loading from the thumbs database or from sortByColor()
		bool m_bHasColorSort;

		// true when mouse over enabled
		// when false, image are loaded into the preview by clicking
		// enabling this may slow down...
		bool m_bPreviewOnMouseOver;

		bool m_bResortOnMoveDelete;

		//V3BETA2
		bool m_bEnableRecursiveLoad;

		// this is the *bigger* side of the thumbnails 
		// loaded by the worker
		// aspect ratio is kept
		unsigned m_uThumbSize;
	
		// the worker thread
		imageSortWorker m_Worker;
		// ...is our friend...
		friend class imageSortWorker;

		// does all internal allocations
		// new: called by the worker...
		void allocate(unsigned numItems);
		// ...
		void deallocate();

		// TODOV3BETA2:
		// we may use a QSet<thumbnail*> m_SelectedPtrs for the selected thumbs
		// the QSet guarantees that an element is only included once
		// it is quite fast
		// currently, we always loop over the complete m_SortItems linked list
		// and check if the thumbnail is selected, and we have to keep track
		// of m_uNumSelected, which would be just m_SelectedPtrs.size()...
		// however, we have to make sure that m_SortItems stay in memory at same positions then...

		// containers to support sort by name, date and size
		// note these sorts are based on qSort() (equivalent to stl::sort())
		// which could also work on the m_SortItems container, too
		// anyway, it's faster to sort these array, and we can keep
		// the sorts after we have done the sorting...
		// however, this consumes more memory...
		// note also that we need m_SortedByName to draw while loading
		// we could also draw from m_SortItems...
#pragma message("use one vector only...")
		// TODOV5: check if we really need these
		// i think we can just use one vector, because the sortBy() functions are really fast
		// just resort in case we want to draw another mode
		// at least, we can throw out the m_ControllerImages vector
		std::vector<refPtr<controllerImage> > m_SortedByName;
		std::vector<refPtr<controllerImage> > m_SortedByDate;
		std::vector<refPtr<controllerImage> > m_SortedBySize;

		// these are the images set to the sorter and the searcher
		std::vector<refPtr<controllerImage> > m_ControllerImages;

		// this copies the contents of m_DownLoadItems to m_SortByName and m_SortItems
		// this has to be done before repaint() and has to be locked, otherwise we get crashes
		void copyDownloadedSimilarItems();

		// background color
		QColor bgColor;

		// splash image
		QImage *m_pSplashImage;

		// the draw parameters
		double m_dZoomFactor;
		
		// V5: this was to shift the sort view if there was a selected image in one of the other views
		//int m_xStartPreview;
		//int m_yStartPreview;

		// the mouse position when left button pressed
		QPoint m_MousePressedLeftPos;

		// the modifier keys when left mouse button is pressed
		// reset by mouseReleaseEvent()
		Qt::KeyboardModifiers m_LeftMouseButtonKeys;
		void resetLeftMouseButtonKeys() {m_LeftMouseButtonKeys&=0;}
		bool leftMouseButtonShift() {return m_LeftMouseButtonKeys.testFlag(Qt::ShiftModifier);}
		bool leftMouseButtonAlt() {return m_LeftMouseButtonKeys.testFlag(Qt::AltModifier);}

		// the current mouse position
		//int m_MousePosX;
		//int m_MousePosY;

		// the widget-relative current mouse coors when the left button is pressed
		// valid until left button released, otherwise -1
		// note this is updated constantly in mouseMoveEvent(), thus do not assume
		// this is the coor where the button was pressed initially
		//int m_MouseLastX;
		//int m_MouseLastY;

		// the mouse move since the left button was pressed
		// valid until left button released, otherwise 0
		// 0 when left button released (but repaint() called before...)
		//double m_dMouseX;
		//double m_dMouseY;

		// V5: no navigation window
		// indicates the the map is dragged in fast mode (in the navigation window)
		// bool m_isFastMoving;
		// bool m_isSlowMoving;
		// indicates if the navigation window is visible
		// bool m_isNavigationInfoDisplayed;

		// the start and end point of the selection
		// used when mouse left button clicked and moved
		// while shift or ctrl are pressed
		// see selectionChanged()
		QRect m_SelectionRect;

		// methods		
		
		void paintScanningScreen();
		void paintLoadingScreen();
		void paintNoImageScreen();
		void paintDirsDirtyScreen();
		void paintStartUpScreen();
		void paintMessage(QString message);

		// called by paintEvent() to implement different draw codes
		// for color mode and non-color modes

		void paintImages();
		//void paintNonColorMode();
				
		/*
		called when selection changes
		
		the selection changes when the left mouse button is released
		while the shift or ctrl button is pressed and the selection rect is not empty

		bSelectMode is false, if ctrl was pressed and true, if shift was pressed
		*/
		void selectionRectChanged(const QRect& r,bool b);
		unsigned m_uNumSelected;
		unsigned numSelected() const {return m_uNumSelected;}

		void selectPreSelected();
		void undoPreSelection();
		void deSelectPreDeSelected();
		void undoPreDeSelection();

		void cleanupMovedOrDeleted();
		unsigned m_nMovedOrDeleted;
		bool hasMovedOrDeleted() const {return m_nMovedOrDeleted;}

		bool m_bNeedsResort;
		bool needsResort() const {return m_bNeedsResort;}

		// a helper for all the loadCompleted() slots...
		// if bShowColorSort is true, the color asort is shown (and probaply done...)
		// if bShowSimilarSort is true (and bShowColorSort is false !), the similarity sort is shown
		// otherwise the current sortmode stays...
		void _loadCompleted(bool bShowColorSort,bool bShowSimilarSort);

		bool m_bDoAutoLoad;
		// a friend of me:
		friend class urlThumbLoadDispatcher;

		bool m_bIsNetSort;
		bool m_bIsSimilarSort;

		// delivers a unique filename for fileName, if fileName exists
		// eg: copy_of_fileName.jpg if fileName.jpg exists in QDir...
		// the leading part (copy_of) will be translated if a translation module is loaded
		bool uniqueFileName(QString *pDstStr,const QFileInfo& fInfo,const QDir& dir);

		InetSearchService m_inetSearchService;

		bool m_bNoWarnings;
		// bool m_bKeyPressed;

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Variable für Drag & Drop Funktionalität
		*/
		bool m_bDragEnabled;
		bool m_bScrolling;
		//ISProgressBar *m_progressBar;

		
		bool m_bInetLoad;
		bool m_bInetLoadSimilar;
		bool m_bHardDiskSearch;
		bool m_bHarddiskLoadSimilar;

		bool m_bNoRepaint;
};
