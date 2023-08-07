#include "imagesortmainwindow.h"
#include "sortoptionsdialog.h"
// this was just for debugging, see imageSortMainWindow::~imageSortMainWindow()
//void QCntMutex::lock()
//{
//	QMutex::lock();
//	++m_LockCnt;
//}
//void QCntMutex::unlock()
//{
//	QMutex::unlock();
//	--m_LockCnt;
//}

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif

imageSortMainWindow::imageSortMainWindow() 
{
	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Fokus für das Hauptfenster setzen
	*/
	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	
	m_bSelectionChangedInternally=false;

	createDocks();
	createMainArea();
	createActions();
	createMenuBar();
	
	createToolBar();
	

	// Minimale Breite setzen
	// setMinimumWidth(900);


	init();
	loadSettings();

	setMouseTracking(true);

    setWindowTitle("ImageSorter v4.3 Beta");
	setWindowIcon(QIcon(":/imageSorterIcon"));
	m_bDelayChangeDirs=false;


}

imageSortMainWindow::~imageSortMainWindow()
{
	reset();
}

void imageSortMainWindow::loadSettings() 
{

	QSettings settings;

	settings.beginGroup("MainWindow");
	
	// load window position
	move(settings.value("windowPos",QPoint(100,100)).toPoint());
	// use reasonable default if last setting not found, i.e. on first use...
	if(!restoreGeometry(settings.value("geometry").toByteArray())){
		QSize size(800,600);
		resize(size);
	}
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();

	settings.beginGroup("ImageSorter");

	// load background color
	m_pImageSortWidget->setBackgroundColor(settings.value("bgColor",QColor(64,64,64)).value<QColor>());
	m_pPreviewCanvas->setBackgroundColor(settings.value("bgColor",QColor(64,64,64)).value<QColor>());
	m_pSearchWidget->setExImgBackgroundColor(settings.value("bgColor",QColor(64,64,64)).value<QColor>());

	// load tool tip state
	previewOnMouseOverAction->setChecked(settings.value("previewOnMouseOver",false).toBool());
	m_pImageSortWidget->enablePreviewOnMouseOver(previewOnMouseOverAction->isChecked());
	//highResPreviewAction->setChecked(settings.value("highResPreview",false).toBool());
	//m_pPreviewCanvas->enableHighResPreview(highResPreviewAction->isChecked());
	resortOnHideMoveDeleteAction->setChecked(settings.value("resortOnHideMoveDelete",false).toBool());
	m_pImageSortWidget->enableResortOnMoveDelete(resortOnHideMoveDeleteAction->isChecked());
	//V3BETA2
	loadRecursiveAction->setChecked(settings.value("loadRecursive",false).toBool());
	m_pImageSortWidget->enableRecursiveLoad(loadRecursiveAction->isChecked());
	// we don't load the sort criteria
	// this is an app to sort images by color similarity, thus this mode is the default always

	// we don't load the visualization (sphere/map mode)
	// since sorting by color is the default always, we always go back to map mode

	noWarnAct->setChecked(settings.value("Disable Warnings",false).toBool());
	m_pImageSortWidget->disableWarnings(noWarnAct->isChecked());

#if defined(Q_WS_MAC)
	showHiddenAct->setChecked(settings.value("Show Hidden Files",false).toBool());
	showHiddenFiles(showHiddenAct->isChecked());
#endif
	
	// load current folders
	m_bDelayChangeDirs=true;
	m_DirsToRemove.clear();
	m_DirsToAdd.clear();

	QString str1="folder";
	QString str2;
	unsigned n=settings.value("numFolders",0).toInt();
	QModelIndex index,parent;
	//bool bDone=false;
	
	//V3BETA2
	QSet<QModelIndex> indexes;
	for(unsigned i=0;i<n;++i){
		str2.setNum(i);
		index=m_Folders.index(settings.value(str1+str2,"").toString());
		if(index.isValid())
			indexes.insert(index);
	}
	// if we have loaded sub folders, we want to *expand* only the top folder
	// anyway, note we have to *select* also the sub folders (really?)
	QSet<QModelIndex>::iterator it=indexes.begin();
	QSet<QModelIndex>::const_iterator itEnd=indexes.constEnd();
	while(it!=itEnd){
		index=parent=*it;
		++it;
		while((parent=parent.parent()).isValid()){
			if(loadRecursiveAction->isChecked()){
				if(!indexes.contains(parent)){
					m_pFolderView->expand(parent);
					if(it==itEnd){
						m_bDelayChangeDirs=false;
						m_pImageSortWidget->enableAutoLoad(true);
					}
					m_pSelectionModel->select(index,QItemSelectionModel::Select);
				}
			}
			else{
				m_pFolderView->expand(parent);
				if(it==itEnd){
					m_bDelayChangeDirs=false;
					m_pImageSortWidget->enableAutoLoad(true);
				}
				m_pSelectionModel->select(index,QItemSelectionModel::Select);
			}
		}

		//if(it==itEnd)
		//	m_pImageSortWidget->load();
		//bDone=true;
	}

	//if(bDone)
	//	m_pImageSortWidget->load();

	settings.endGroup();

	settings.beginGroup("Search Options");
	unsigned numQueries=settings.value("number of queries",150).toUInt();
	unsigned numSimilarQueries=settings.value("number of similar queries",450).toUInt();
	unsigned numSimilarsShown=settings.value("number of similar shown",50).toUInt();
	unsigned numNetSearchThreads=settings.value("number of net search thread",20).toUInt();
	m_pImageSortWidget->setSearchOptions("",numQueries,numSimilarQueries,numSimilarsShown,numNetSearchThreads);

	//int j=settings.value("inet search service",0).toInt();
	//// Old search toolbar
	////inetSearchServiceDropList->setCurrentIndex(j);
	//// TODO: Set value in ISSearchWidget!
	//// TODO V4: really?
	//m_pImageSortWidget->setInetSearchService(j);

	settings.endGroup();
	
	settings.beginGroup("Sort Options");
	double dimFactor=settings.value("image spacing",1.2).toDouble();
	m_pImageSortWidget->setDimFactor(dimFactor);
	settings.endGroup();
}


void imageSortMainWindow::saveSettings() 
{
	QSettings settings;

	settings.beginGroup("MainWindow");

	//// save window size
	//settings.setValue("windowSize",size());

	// save window position	
	settings.setValue("windowPos",pos());
    settings.setValue("geometry",saveGeometry());
    settings.setValue("state",saveState());
	settings.endGroup();

	settings.beginGroup("ImageSorter");

	// save background color
	settings.setValue("bgColor",m_pImageSortWidget->getBackgroundColor());

	settings.setValue("previewOnMouseOver",previewOnMouseOverAction->isChecked());

	//settings.setValue("highResPreview",highResPreviewAction->isChecked());
	settings.setValue("resortOnHideMoveDelete",resortOnHideMoveDeleteAction->isChecked());

	//V3BETA2
	settings.setValue("loadRecursive",m_pImageSortWidget->recursiveLoadEnabled());

	settings.setValue("Disable Warnings",noWarnAct->isChecked());

#if defined(Q_WS_MAC)
	settings.setValue("Show Hidden Files",showHiddenAct->isChecked());
#endif
	
	// save current folders
	QString str1="folder";
	QString str2;
	QList<QString> folders;
	if(m_pImageSortWidget->recursiveLoadEnabled())
		folders=m_pImageSortWidget->topDirsLoaded();
	else
		folders=m_pImageSortWidget->dirsLoaded();

	unsigned n=folders.size();
	settings.setValue("numFolders",n);

	for(unsigned i=0;i<n;++i){
		str2.setNum(i);
		// V2.03: save the name of the sym link if the dir added by a sym link...
		settings.setValue(str1+str2,m_pImageSortWidget->symLinkName(folders[i]));
		// settings.setValue(str1+str2,folders[i]);
	}
	

	settings.endGroup();

	settings.beginGroup("Search Options");
	settings.setValue("number of queries",m_pImageSortWidget->numQueries());
	settings.setValue("number of similar queries",m_pImageSortWidget->numSimilarQueries());
	settings.setValue("number of similar shown",m_pImageSortWidget->numSimilarShown());
	settings.setValue("number of net search thread",m_pImageSortWidget->numNetSearchThreads());
	//settings.setValue("inet search service",m_pImageSortWidget->inetSearchService());
	settings.endGroup();

	settings.beginGroup("Sort Options");
	settings.setValue("image spacing",m_pImageSortWidget->dimFactor());
	settings.endGroup();
	
}
	

void imageSortMainWindow::init() 
{
	resetVisualization(false);
	restrictGui();
}

void imageSortMainWindow::createDocks() 
{
	explorerDock = new QDockWidget(tr("Explorer"), this);
	// neede for saveState()
	explorerDock->setObjectName("exporerdock");
	// V1.1: no longer movable
	explorerDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
	//explorerDock->setFeatures(QDockWidget::DockWidgetMovable); 
    //explorerDock->setAllowedAreas(Qt::AllDockWidgetAreas);

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzt eine mit CSS gestylten Überschriftsbereich für den Explorer
	*/
	QLabel *titleLabelExp = new QLabel("Explorer",explorerDock);
	titleLabelExp->setObjectName("explorerDockTitle");
	explorerDock->setTitleBarWidget(titleLabelExp); 

	previewDock = new QDockWidget(tr("Preview"), this);
	// needed for saveState()
	previewDock->setObjectName("previewDock");

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzt eine mit CSS gestylten Überschriftsbereich für den Vorschaubereich
	*/
	QLabel *titleLabelPrev = new QLabel("Preview",previewDock);
	titleLabelPrev	->setObjectName("previewDockTitle");
	previewDock->setTitleBarWidget(titleLabelPrev);


	// V1.1: no longer movable
	previewDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
	//previewDock->setFeatures(QDockWidget::DockWidgetMovable);
    //previewDock->setAllowedAreas(Qt::AllDockWidgetAreas);

	//imageInfoDock=new QDockWidget(tr("Image Info"),this);
	//imageInfoDock->setObjectName("imageinfodock");
	//imageInfoDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
	//imageInfoDock->setAllowedAreas(Qt::BottomDockWidgetArea);
	//imageInfoDock->setMaximumHeight(100);



	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Erstellt ein DockWidget für den Bildsuch-Bereich
	*/
	searchDock = new QDockWidget(tr("Image Search"), this);
	searchDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	searchDock->setObjectName("imageSearchDock");

	QLabel *titleSearchDock = new QLabel("Image Search",searchDock);
	
	titleSearchDock->setObjectName("searchDockTitle");
	
	searchDock->setTitleBarWidget(titleSearchDock); 

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Erstellt ein DockWidget für den Statusbereich
	*/
	statusDock = new QDockWidget(tr("Status"), this);
	statusDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	statusDock->setObjectName("statusDock");
	QLabel *titleStatusDock = new QLabel("Status",statusDock);
	titleStatusDock->setObjectName("statusDockTitle");
	statusDock->setTitleBarWidget(titleStatusDock); 

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Erstellt ein DockWidget für den Infobereich
	*/
	infoDock = new QDockWidget(tr("Info"), this);
	infoDock->setAutoFillBackground(true);
	infoDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
	infoDock->setObjectName("infoDock");
	QLabel *titleInfoDock = new QLabel("Info",infoDock);
	titleInfoDock->setObjectName("infoDockTitle");
	infoDock->setTitleBarWidget(titleInfoDock); 

	// Setzen einer festen Höhe für den Vorschaubereich und Infobereich
	infoDock->setFixedHeight(150);
	previewDock->setFixedHeight(200);

}


void imageSortMainWindow::createActions() 
{
	// exit application action
    exitAction = new QAction(tr("Exit"), this);
	exitAction->setToolTip(tr("Exit ImageSorter"));
    exitAction->setShortcut(tr("Ctrl+Q"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	// background color action
	bgColorAction = new QAction(tr("Background Color"), this);
	bgColorAction->setToolTip(tr("Change Background Color"));
	bgColorAction->setShortcut(tr("Ctrl+B"));
	bgColorAction->setIcon(QIcon(":/backgroundColorIcon"));
	connect(bgColorAction, SIGNAL(triggered()), this, SLOT(backgroundColorActionTriggered()));

	// sort by name action
	sortByNameAction = new QAction(tr("Sort By Name"), this);
	sortByNameAction->setToolTip(tr("Sort By Name"));
	sortByNameAction->setShortcut(tr("Ctrl+N"));
	sortByNameAction->setIcon(QIcon(":/sortByNameIcon"));
	sortByNameAction->setCheckable(true);
	sortByNameAction->setChecked(true);
	connect(sortByNameAction, SIGNAL(triggered()), this, SLOT(sortByActionTriggered()));

	// sort by size action
	sortBySizeAction = new QAction(tr("Sort By Size"), this);
	sortBySizeAction->setToolTip(tr("Sort By Size"));
	sortBySizeAction->setShortcut(tr("Ctrl+I"));
	sortBySizeAction->setIcon(QIcon(":/sortBySizeIcon"));
	sortBySizeAction->setCheckable(true);
	sortBySizeAction->setChecked(false);
	connect(sortBySizeAction, SIGNAL(triggered()), this, SLOT(sortByActionTriggered()));

	// sort by date action
	sortByDateAction = new QAction(tr("Sort By Date"), this);
	sortByDateAction->setToolTip(tr("Sort By Date"));
	sortByDateAction->setShortcut(tr("Ctrl+D"));
	sortByDateAction->setIcon(QIcon(":/sortByDateIcon"));
	sortByDateAction->setCheckable(true);
	sortByDateAction->setChecked(false);
	connect(sortByDateAction, SIGNAL(triggered()), this, SLOT(sortByActionTriggered()));

	// sort by color action
	sortByColorAction = new QAction(tr("Sort By Color"), this);
	sortByColorAction->setToolTip(tr("Sort By Color"));
	sortByColorAction->setShortcut(tr("Ctrl+O"));
	sortByColorAction->setIcon(QIcon(":/sortByColorIcon"));
	sortByColorAction->setCheckable(true);
	sortByColorAction->setChecked(false);
	connect(sortByColorAction, SIGNAL(triggered()), this, SLOT(sortByActionTriggered()));

	// sort by similarity action
	sortBySimilarityAction = new QAction(tr("Sort By Similarity"), this);
	sortBySimilarityAction->setToolTip(tr("Sort By Similarity"));
	//sortBySimilarityAction->setShortcut(tr("Ctrl+O"));
	sortBySimilarityAction->setIcon(QIcon(":/sortBySimilarityIcon"));
	sortBySimilarityAction->setCheckable(true);
	sortBySimilarityAction->setChecked(false);
	connect(sortBySimilarityAction, SIGNAL(triggered()), this, SLOT(sortByActionTriggered()));

	// reset Vizualisation action
	//resetVisualizationAction = new QAction(tr("&Reset Visualization"), this);
	/*
	resetVisualizationAction = new QAction(tr("Reset Visualization"), this);
	resetVisualizationAction->setToolTip(tr("Reset Visualization"));
	resetVisualizationAction->setShortcut(tr("Ctrl+R"));
	resetVisualizationAction->setIcon(QIcon(":/resetIcon"));
	resetVisualizationAction->setCheckable(false);
	connect(resetVisualizationAction, SIGNAL(triggered()), this, SLOT(resetVisualization()));
	*/

	// reset Zoom action
	//resetZoomAction = new QAction(tr("Reset &Zoom"), this);
	resetZoomAction = new QAction(tr("Reset Zoom"), this);
	resetZoomAction->setToolTip(tr("Reset Zoom"));
	resetZoomAction->setShortcut(tr("Ctrl+Z"));
	resetZoomAction->setIcon(QIcon(":/zoomOrigSizeIcon"));
	resetZoomAction->setCheckable(false);
	connect(resetZoomAction, SIGNAL(triggered()), m_pImageSortWidget, SLOT(resetZoom()));

	// zoom in action
	zoomInAction = new QAction(tr("Zoom In"), this);
	zoomInAction->setToolTip(tr("Zoom In"));
	zoomInAction->setShortcut(tr("+"));
	zoomInAction->setIcon(QIcon(":/zoomInIcon"));
	zoomInAction->setCheckable(false);
	connect(zoomInAction, SIGNAL(triggered()), m_pImageSortWidget, SLOT(zoomIn()));

	// zoom out action
	zoomOutAction = new QAction(tr("Zoom Out"), this);
	zoomOutAction->setToolTip(tr("Zoom Out"));
	zoomOutAction->setShortcut(tr("-"));
	zoomOutAction->setIcon(QIcon(":/zoomOutIcon"));
	zoomOutAction->setCheckable(false);
	connect(zoomOutAction, SIGNAL(triggered()), m_pImageSortWidget, SLOT(zoomOut()));

	// fullscreen action
	// TOFDO V4: rename this, this is NOT fullscreen mode...
	fullscreenAction = new QAction(tr("Fullscreen"), this);
	fullscreenAction->setToolTip(tr("Fullscreen"));
	fullscreenAction->setShortcut(tr("F11"));
	//fullscreenAction->setIcon(QPixmap("Resources/fullscreen.png"));
	fullscreenAction->setIcon(QIcon(":/fullscreen"));
	fullscreenAction->setCheckable(true);
	connect(fullscreenAction, SIGNAL(triggered()), SLOT(showMainArea()));

	previewOnMouseOverAction=new QAction(tr("Preview on Mouse Over"), this);
	previewOnMouseOverAction->setToolTip(tr("Preview on Mouse Over"));
	previewOnMouseOverAction->setCheckable(true);
	connect(previewOnMouseOverAction,SIGNAL(toggled(bool)),m_pImageSortWidget,SLOT(enablePreviewOnMouseOver(bool)));

	//V3BETA2
	loadRecursiveAction=new QAction(tr("Load Subdirectories"), this);
	loadRecursiveAction->setToolTip(tr("Load Subdirectories"));
	loadRecursiveAction->setCheckable(true);
	connect(loadRecursiveAction,SIGNAL(toggled(bool)),m_pImageSortWidget,SLOT(enableRecursiveLoad(bool)));

	//highResPreviewAction=new QAction(tr("High Quality Preview"), this);
	//highResPreviewAction->setToolTip(tr("High Quality Preview"));
	//highResPreviewAction->setCheckable(true);
	//connect(highResPreviewAction,SIGNAL(toggled(bool)),m_pPreviewCanvas,SLOT(enableHighResPreview(bool)));

	resortOnHideMoveDeleteAction=new QAction(tr("Resort on Move or Delete"), this);
	resortOnHideMoveDeleteAction->setToolTip(tr("Resort on Move or Delete"));
	resortOnHideMoveDeleteAction->setCheckable(true);
	connect(resortOnHideMoveDeleteAction,SIGNAL(toggled(bool)),m_pImageSortWidget,SLOT(enableResortOnMoveDelete(bool)));
		
	internetSetAct = new QAction(tr("Search Options"), this);
	internetSetAct->setToolTip(tr("Search Options"));
	internetSetAct->setCheckable(false);
	connect(internetSetAct, SIGNAL(triggered()), this, SLOT(setSearchOptions()));

	setSortOptionsAction = new QAction(tr("Sort Options"), this);
	setSortOptionsAction->setToolTip(tr("Sort Options"));
	setSortOptionsAction->setCheckable(false);
	connect(setSortOptionsAction, SIGNAL(triggered()), this, SLOT(setSortOptions()));

	noWarnAct = new QAction(tr("Disable all warnings"), this);
	noWarnAct->setToolTip(tr("Disable all warnings"));
	noWarnAct->setCheckable(true);
	connect(noWarnAct, SIGNAL(toggled(bool)), m_pImageSortWidget, SLOT(disableWarnings(bool)));

#if defined(Q_WS_MAC)	
	showHiddenAct = new QAction(tr("Show hidden files"), this);
	showHiddenAct->setToolTip(tr("Show hidden files"));
	showHiddenAct->setCheckable(true);
	connect(showHiddenAct, SIGNAL(toggled(bool)), this, SLOT(showHiddenFiles(bool)));
#endif
	
	// about action
    //aboutAct = new QAction(tr("&About"), this);
    aboutAct = new QAction(tr("About"), this);
	aboutAct->setToolTip(tr("About"));
	aboutAct->setCheckable(false);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

}


void imageSortMainWindow::createMenuBar() 
{
	m_menuBar = new QMenuBar(this);

	// create top menu items
	fileMenu = new QMenu(m_menuBar);
	fileMenu->setTitle(tr("File"));

	viewMenu = new QMenu(m_menuBar);
	viewMenu->setTitle(tr("View"));

	optionsMenu = new QMenu(m_menuBar);
	optionsMenu->setTitle(tr("Options"));

	helpMenu = new QMenu(m_menuBar);
	helpMenu->setTitle(tr("Info"));

    m_menuBar->addAction(fileMenu->menuAction());
    m_menuBar->addAction(viewMenu->menuAction());
    m_menuBar->addAction(optionsMenu->menuAction());
    m_menuBar->addAction(helpMenu->menuAction());
	
	// create sort criteria action group
	sortCriteriaGroup = new QActionGroup(this);
	sortCriteriaGroup->setExclusive(true);
	sortCriteriaGroup->addAction(sortByColorAction);
	sortCriteriaGroup->addAction(sortByNameAction);
	sortCriteriaGroup->addAction(sortBySizeAction);
	sortCriteriaGroup->addAction(sortByDateAction);
	sortCriteriaGroup->addAction(sortBySimilarityAction);
	
	// create visualization action group
	visualizationGroup = new QActionGroup(this);
	visualizationGroup->setExclusive(true);

	// add actions
	fileMenu->addAction(exitAction);

	viewMenu->addActions(sortCriteriaGroup->actions());
	viewMenu->addSeparator();
	viewMenu->addActions(visualizationGroup->actions());
	//viewMenu->addAction(resetVisualizationAction);
	viewMenu->addSeparator();
	viewMenu->addAction(zoomInAction);
	viewMenu->addAction(zoomOutAction);
	viewMenu->addAction(resetZoomAction);
	viewMenu->addAction(fullscreenAction);
	viewMenu->addSeparator();
	viewMenu->addAction(bgColorAction);

	optionsMenu->addAction(previewOnMouseOverAction);
	//optionsMenu->addAction(highResPreviewAction);
	optionsMenu->addAction(resortOnHideMoveDeleteAction);
	// V3BETA2
	optionsMenu->addAction(loadRecursiveAction);
	optionsMenu->addAction(internetSetAct);
	optionsMenu->addAction(setSortOptionsAction);
	// V3BETA3
	optionsMenu->addAction(noWarnAct);
#if defined(Q_WS_MAC)	
	// V4BETA2
	optionsMenu->addAction(showHiddenAct);
#endif
	

		
	//optionsMenu->addAction(autoColorSortAction);
	//optionsMenu->addAction(loadAction);

	//helpMenu->addAction(helpAction);
	helpMenu->addAction(aboutAct);

	// add menubar to window
	setMenuBar(m_menuBar);
}


/*
*	Geändert von Claudius Brämer und David Piegza
*
*   Ordnet die Toolbar im imageSortWidget vertikal an.
*   Bisherige Toolbarfunktionen bleiben erhalten
*/
void imageSortMainWindow::createToolBar() {
	toolBar = new QToolBar();
	toolBar->setObjectName("toolbar");

	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->addWidget(toolBar);
	m_hLayout->insertLayout(0,vLayout);

	toolBar->setFixedWidth(35);
	toolBar->setAutoFillBackground(true);
	toolBar->move(0,0);
	// setup toolbar
    toolBar->setMovable(false);
    toolBar->setOrientation(Qt::Vertical);
    // TODO V4: toolbar icaons are ugly... (V3 code:)
    // toolBar->setIconSize(QSize(18, 18));
    
	// add actions and menus to toolbar
	toolBar->addActions(sortCriteriaGroup->actions());
	toolBar->addSeparator();
	toolBar->addActions(visualizationGroup->actions());

	toolBar->addSeparator();
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(resetZoomAction);
	toolBar->addAction(fullscreenAction);
	toolBar->addSeparator();
	toolBar->addAction(bgColorAction);
	//toolBar->addAction(resetVisualizationAction);
	//toolBar->addSeparator();
	
	toolBar->setCursor(Qt::ArrowCursor);
	// Toolbar wird nicht dem Hauptfenster sondern dem
	// imageSortWidget hinzugefügt
	//addToolBar(Qt::LeftToolBarArea, toolBar); 
}


void imageSortMainWindow::createMainArea() {    
	QString backgroundStyle("background-color:#E1E1E1");

	// (pre)create the sort widget
	// we need this to get the name filter for the tree model below...

	QWidget *wrapper = new QWidget(this);

	m_hLayout = new QHBoxLayout();
	m_hLayout->setContentsMargins(0,0,0,0);
	m_hLayout->setSpacing(0);

	m_pImageSortWidget = new imageSortWidget();
	m_hLayout->addWidget(m_pImageSortWidget);

	wrapper->setLayout(m_hLayout);

	//m_pImageSortWidget = new imageSortWidget();

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzen des Objektnamens für StyleSheets
	*/
	m_pImageSortWidget->setObjectName("sortWidget");

	
	// create tree model
	m_Folders.setReadOnly(true);
	m_Folders.setResolveSymlinks(true);
	// V4.2: m_Folders is a QFilseSystemModel, no longer a QDirModel...
	//m_Folders.setLazyChildCount(true);
	//m_Folders.setSorting(QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
	//m_Folders.setNameFilters(m_NameFilter);
	m_Folders.setNameFilters(m_pImageSortWidget->nameFilter());
#if defined(Q_WS_MAC)
	// V4.2 show hidden files on OS X to show /VOLUMES
	// done in loadSettings()
	// showHiddenFiles(showHiddenAct->isChecked())
	//m_Folders.setRootPath("/");
	//m_Folders.setRootPath(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
	m_Folders.setRootPath(m_Folders.myComputer().toString());								
#else
	m_Folders.setFilter(QDir::AllDirs|QDir::Files|QDir::CaseSensitive|QDir::NoDotAndDotDot);
	m_Folders.setRootPath(m_Folders.myComputer().toString());
#endif

	// create tree view
	m_pFolderView = new ISTreeView(explorerDock);
#if defined(Q_WS_WIN)	
	m_pFolderView->setStyleSheet("QTreeView { " + backgroundStyle + "}");
#else
	// have a background when non-selected items are hovered...
	m_pFolderView->setStyleSheet("QTreeView::item:hover:!selected { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FAFBFE, stop: 1 #DCDCDC); }");
#endif	
	//m_pFolderView = new QTreeView(explorerDock);
	// new: allow multiple selection
	m_pFolderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pFolderView->setModel(&m_Folders);
	//m_pFolderView->setContextMenuPolicy(Qt::CustomContextMenu);
	//m_pFolderView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// hide header and columns

	m_pFolderView->header()->setHidden(true);
	m_pFolderView->setColumnHidden(1,true);
	m_pFolderView->setColumnHidden(2,true);
	m_pFolderView->setColumnHidden(3,true);
	
	// new: get the tree views selection model (pointer)
	m_pSelectionModel=m_pFolderView->selectionModel();

	// add signals
    connect(m_pFolderView, SIGNAL(fileClicked(const QFileInfo&)),
			this, SLOT(openInPreview(const QFileInfo&)));

	// V4.2 recalculate column 0 width if something collapsed/expanded to have horizontal scrollbar
    connect(m_pFolderView, SIGNAL(collapsed(const QModelIndex&)),
			m_pFolderView, SLOT(indexCollapsed(const QModelIndex&)));
	
    connect(m_pFolderView, SIGNAL(expanded(const QModelIndex&)),
			m_pFolderView, SLOT(indexExpanded(const QModelIndex&)));
	
	connect(m_pSelectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), 
		this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

	//connect(m_pFolderView, SIGNAL(selectionChangedSignal(const QItemSelection &, const QItemSelection &)), 
	//		this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

	// add explorer dock to the application window
	explorerDock->setWidget(m_pFolderView);
	addDockWidget(Qt::LeftDockWidgetArea, explorerDock);

	// create preview dock content
	m_pPreviewCanvas = new PreviewCanvas();
	m_pPreviewCanvas->setObjectName("canvasView");
	// add preview dock to the application window
	previewDock->setWidget(m_pPreviewCanvas);
	addDockWidget(Qt::LeftDockWidgetArea,previewDock);

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Erstellt die Widgets für Bildsuche, Status- und
	*   Bildinformation
	*/
	m_pSearchWidget = new ISSearchWidget();
#if defined(Q_WS_WIN)	
	m_pSearchWidget->setStyleSheet(backgroundStyle);
#endif
	//m_pSearchWidget->setBackgroundColor();

	m_pStatusWidget = new ISStatusWidget();
#if defined(Q_WS_WIN)	
	//m_pSearchWidget->setStyleSheet(backgroundStyle);
	m_pStatusWidget->setStyleSheet(backgroundStyle);
#endif
	
	m_pInfoWidget = new ISInfoWidget();
#if defined(Q_WS_WIN)	
	m_pInfoWidget->setStyleSheet(backgroundStyle);
	//m_pSearchWidget->setStyleSheet(backgroundStyle);
#endif
	


	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Widget werden dem DockWidget hinzugefügt
	*/
	searchDock->setWidget(m_pSearchWidget);
	addDockWidget( Qt::RightDockWidgetArea, searchDock );
	statusDock->setWidget(m_pStatusWidget);
	addDockWidget( Qt::RightDockWidgetArea, statusDock );
	infoDock->setWidget(m_pInfoWidget);
	addDockWidget( Qt::LeftDockWidgetArea, infoDock );

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzt den Objektnamen für StyleSheets
	*/
	m_pFolderView->setObjectName("folderView");
	m_pStatusWidget->setObjectName("statusView");
	m_pSearchWidget->setObjectName("searchView");
	m_pInfoWidget->setObjectName("infoView");



	/*QFrame *w = new QFrame(m_pImageSortWidget);

	QProgressBar *pBar = new QProgressBar();
	pBar->setMaximumHeight(16);
	pBar->setMinimumWidth(200);
	pBar->setMaximum(100);
	pBar->setMinimum(0);
	pBar->setValue(42);

	QPushButton *b = new QPushButton("Cancel");
	b->setMaximumWidth(75);



	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(pBar);
	vLayout->setMargin(25);

	vLayout->addWidget(new QLabel("<b>Loading image 258 of 614...</b>"));

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(b);

	vLayout->addSpacing(10);
	vLayout->addLayout(hLayout);
	
	w->setLayout(vLayout);
	w->setObjectName("testw");
	w->setMinimumSize(300,150);
	w->move(300, 400);
	w->show();*/


	connect(m_pSearchWidget,SIGNAL(search(searchOptions&)),this,SLOT(search(searchOptions&)));

	//m_pImageInfoWidget=new imageSortInfoWidget(this);
	//imageInfoDock->setWidget(m_pImageInfoWidget);
	//addDockWidget(Qt::BottomDockWidgetArea,imageInfoDock);
	// add central (sort) widget
	// take care: if you do it this way:
	// connect(m_pImageSortWidget,SIGNAL(selectionRectChanged(const QRect& r,bool b)),this,SLOT(selectionRectChanged(const QRect& r,bool b)));
	// that is, if you give the names of the signal/slot parameters, the code compiles but fails
	// the slot doesn't get called...

	//connect(m_pSearchWidget,SIGNAL(search(const searchOptions&)),m_pImageSortWidget,SLOT(download(const searchOptions&)));
	

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Verbindung zwischen Signals und Slot-Methoden
	*/
	connect(m_pImageSortWidget,SIGNAL(imageClicked(const thumbnail*)),this,SLOT(openInPreview(const thumbnail*)));
	connect(m_pPreviewCanvas,SIGNAL(showImageInfo(const imageInfo&)),m_pInfoWidget,SLOT(showImageInfo(const imageInfo&)));
	connect(m_pImageSortWidget,SIGNAL(setStatusWidget(const QString&, const QString&)),m_pStatusWidget,SLOT(setStatusWidget(const QString&, const QString&)));

	// TODO: check this connect, updateSearchButtons() is obsolete
	//connect(m_pImageSortWidget,SIGNAL(selectionChanged()),this,SLOT(updateSearchButtons()));

	connect(m_pImageSortWidget,SIGNAL(imageHovered(const thumbnail*)),this,SLOT(openInPreview(const thumbnail*)));
	connect(m_pImageSortWidget,SIGNAL(stateChanged()),this,SLOT(sortWidgetStateChanged()));
	connect(m_pImageSortWidget,SIGNAL(sortModeChanged()),this,SLOT(sortWidgetSortModeChanged()));
	connect(m_pImageSortWidget,SIGNAL(refreshFileSystem()),this,SLOT(refreshFileSystem()));
	connect(m_pImageSortWidget,SIGNAL(refreshFileSystemPath(const QString&)),this,SLOT(refreshFileSystemPath(const QString&)));

	connect(m_pImageSortWidget,SIGNAL(startWorking()),this,SLOT(showWaitCursor()));
	connect(m_pImageSortWidget,SIGNAL(stoppedWorking()),this,SLOT(hideWaitCursor()));
	connect(m_pImageSortWidget,SIGNAL(setNumSelected(const QString&)),m_pStatusWidget,SLOT(setNumSelected(const QString&)));
	//connect(m_pImageSortWidget,SIGNAL(newRawFileExts()),this,SLOT(newRawFileExts()));

	//connect(m_pPreviewCanvas,SIGNAL(showImageInfo(const imageInfo&)),this,SLOT(showImageInfo(const imageInfo&)));

	connect(m_pImageSortWidget,SIGNAL(dirCreated(const QString&)),this,SLOT(scrollExplorerTo(const QString&)));
	connect(m_pImageSortWidget,SIGNAL(lastLoadedDirs(QStringList)),this,SLOT(selectDirs(QStringList)));
	connect(m_pImageSortWidget,SIGNAL(deSelectDirs()),this,SLOT(deSelectDirs()));
	// V4DEADCODE
	// connect(m_pImageSortWidget,SIGNAL(showProgress(unsigned int)),this,SLOT(showProgress(unsigned int)));
	connect(m_pImageSortWidget,SIGNAL(startDrag()),m_pSearchWidget,SLOT(openExampleImages()));
	
	//setCentralWidget(m_pImageSortWidget);
	setCentralWidget(wrapper);
}

void imageSortMainWindow::backgroundColorActionTriggered() 
{
	QColor currentColor=m_pImageSortWidget->getBackgroundColor();
	QColorDialog::setStandardColor(0,qRgb(120,120,120));
	QColor color=QColorDialog::getColor(currentColor, this);
	
	if(color.isValid()){
		m_pImageSortWidget->setBackgroundColor(color);
		// V4.2
		m_pImageSortWidget->repaint();
		m_pPreviewCanvas->setBackgroundColor(color);
		m_pSearchWidget->setExImgBackgroundColor(color);
	}
}

void imageSortMainWindow::sortByActionTriggered() 
{
	if (sortByColorAction->isChecked()&&sortByColorAction->isEnabled()){
		// note if the dirs are dirty, load, color sort will be done automatically
		if(m_pImageSortWidget->hasDirtyDirs())
			m_pImageSortWidget->load();
		else
			m_pImageSortWidget->setSortMode(imageSortWidget::BY_COLOR,true);
		return;
	}

	// note this is bad hack:
	// since we want to use the sortByColorAction for loading if the 
	// widget dirs are dirty, we have to make sure that a user can trigger it.
	// anyway, since these actions are in a QActionGroup with exclusive access,
	// the actions can only be triggered once. if triggered, it can only be triggered again if
	// another action from the group has been triggered meanwhile.
	// the bad hack is to first disable and then trigger() the sortByNameAction 
	// in restrictGui() when the dirs are dirty. the trigger comes out here,
	// if another action was triggered before, thus we have to test this action for
	// beeing disabled...
	if (sortByNameAction->isChecked()&&sortByNameAction->isEnabled())
		m_pImageSortWidget->setSortMode(imageSortWidget::BY_NAME,true);
	else if (sortByDateAction->isChecked())		
		m_pImageSortWidget->setSortMode(imageSortWidget::BY_DATE,true);
	else if (sortBySizeAction->isChecked())
		m_pImageSortWidget->setSortMode(imageSortWidget::BY_SIZE,true);
	else if (sortBySimilarityAction->isChecked())
		m_pImageSortWidget->setSortMode(imageSortWidget::BY_SIMILARITY_LIST,true);
}

void imageSortMainWindow::restrictGui()
{
	switch(m_pImageSortWidget->getState()){
		case imageSortWidget::LOADING:
		case imageSortWidget::CHANGING_DIRS:
			// V4.2: we may abort this by ESC
			setFocus();
			m_pImageSortWidget->setEnabled(false);
			// ANUJ***
			// this is definitly the wrong place for resetting the zoom
			// because as the function name implies, this function restricts
			// the user interface capabilities, not the visual representation
			m_pImageSortWidget->resetZoom(true);
			// ***ANUJ
			sortByNameAction->setEnabled(false);
			sortByDateAction->setEnabled(false);
			sortBySizeAction->setEnabled(false);
			sortByColorAction->setEnabled(false);
			sortBySimilarityAction->setEnabled(false);

			//resetVisualizationAction->setEnabled(false);
			resetZoomAction->setEnabled(false);
			fullscreenAction->setEnabled(false);

			zoomInAction->setEnabled(false);
			zoomOutAction->setEnabled(false);

			bgColorAction->setEnabled(false);

			//m_pImageSortWidget->setEnabled(false);
			m_pFolderView->setEnabled(false);
			m_pPreviewCanvas->setEnabled(false);

			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Slot-Methode für Einzelbildansicht
			*/
			m_pSearchWidget->setEnabled(false);

			break;
		case imageSortWidget::SORTING:
			// V4.2: we may abort this by ESC
			setFocus();
			m_pImageSortWidget->setEnabled(false);
			sortByColorAction->setEnabled(true);
			sortByNameAction->setEnabled(m_pImageSortWidget->canDoNameSort());
			sortByDateAction->setEnabled(m_pImageSortWidget->canDoDateSort());
			sortBySizeAction->setEnabled(m_pImageSortWidget->canDoSizeSort());
			sortBySimilarityAction->setEnabled(m_pImageSortWidget->hasSimilaritySort());

			//resetVisualizationAction->setEnabled(false);
			resetZoomAction->setEnabled(false);
			fullscreenAction->setEnabled(false);

			zoomInAction->setEnabled(false);
			zoomOutAction->setEnabled(false);

			bgColorAction->setEnabled(false);

			m_pImageSortWidget->setEnabled(false);
			m_pFolderView->setEnabled(false);
			m_pPreviewCanvas->setEnabled(false);

			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Slot-Methode für Einzelbildansicht
			*/
			m_pSearchWidget->setEnabled(false);

			break;
		case imageSortWidget::DIRSDIRTY:
			// V4.2: we may abort this by ESC
			setFocus();
			m_pImageSortWidget->setEnabled(false);
			sortByNameAction->setEnabled(false);
			// see sortByActionTriggered()
			sortByNameAction->trigger();
			sortByDateAction->setEnabled(false);
			sortBySizeAction->setEnabled(false);
			sortByColorAction->setEnabled(true);
			sortBySimilarityAction->setEnabled(false);

			//resetVisualizationAction->setEnabled(false);
			resetZoomAction->setEnabled(false);
			fullscreenAction->setEnabled(false);

			zoomInAction->setEnabled(false);
			zoomOutAction->setEnabled(false);

			bgColorAction->setEnabled(false);

			m_pImageSortWidget->setEnabled(true);
			m_pFolderView->setEnabled(true);
			m_pPreviewCanvas->setEnabled(true);
			
			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Slot-Methode für Einzelbildansicht
			*/
			m_pSearchWidget->setEnabled(true);
			break;
			// V4.2: new state
			// same as HARDDISK_SEARCH, but no wait cursor is displayed...
		case imageSortWidget::PREPARE_HARDDISK_SEARCH:
			// V4.2: we may abort this by ESC
			setFocus();
			m_pImageSortWidget->setEnabled(false);
			sortByNameAction->setEnabled(false);
			sortByDateAction->setEnabled(false);
			sortBySizeAction->setEnabled(false);
			sortByColorAction->setEnabled(false);
			sortBySimilarityAction->setEnabled(false);
			
			//resetVisualizationAction->setEnabled(false);
			resetZoomAction->setEnabled(false);
			fullscreenAction->setEnabled(false);
			
			zoomInAction->setEnabled(false);
			zoomOutAction->setEnabled(false);
			
			bgColorAction->setEnabled(false);
			
			m_pImageSortWidget->setEnabled(true);
			m_pFolderView->setEnabled(true);
			m_pPreviewCanvas->setEnabled(false);
			
			/*
			 *	Hinzugefügt von Claudius Brämer und David Piegza
			 *
			 *   Slot-Methode für Einzelbildansicht
			 */
			m_pSearchWidget->setEnabled(false);
			break;
		case imageSortWidget::HARDDISK_SEARCH:
			// V4.2: we may abort this by ESC
			setFocus();
			m_pImageSortWidget->setEnabled(false);
			sortByNameAction->setEnabled(false);
			sortByDateAction->setEnabled(false);
			sortBySizeAction->setEnabled(false);
			sortByColorAction->setEnabled(false);
			sortBySimilarityAction->setEnabled(false);

			//resetVisualizationAction->setEnabled(false);
			resetZoomAction->setEnabled(false);
			fullscreenAction->setEnabled(false);

			zoomInAction->setEnabled(false);
			zoomOutAction->setEnabled(false);

			bgColorAction->setEnabled(false);

			m_pImageSortWidget->setEnabled(true);
			m_pFolderView->setEnabled(true);
			m_pPreviewCanvas->setEnabled(false);

			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Slot-Methode für Einzelbildansicht
			*/
			m_pSearchWidget->setEnabled(false);
			break;

		case imageSortWidget::IDLE:
		default:
			m_pImageSortWidget->setEnabled(true);
			// dirty hack:
			// see sortByActionTriggered
			sortByColorAction->setEnabled(false);
			sortByColorAction->trigger();
			sortByColorAction->setEnabled(true);

			sortByNameAction->setEnabled(m_pImageSortWidget->canDoNameSort());
			sortByDateAction->setEnabled(m_pImageSortWidget->canDoDateSort());
			sortBySizeAction->setEnabled(m_pImageSortWidget->canDoSizeSort());
			sortBySimilarityAction->setEnabled(m_pImageSortWidget->hasSimilaritySort());

			//resetVisualizationAction->setEnabled(true);
			resetZoomAction->setEnabled(true);
			fullscreenAction->setEnabled(true);

			zoomInAction->setEnabled(true);
			zoomOutAction->setEnabled(true);

			bgColorAction->setEnabled(true);

			m_pFolderView->setEnabled(true);
			m_pPreviewCanvas->setEnabled(true);
			m_pImageSortWidget->setEnabled(true);
			
			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Slot-Methode für Einzelbildansicht
			*/
			m_pSearchWidget->setEnabled(true);

			/* Old search toolbar
			queryLine->setEnabled(true);
			inetSearchServiceDropList->setEnabled(true);
			updateSearchButtons();*/
			// set focus, otherwise wheel events are not delivered to sort widget...
			//m_pImageSortWidget->setFocus();
			break;
	}
}

void imageSortMainWindow::about() 
{
	AboutDialog dlg(this);
	dlg.exec();
}

#if defined(Q_WS_MAC)
void imageSortMainWindow::showHiddenFiles(bool b)
{
	if(b)
		m_Folders.setFilter(QDir::AllDirs|QDir::Files|QDir::Hidden|QDir::CaseSensitive|QDir::NoDotAndDotDot);
	else
		m_Folders.setFilter(QDir::AllDirs|QDir::Files|QDir::CaseSensitive|QDir::NoDotAndDotDot);
}
#endif


void imageSortMainWindow::setSearchOptions() 
{
	// this was a severe anujish bug:
	// InternetSetDialog dlg=new InternetSetDialog(this);
	// the funny thing is: it worked on win32/visual studio 2005, although it is complete nonsense
	// the correct way is either:
	// 
	// InternetSetDialog *dlg=new InternetSetDialog(this);
	// dlg->setQuery() 
	// ... and so on ...
	// delete dlg
	// or:
	InternetSetDialog dlg(this);
	dlg.setNumQueries(m_pImageSortWidget->numQueries());
	dlg.setNumSimilarQueries(m_pImageSortWidget->numSimilarQueries());
	dlg.setNumSimilarShown((m_pImageSortWidget->numSimilarShown()));
	dlg.setNumNetSearchThreads((m_pImageSortWidget->numNetSearchThreads()));
	if(dlg.exec()==QDialog::Accepted){
		m_pImageSortWidget->setSearchOptions("",dlg.numQueries(),dlg.numSimilarQueries(),dlg.numSimilarShown(),dlg.numnetSearchThreads());
	}
	// on os x, this did not compile because QDialog has a private copy ctor and the compiler
	// somehow tried to create the InternetSetDialig instance dlg from the pointer returned by new()...
	// i have no idea how visual studio 2005 can compile this, at all...
}

void imageSortMainWindow::setSortOptions() 
{
	sortOptionsDialog dlg(this);
	dlg.setDimFactor(m_pImageSortWidget->dimFactor());
	if(dlg.exec()==QDialog::Accepted){
		m_pImageSortWidget->setDimFactor(dlg.dimFactor());
	}
}


void imageSortMainWindow::closeEvent(QCloseEvent *event) 
{
	saveSettings();
	event->accept();
}

void imageSortMainWindow::openInPreview(const QFileInfo& info) 
{
	assert(!info.isDir());
	
	// may be an empty (constructed) QFileIno
	if(info.isFile()){
		// show selected image in preview dock
		previewDock->setWindowTitle(info.absoluteFilePath());
		m_pPreviewCanvas->setImageFileInfo(info);
	}
	else{
		previewDock->setWindowTitle(tr("Preview"));
		m_pPreviewCanvas->clearCanvas();
	}
}

void imageSortMainWindow::openInPreview(const thumbnail* pThumb) 
{
	if(pThumb)
		previewDock->setWindowTitle(pThumb->m_YahooURLs.m_ClickUrl);
	else
		previewDock->setWindowTitle(tr("Preview"));

	m_pPreviewCanvas->setThumbnail(pThumb);
}




void imageSortMainWindow::dockFloatStateChanged(bool b)
{
	// TODOV1.1: really needed?
	m_pImageSortWidget->repaint();
}

void imageSortMainWindow::reset()
{
	m_pImageSortWidget->reset();

	restrictGui();

}

void imageSortMainWindow::resetVisualization(bool bForceRedraw)
{
	// TODOV1.1: call updateVisualizationButtons() as a Slot based on a signal emitted by 
	// m_pImageSortWidget
	m_pImageSortWidget->resetVisualization();
	updateVisualizationButtons();

}

//V4DEADCODE
//void imageSortMainWindow::updateStatusBarInfoLabel(QString text, bool forceRedraw) 
//{
//	if(statusBar != NULL) {
//		m_pStatusBarInfoLabel->setText(text);
//		if (forceRedraw)
//			m_pStatusBarInfoLabel->repaint();
//	}
//}

void imageSortMainWindow::updateVisualizationButtons() 
{
}


void imageSortMainWindow::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) 
{
	// we may get here by scrollExplorerTo(directoryCreatedByWorker), where the dir 
	// is created by the worker and thus already added. we neither need this stuff here
	// nor do we want it, because in turn it tells the widget to redraw, which is done by the worker already...
	// thus:
	if(m_bSelectionChangedInternally)
		return;

	// note our QTreeView derivate ISTreeView only selects dirs, no files...

	// V3BETA2: we may load recursive now. we have to first remove, then add the dirs for this...
	// remove all deselected dirs from the widgets dir list

	// V3BETA2: i don't know why, but the QItemSelections we get here contain 4 QModelIndexValues per directory
	// i think i misunderstood the model/view paradigm of qt. Anyway, the 4 QModelIndex Values seem to index always the same folder
	// thus we do the following:
	// we get the folder for each index and push them in a QSet
	// then we call our widget with the contents of the set...

	QModelIndexList deSelList=deselected.indexes();
	QModelIndexList::iterator it=deSelList.begin();
	while(it!=deSelList.end()){
		QModelIndex& rIndex=*it;
		++it;
		assert(m_Folders.isDir(rIndex));
		m_DirsToRemove.insert(m_Folders.fileInfo(rIndex).absoluteFilePath());
	}

	QModelIndexList selList=selected.indexes();
	it=selList.begin();
	while(it!=selList.end()){
		QModelIndex& rIndex=*it;
		++it;
		assert(m_Folders.isDir(rIndex));
		m_DirsToAdd.insert(m_Folders.fileInfo(rIndex).absoluteFilePath());
	}

	// note if we get here from loadSettings(), only do this on the last folder added...
	if(!m_bDelayChangeDirs){
		m_pImageSortWidget->changeDirs(m_DirsToRemove,m_DirsToAdd);
		m_DirsToRemove.clear();
		m_DirsToAdd.clear();
	}
	// V2.03: we can have sym links to folders now
	// due to the logic of imageSortWidget::addDir(), adding a sym link folder fails if
	// the sym link target is already in the list. thus, if a user had selected the sym link target
	// and then selects the sym link, the first addDir() fails, the removeDir() works and thus the
	// dir list is empty and we get the 'no folder selected' message. thus, add again, this will be NOP
	// on all dirs already contained
	// (first removing all unselected dirs and then adding seems to be more logically, but gives strange results...)
	//it=selList.begin();
	//while(it!=selList.end()){
	//	QModelIndex& rIndex=*it;
	//	++it;
	//	assert(m_Folders.isDir(rIndex));
	//	m_pImageSortWidget->addDir(m_Folders.fileInfo(rIndex).absoluteFilePath());
	//}
}

void imageSortMainWindow::hideWaitCursor()
{
	// note the override cursor stack of QApplication can be annoying, since 
	// we have a multithreaded application and this function is called from queued slots
	// i don't know if the calling order for these slots is given, anyway we sometimes
	// ended up in having the wait cursor left. thus, we pop the stack until it is empty here...
	// TODOV4: check why this is no longer needed...
	while(QApplication::overrideCursor())
		QApplication::restoreOverrideCursor();
	//m_pImageSortWidget->setCursor(Qt::ArrowCursor);

}
void imageSortMainWindow::showWaitCursor()
{
	// TODOV4: check why this is no longer needed...
	// if the override cursor stack is not empty, pop it...
	hideWaitCursor();
	QApplication::setOverrideCursor(Qt::WaitCursor);
	//m_pImageSortWidget->setCursor(Qt::WaitCursor);
}


/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Event-Methode zur Anpassung der Toolbarhöhe
*/
// TODOV4: seems to be obsolete
void imageSortMainWindow::resizeEvent(QResizeEvent *re) 
{
	//toolBar->setMinimumHeight(re->size().height());
	//toolBar->setMaximumHeight(re->size().height());
}


void imageSortMainWindow::keyPressEvent(QKeyEvent *pe) 
{
#pragma message("do we need this in the main window??")
	//qDebug("%d",focusPolicy());
	//qDebug("key %d", pe->key());
	switch(pe->key()){
		case Qt::Key_Escape:
			m_pImageSortWidget->escPressed();
			//m_pImageSortWidget->abortLoadOrSort();
			break;
		//case Qt::Key_F5:
		//	m_pImageSortWidget->load();
		//	break;		
		case Qt::Key_F11:
			showMainArea();
			break;		
		default:
			QMainWindow::keyPressEvent(pe);
			//pe->ignore();
	}
}


/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Event-Methode für Vollbildmodus
*/
// TODOV4: notwendig?
bool imageSortMainWindow::event(QEvent *event)
{
	// Abfangen der Tabulator-Taste
    if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ke = static_cast<QKeyEvent *>(event);
         if (ke->key() == Qt::Key_Tab) {
			 showMainArea();
             return true;
         }
     }
     return QMainWindow::event(event);
}


/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Methode für Vollbildmodus
*/
// TODOV4: does this toggle from/to main view? then: rename it...
void imageSortMainWindow::showMainArea() 
{
	//toolBar->setVisible(!toolBar->isVisible());
	m_menuBar->setVisible(!m_menuBar->isVisible());
	explorerDock->setVisible(!explorerDock->isVisible());
	previewDock->setVisible(!previewDock->isVisible());
	infoDock->setVisible(!infoDock->isVisible());
	searchDock->setVisible(!searchDock->isVisible());
	statusDock->setVisible(!statusDock->isVisible());
}


void imageSortMainWindow::sortWidgetStateChanged()
{
	//V4DEADCODE
	//updateStatusBarInfoLabel(m_pImageSortWidget->stateString(),true);
	restrictGui();
	if(m_pImageSortWidget->isIdle()||m_pImageSortWidget->isPreparingHardDiskSearch()||m_pImageSortWidget->hasDirtyDirs())
		hideWaitCursor();
	else
		showWaitCursor();
}

//V4DEADCODE
//void imageSortMainWindow::showProgress(unsigned int numLoaded)
//{
//	updateStatusBarInfoLabel(tr("checked ")+QString::number(numLoaded)+tr(" images... Press ESC to abort..."),true);
//}

void imageSortMainWindow::refreshFileSystem()
{
	// V2.03:
	// niote this is really strange: if we call QDirModel::refresh(),
	// our ISTreeView highlights the sym link target folder when we have loaded
	// a dir by a sym link. since we build in the whole sym link stuff to
	// have a workaround for the external volumes not shown on mac os, we 
	// cannot do this here...
	// V4.2: m_Folders is a QFileSystemModel, no QDirModel any longer
	//if(!m_pImageSortWidget->hasSymLinkDirs())
	//	m_Folders.refresh();
}

void imageSortMainWindow::refreshFileSystemPath(const QString& path)
{
	// V2.03:
	// niote this is really strange: if we call QDirModel::refresh(),
	// our ISTreeView highlights the sym link target folder when we have loaded
	// a dir by a sym link. since we build in the whole sym link stuff to
	// have a workaround for the external volumes not shown on mac os, we 
	// cannot do this here...
	// V4.2: m_Folders is a QFileSystemModel, no QDirModel any longer
	//if(!m_pImageSortWidget->hasSymLinkDirs())
	//	m_Folders.refresh(m_Folders.index(path));
}

void imageSortMainWindow::sortWidgetSortModeChanged()
{
	switch(m_pImageSortWidget->getSortMode()){
		case imageSortWidget::BY_COLOR:
			sortByColorAction->setChecked(true);
			break;
		case imageSortWidget::BY_DATE:
			sortByDateAction->setChecked(true);
			break;
		case imageSortWidget::BY_SIZE:
			sortBySizeAction->setChecked(true);
			break;
		case imageSortWidget::BY_SIMILARITY_LIST:
			sortBySimilarityAction->setChecked(true);
			break;
		case imageSortWidget::BY_NAME:
		default:
			sortByNameAction->setChecked(true);
			break;
	}
}



/*void imageSortMainWindow::showImageInfo(const imageInfo& info)
{
	if(!info.m_bIsValid){
		m_pStatusBarImageNameLabel->setText("");
		m_pStatusBarImageSizeLabel->setText("");
		m_pStatusBarFileSizeLabel->setText("");
		m_pStatusBarImageDateLabel->setText("");
	}
	else{
		
		
		m_pStatusBarImageNameLabel->setText(info.m_PathOrURL);
		if(info.m_bLoadFailure)
			m_pStatusBarImageSizeLabel->setText(tr("load error"));
		else{
			QString str;
			if(info.m_XSize==0)
				str="?";
			else
				str.setNum(info.m_XSize);
			str+=" * ";
			QString str2;
			if(info.m_YSize==0)
				str2="?";
			else
				str2.setNum(info.m_YSize);
			str+=str2;
			str+=tr(" Pixels");
			m_pStatusBarImageSizeLabel->setText(str);
		}
		if(info.m_bIsFromNet){
			m_pStatusBarFileSizeLabel->setText("");
			m_pStatusBarImageDateLabel->setText("");
		}
		else{
			m_pStatusBarFileSizeLabel->setText(formatImageSize(info.m_FileSize));
			m_pStatusBarImageDateLabel->setText(info.m_LastModified.toLocalTime().toString("dd.MM.yyyy hh:mm"));
		}
	}
	statusBar->repaint();
}

QString imageSortMainWindow::formatImageSize(qint64 size) 
{
	double s=(double)size;

	// return Bytes
	if (s < 1024.)
		return QString("%L1").arg(s,0,'f',0)+" Byte";


	// no rounding on KByte or MByte, just cutoff...

	// calculate KB
	s /= 1024.;

	// return KB
	if (s < 1000.)
		return QString("%L1").arg(s,0,'f',0)+" KByte";
	
	// calculate MB
	s /= 1024.;

	// return MB
	return QString("%L1").arg(s,0,'f',0)+" MByte";
}
*/

/*!
called when the user changes file extensions for raw files
*/
/*
void imageSortMainWindow::newRawFileExts()
{
	m_Folders.setNameFilters(m_pImageSortWidget->nameFilter());
	m_Folders.refresh();
}
*/
//void imageSortMainWindow::queryReturnPressed()
//{
//	// if we have a selection in the widget, we'll search for similar on the net,
//	// if not, we'll do a normal net search
//	// took this out again...
//	//if(m_pImageSortWidget->hasSelection())
//	//	m_pImageSortWidget->downloadSimilars();
//	//else
//		m_pImageSortWidget->download();
//}

void imageSortMainWindow::scrollExplorerTo(const QString& absPath)
{
	// TODO: this is a bit mad, we maybe better disconnect the selections model signal...
	m_bSelectionChangedInternally=true;
	m_pSelectionModel->clear();
	m_pSelectionModel->select(m_Folders.index(absPath),QItemSelectionModel::Select);
	m_pFolderView->collapseAll();
	m_pFolderView->scrollTo(m_Folders.index(absPath));
	m_bSelectionChangedInternally=false;

}

void imageSortMainWindow::deSelectDirs()
{
	m_pSelectionModel->clear();
}

void imageSortMainWindow::selectDirs(QStringList dirs)
{
	if(dirs.empty())
		return;

	// TODO: this is a bit mad, we maybe better disconnect the selections model signal...
	// m_bSelectionChangedInternally=true;
	m_bDelayChangeDirs=true;
	m_pSelectionModel->clear();
	m_pFolderView->collapseAll();
	int n=dirs.size();
	for(int i=0;i<n;++i){
		if(i+1==n){
			m_bDelayChangeDirs=false;
			// V3 BETA 3: took this out, was a bug, i think:
			// when a dir ABC was selected (but not loaded, thus the load message ("dbl clck here..."
			// was displayed and a user changes to another dir (let's say C: or /), the dir scan started. 
			// When the user aborted the dir scan, the dir ABC was *loaded*. Without this, the load message is displayed..
			// m_pImageSortWidget->enableAutoLoad(true);
		}
		m_pSelectionModel->select(m_Folders.index(dirs[i]),QItemSelectionModel::Select);
		m_pFolderView->scrollTo(m_Folders.index(dirs[i]));
	}
	// m_bSelectionChangedInternally=false;

}

/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Slot-Methode für Bildsuche, wird ausgeführt, wenn der Suchbutton
*   geklickt wurde
*/
void imageSortMainWindow::search(searchOptions& options) {
	if(options.m_Source == 0) {
		// show info widget to select a folder in explorer
		deSelectDirs();
		m_pImageSortWidget->searchDisk(options);
		hideWaitCursor();
	} else {
		m_pImageSortWidget->download(options);
	}
	m_pPreviewCanvas->clearCanvas();
}
