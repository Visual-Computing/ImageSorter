#include "imagesortwidget.h"
#include "imagesortmainwindow.h"
#include "isfilterwidgets.h"

#include "desktopserviceopenurlthread.h"
//#include <QDesktopServices>
#include <assert.h>

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
imageSortWidget::imageSortWidget(QFrame *p, Qt::WFlags f) : QFrame(p,f)
{
	setBackgroundColor(QColor(120,120,120));
	setPalette(QPalette(getBackgroundColor()));

	// TODO: do not alloc by new...
	m_pSplashImage = new QImage(":/splash");

	setAutoFillBackground(true);
	setMouseTracking(true);

	resetLeftMouseButtonKeys();
	m_MousePressedLeftPos.rx()=0;
	m_MousePressedLeftPos.ry()=0;
	//m_MousePressedX = 0;
	//m_MousePressedY = 0;
	//m_MouseLastX = -1;
	//m_MouseLastY = -1;
	//m_MousePosX = 0;
	//m_MousePosY = 0;

	m_SortMode=BY_NAME;

	// note this is more or less a static const currently
	// there is no interface to set this...
	m_uThumbSize=128;

	createContextMenu();

	m_Worker.init(this);
	// note these are queued connections, since the sender emits the signal from
	// a different thread...
	connect(&m_Worker,SIGNAL(dirsChanged()),this,SLOT(dirsChanged()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(dirsChangeAborted()),this,SLOT(dirsChangeAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(loadingStarted()),this,SLOT(loadingStarted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(repaintWidget()),this,SLOT(repaintRequested()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(updateStatusBar()),this,SLOT(updateStatusBarRequested()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(updateStatusBar2(unsigned,unsigned)),this,SLOT(updateStatusBar2Requested(unsigned,unsigned)),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(loadCompleted()),this,SLOT(loadCompleted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(loadAborted()),this,SLOT(loadAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(loadSimilarCompleted()),this,SLOT(loadSimilarCompleted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(loadSimilarAborted()),this,SLOT(loadSimilarAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(inetLoadCompleted()),this,SLOT(inetLoadCompleted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(inetLoadAborted()),this,SLOT(inetLoadAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(inetLoadFailed(QString)),this,SLOT(inetLoadFailed(QString)),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(inetLoadSimilarCompleted()),this,SLOT(inetLoadSimilarCompleted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(inetLoadSimilarAborted()),this,SLOT(inetLoadSimilarAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(sortingStarted()),this,SLOT(colorSortStarted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(newSort()),this,SLOT(newColorSort()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(sortCompleted()),this,SLOT(colorSortCompleted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(sortAborted()),this,SLOT(colorSortAborted()),Qt::QueuedConnection);
	connect(&m_Worker,SIGNAL(dirCreated(const QString&)),this,SLOT(dirCreatedByWorker(const QString&)));


	// a signal to signal connection:
	// queued because m_SimilarItems.insert() is called in the worker thread...

	// this is a bit tricky: inserted() is a signal of the worker. it is emitted on behalf of m_DynamicSearcher (which does not know about signals/slots)
	connect(&m_Worker,SIGNAL(inserted(unsigned int)),this,SLOT(updateProgressBar(unsigned int)),Qt::QueuedConnection);

	reset();
	resetVisualization(false);
	// note if this is set to true here, the splash screen is shown for a second in the widget
	// when the first paintEvent() occurs...
	m_bShowSplash=true;

	m_SelectionRect.setRect(0,0,0,0);

	// V2.02 display arrow cursor as standard cursor
	// display closed hand cursor when dragging
	// V4.2: just use the ArrowCursor only...
	setCursor(Qt::ArrowCursor);

	setFocusPolicy(Qt::WheelFocus);
	//m_bKeyPressed = false;

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Initialisierung der Variable für Drag & Drop
	*/
	m_bDragEnabled = false;
	m_bScrolling = false;

	m_bInetLoad = false;
	m_bInetLoadSimilar = false;
	m_bHardDiskSearch = false;
	m_bHarddiskLoadSimilar = false;


	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Erstellen des Buttons für den Start des Ladevorgangs von der Festplatte
	*/

	m_pMsgWidget = new ISMsgWidget(this);
	m_pMsgWidget->resize(300,150);
	//m_pMsgWidget->setVisible(false);
	
	m_pMsgStart = new ISMsgWidget(this);
	m_pMsgStart->resize(300,150);
	//m_pMsgStart->setVisible(false);
	QPushButton *pb = m_pMsgStart->addButton(tr("Load and sort images"));
#if defined(Q_WS_MAC)
	pb->setStyleSheet("color : black");
#endif
#if defined(Q_WS_WIN)
	pb->setMaximumHeight(40);
	pb->setMaximumWidth(180);
#endif	
	connect(pb, SIGNAL(clicked()),this, SLOT(loadAndSortBtnClicked())); 

	m_pMsgWidget->setObjectName("msgWidget");
	m_pMsgStart->setObjectName("msgStart");
	hideMsgWidgets();

	// we need a mutex here because loading is and sorting/searching may be in a separate thread
	m_Sorter.setMutex(&m_Mutex);
	m_DynamicSearcher.setMutex(&m_Mutex);
	// we always sort in a separate thread, thus:
	m_Sorter.enableAutoMutex(false);
	m_Sorter.enableMutex();
	// enableAutoMutex(false) by default here:
	m_DynamicSearcher.enableMutex();

	// we use a guiController to handle mouse, zoom and size events
	m_Sorter.attach(&m_GuiController);
	m_DynamicSearcher.attach(&m_GuiController);

	// this initializes the guiController
	m_GuiController.reset(false);

	// we need to register a callback in the guiControler
	// this get's called whenever the guiController requests a (re)draw of the images,
	// i.e. due to a zoom event
	m_GuiController.registerPaintRequestCallback((isortisearch::pxCallback)repaintRequestedStaticCB,this);

	m_bNoRepaint=false;
}

imageSortWidget::~imageSortWidget() {
	// stop the color sorter gracefully
	// user may close app while color sorting...
	m_Worker.stop();
	// note: during development of version 1, there where crashes (access violation)
	// in the QMutex scalar deletion operator called here implicitly
	// debugging turned out that the mutex was 
	// 
	// a) locked 
	// b) by another thread (id)
	//
	// both conditions where quite strange, since 
	//
	// a) the lock/unlock count was balanced 
	// (proven by subclassing QMutex and inc/decrementing
	// a counter on each lock()/unlock())
	// b) there was no thread with that id during executuion
	//
	// in this situation, the following code will else-branch
	// (and crash, since a QMutex cannot be unlocked by a thread which didn't lock it):
	//
	//if(m_Mutex.tryLock())
	//	m_Mutex.unlock();
	//else{
	//	m_Mutex.unlock();
	//}
	//
	// another strange behaviour was that it was not possible to set a breakpoint in main()
	// (to check the main thread id) in this situation. however, the behaviour went away
	// silently. propably by a rebuild all...

	delete m_pSplashImage;

	deallocate();
}


/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Slot-Methode zum Laden der Bilder von der Festplatte
*/
void imageSortWidget::loadAndSortBtnClicked()
{	
	//m_pMsgStart->setVisible(false);
	//m_pMsgWidget->setVisible(false);
	hideMsgWidgets();
	
	if(hasDirtyDirs()){
		load();
		return;
	}
}


QSize imageSortWidget::minimumSizeHint() const 
{
	return QSize(50, 50);
}

void imageSortWidget::allocate(unsigned numItems)
{
	deallocate();

	//assert(numItems);

	// note new behaviour:
	// allocate() is called by our worker now
	// the worker also calculates the number of files to load a.k.a the expected thumbs:
	m_uNumThumbsExpected=numItems;

	if(!numItems)
		return;

	// allocate the sort arrays
	// note we'll have the sort by name by reading in from the QFileInfoList...
	// NOTE we don't really need all these in all situations
	// e.g., when we look for similars on flickr, we don't need the name sort, the date sort and the size sort
	// because we don't have any meaningfull values to sort here...
#pragma message("only allocate sort arrays we really need...")
	m_SortedByName.reserve(numItems);
	m_SortedByDate.reserve(numItems);
	m_SortedBySize.reserve(numItems);
	m_ControllerImages.reserve(numItems);
}



void imageSortWidget::deallocate()
{
	m_SortedByName.clear();
	m_SortedByDate.clear();
	m_SortedBySize.clear();
	m_ControllerImages.clear();
	//TODOV5: correct?
	//m_DynamicSearcher.clearResults();
	//m_DynamicSearcher.clearTargets();
}


// TODOV4: obsolete?
void imageSortWidget::enterEvent(QEvent *) 
{
	//setFocus();
}

void imageSortWidget::leaveEvent(QEvent *) 
{
	// V5: tell the library
	m_GuiController.onMouseLeave();
	
	if(m_bPreviewOnMouseOver)
		emit imageHovered(NULL);
}

void imageSortWidget::mouseDoubleClickEvent(QMouseEvent *event) 
{
	// V4.2:
	if(!isIdle())
		return;
	   
		
	// note this prevents selections in mouse dbl click (hopefully)
	m_MousePressedLeftPos=event->pos();

	// V5: getPictureAt() returns a refPtr<controllerImage>...
	thumbnail *item = getPictureAt(event->pos());

	if(item){
		if(item->isNotLoaded()){
			QString str(tr("Could not open image file "));
			//str+=item->m_FileInfo.absoluteFilePath();
			str+=item->m_FileName;
			QMessageBox::critical(this,
								 tr("Error"),
								 str,
								 QMessageBox::Ok,0);
			return;
		}
		else{
			QUrl toOpen;
			if(item->isLoadedFromNet()){
				if(item->isFromYahoo())
					// toOpen.setUrl(item->m_YahooURLs.m_ClickUrl);
					// new V4: open the web page, not the image...
					toOpen.setUrl(item->m_YahooURLs.m_RefererUrl);
				else
					toOpen.setUrl(item->m_flickrURLs.clickURL());
			}
			else
			{	
				//toOpen=QUrl::fromLocalFile(item->m_FileInfo.absoluteFilePath());
				toOpen=QUrl::fromLocalFile(item->m_FileName);
				
			}	
			
			// this thread will delete itself when finished:
			desktopServiceOpenUrlThread* pFireAndForget=new desktopServiceOpenUrlThread(toOpen);
			pFireAndForget->start();
		}
	}
}


void imageSortWidget::paintEvent(QPaintEvent *)
{
	// show splash once at startup for a second
	if(m_bShowSplash){
		paintStartUpScreen();
		QTimer::singleShot(1000,this,SLOT(splashWindowShown()));
		return;
	}

	if(isScanning()){
		paintScanningScreen();
		return;
	}
	// V4.3: loading thumbs db is gone...
	//if(!isEnabled()&&isLoadingThumbsDB()) {
	//	hideMsgWidgets();
	//	//paintLoadingScreen();
	//	return;
	//}

	// Why???
	//if(m_State==HARDDISK_SEARCH/* || m_bHardDiskSearch*/)
	//	return;

	//if(m_State==PREPARE_HARDDISK_SEARCH)
	//	return;

	if(m_bNoRepaint)
		return;

	// new: paint message if dirs are dirty
	// but *only* if the widget is enabled
	// if the widget is not enabled, we load or sort currently.
	// the loader itself sets the dirs dirty, too
	// thus, w/o the enabled check we'll have this message as long as the load takes...
	if(isEnabled()){
		if(m_State==DIRSDIRTY){
		//if(m_Worker.cdirs().dirty()){
			paintDirsDirtyScreen();
			return;
		}
	}

	// no images?
	const unsigned nThumbs=numThumbs();
	if(!nThumbs){
		paintNoImageScreen();
		return;
	}

	// if we paint while loading, we have to let the gui controller calculate
	if(m_SortMode==BY_DISK_LOAD){
		// note locks...
		m_GuiController.calculate();
	}

	paintImages();
	
	return;
}

void imageSortWidget::paintImages()
{
	const bool bNeedLock=isSorting()||isLoading();

	QPainter painter(this);

	// draw
	setPalette(QPalette(getBackgroundColor()));

	if(bNeedLock)
		lock();

	std::vector<refPtr<controllerImage> >::const_iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::const_iterator itEnd=m_ControllerImages.end();
	while(it!=itEnd){
		thumbnail* pImg=(thumbnail*)((*it).operator->());
		++it;
		
		if(pImg->isMovedOrDeleted()||!pImg->isValid()||!pImg->visible())
			continue;

		const int ulx=pImg->upperLeftX();
		const int uly=pImg->upperLeftY();
		const int lrx=pImg->lowerRightX();
		const int lry=pImg->lowerRightY();
		int w=lrx-ulx;
		int h=lry-uly;

		QImage imgToDraw;

		// draw the high resolution image if the size becomes bigger than 300 pixels 
		// and the center of this image is visible
		// NOTE do not do this while loading...
		if (!bNeedLock && !pImg->isLoadedFromNet() && (w > 300 || h > 300)) {
			if (pImg->m_highResImage.isNull()){
				emit startWorking();
				//if(!(pImg->m_highResImage.load(pImg->m_FileInfo.absoluteFilePath())))
				if(!(pImg->m_highResImage.load(pImg->m_FileName)))
					pImg->m_highResImage=QImage();
				emit stoppedWorking();
			}

			if(!(pImg->m_highResImage.isNull()))
				imgToDraw = pImg->m_highResImage;
			else
				imgToDraw=pImg->m_Img;
		}
		else {
			// delete the high res image if we do not need it anymore
			if(!(pImg->m_highResImage.isNull())) {
#pragma message("test if the highres image is deleted by detach...")
				pImg->m_highResImage=QImage();
			}
			imgToDraw=pImg->m_Img;
		}

		QRectF target(ulx,uly,w,h);
		QRectF source(0.0,0.0,imgToDraw.width(),imgToDraw.height());

		painter.drawImage(target,imgToDraw,source);

		// draw border if image is selected, but not preDeSelcted or if preSelected
		if ((pImg->selected()&&!pImg->preDeSelected())||pImg->preSelected()) {
			QRectF border(ulx-1,uly-1,w+2,h+2);
			painter.drawRect(border);
		}	
	}

	if(bNeedLock)
		unlock();
	
	if(m_SelectionRect.isValid()){
		QRectF selectRect(m_SelectionRect);
		painter.drawRect(selectRect);
	}

}

thumbnail* imageSortWidget::getPictureAt(const QPoint &pos) 
{	
	if(!hasImages())
		return NULL;

	// get the image from the library
	// note the library returns a rePptr to the base class of thumbnail, safe to cast here
	thumbnail *p=(thumbnail*)(m_GuiController.getImageAt(pos.x(),pos.y()).operator->());
	return p;
}


/*
*	Geändert von Claudius Brämer und David Piegza
*
*   Anzeige der Anzahl Dateien aus ausgewähltem Ordner
*/
void imageSortWidget::paintMessage(QString message)
{	
	if(!m_bHardDiskSearch) {
		hideMsgWidgets();
		m_pMsgWidget->reset();
		m_pMsgWidget->setText(message);
		m_pMsgWidget->show();
	}	
}


void imageSortWidget::reset() 
{
	//m_uNumThumbs=0;
	m_uNumThumbsExpected=0;
	m_bHasColorSort=false;
	// V4.2: no longer used
	// m_StateString=tr("Ready...");
	m_uNumSelected=0;
	deallocate();
	m_nMovedOrDeleted=0;
	m_bNeedsResort=false;
	enableAutoLoad(false);
	m_bIsNetSort=false;
	m_bIsSimilarSort=false;

	// set empty search options
	m_Worker.useSearchOptions(false);
	m_bInetLoad = false;
	m_bInetLoadSimilar = false;
	m_bHarddiskLoadSimilar = false;

	emit setStatusWidget("-","-");
	emit setNumSelected("-");
}

void imageSortWidget::paintScanningScreen() 
{
	paintMessage(tr("Scanning directories, please wait... Press ESC to abort..."));
}

void imageSortWidget::paintLoadingScreen() 
{
	//paintMessage(tr("Loading images..."));
}

void imageSortWidget::paintStartUpScreen() 
{
	setPalette(QPalette(getBackgroundColor()));

	// image size
	int w = m_pSplashImage->width();
	int h = m_pSplashImage->height();

	// calculate image size
	double tmpW = (double)width() / w;
	double tmpH = (double)height() / h;

	double z = (tmpW < tmpH) ? tmpW : tmpH;
	z = (z > 1) ? 1.0 : z;

	double drawWidth = w * z;
	double drawHeight = h * z;

	// canvas center
	const int centerX = width() / 2; 
	const int centerY = height() / 2;

	// draw position
	int x = centerX - drawWidth / 2;
	int y = centerY - drawHeight / 2;

	QRect target(x, y, drawWidth, drawHeight);
	QRect source(0, 0, w, h);

	// draw image
	QPainter painter(this);
	painter.drawImage(target, *m_pSplashImage, source);
}
// TODOV4: obsolete?
void imageSortWidget::paintNoImageScreen() 
{	
//	if(hasOutSorted())
//		paintMessage(tr("No images to display...\nSelect \'Show All Images\' from the context menu or choose different folders..."));
//	else
//		paintMessage(tr("No images to display...\nChoose different folders..."));

}

void imageSortWidget::paintDirsDirtyScreen() 
{	
	QString msg;

	unsigned numDirs=m_Worker.cdirs().size();
	unsigned numFiles=m_Worker.cdirs().numFiles();
	strNumDirs.setNum(numDirs);
	strNumFiles.setNum(numFiles);

	
	if(!numDirs){
		msg=tr("No folder selected");
		paintMessage(msg);
	} else if(!numFiles) {
		if(numDirs==1)
			msg=tr("Selected folder does not contain image files.");
		else
			msg=tr("Selected folders do not contain image files.");
		paintMessage(msg);	
	} else {
		//m_pMsgStart->setVisible(true);
		m_pMsgWidget->setVisible(false);

		if(numDirs==1){
			if(numFiles==1)
				msg+=tr("Selected folder contains one image file.");
			else{
				msg+=tr("Selected folder contains ");
				msg+=strNumFiles;
				msg+=tr(" image files.");
			}
		}
		else{
			if(numFiles==1){
				msg+=tr("Selected folders contain one image file.");
			}
			else{
				msg+=tr("Selected folders contain ");
				msg+=strNumFiles;
				msg+=tr(" image files.");
			}
		}
#if defined(Q_WS_WIN)		
		msg+=tr("\n\nUse CTRL- or Shift-click to add more folders.");
#else // OS X
		msg+=tr("\n\nUse CMD- or Shift-click to add more folders.");
#endif
		m_pMsgStart->setVisible(true);
		m_pMsgStart->setText(msg);
	}
}

void imageSortWidget::resizeEvent(QResizeEvent *) 
{
	//loadImgButton->move((this->width()/2)-180,(this->height()/2)-75);
	const int w=width();
	const int h=height();
	m_pMsgWidget->move((w/2)-133,(h/2)-75);
	m_pMsgStart->move((w/2)-133,(h/2)-75);
	//m_progressBar->move((w/2)-133,(h/2)-75);
	m_GuiController.setCanvasSize(w,h,true);
}


void imageSortWidget::mousePressEvent(QMouseEvent *event)
{
	// V4.2: we don't process mouse events if not idle state
	// reason: we have to enable the sort widget even in non-idle states,
	// because the msg widget is disabled otherwise...
	if(!isIdle()){
		event->ignore();
		return;
	}	

	if(event->button()==Qt::LeftButton){
		// store modifiers 
		m_LeftMouseButtonKeys=event->modifiers();
		// store pressed position
		m_MousePressedLeftPos=event->pos();

		//if(m_SortMode==BY_COLOR){
		//	m_GuiController.onMouseDown(event->pos().x(),event->pos().y());
		//}
		//else{
		//	// if not in select rect mode (shift pressed), init mouse dragging 
		//	if(!leftMouseButtonShift()){
		//		//setCursor(Qt::ClosedHandCursor);
		//		m_MouseLastX=m_MousePressedLeftPos.x();
		//		m_MouseLastY=m_MousePressedLeftPos.y(); 
		//	}
		//}
		// if not in select rect mode (shift pressed), init mouse dragging 
		if(!leftMouseButtonShift())
			m_GuiController.onMouseDown(event->pos().x(),event->pos().y());
		event->accept();
     } else
		event->ignore();
}


void imageSortWidget::mouseMoveEvent(QMouseEvent *event) 
{
	// V4.2: we don't process mouse events if not idle state
	// reason: we have to enable the sort widget even in non-idle states,
	// because the msg widget is disabled otherwise...
	if(!isIdle()){
		event->ignore();
		return;
	}	
	
	const int xpos=event->pos().x();
	const int ypos=event->pos().y();
	m_GuiController.onMouseMove(xpos,ypos);

	if(event->buttons()==Qt::LeftButton){
		thumbnail *pItem=getPictureAt(event->pos());

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Implementierung der Drag-Funktion
		*/
		if(m_State == IDLE) {

			if( pItem /*&& !m_bKeyPressed*/ && !m_bScrolling ) { 
				QRect rect;
				rect.setRect(pItem->upperLeftX(),
							 pItem->upperLeftY(),
							 pItem->lowerRightX()-pItem->upperLeftX(),
							 pItem->lowerRightY()-pItem->upperLeftY());

				int x = rect.width()/6;
				int y = rect.height()/6;
				if(xpos > rect.left()+x && xpos < rect.right()-x && ypos > rect.top()+y && ypos < rect.bottom()-y) {
					// V4.2: only use arrow cursor...
					//setCursor(Qt::PointingHandCursor);
					m_bDragEnabled = true;
				} else {
					// V4.2: took out this stuff, we use the arrow cursor only
					// m_ControllerImages.size() /*&& (bColorSort)*/ ? setCursor(Qt::SizeAllCursor) : setCursor(Qt::ArrowCursor);
					m_bDragEnabled = false;
				}
			} 
			// V4.2: took out this stuff, we use the arrow cursor only
			// else {
			//	m_ControllerImages.size() /*&& (bColorSort)*/ ? setCursor(Qt::SizeAllCursor) : setCursor(Qt::ArrowCursor);
			//}
		}
		const bool bSelecting=leftMouseButtonShift();

		if(!bSelecting && !m_bDragEnabled){
			m_bScrolling = true;
		} else if(m_bDragEnabled) {
			thumbnail *tn=getPictureAt(m_MousePressedLeftPos);

			if(tn) {
				//setCursor(Qt::ClosedHandCursor);
				
				/*
				*	Hinzugefügt von Claudius Brämer und David Piegza
				*
				*   Ausführen der Drag-Funktion
				*/
				// TODOV4: check allocations...
				emit startDrag();
				QDrag *drag = new QDrag(this);
								
				ISMimeData *mimeData = new ISMimeData;
				mimeData->setText("ImageSorter thumbnail");
				// V5: this copis the thumbnail, which is ok...
				mimeData->setThumbnail(*tn);
				
				drag->setMimeData(mimeData);
				drag->setPixmap(QPixmap::fromImage(tn->m_Img).scaled(100,100,Qt::KeepAspectRatio));
				drag->setHotSpot(QPoint(50,50));
				Qt::DropAction dropAction = drag->exec();
				m_bDragEnabled = false;
				//setCursor(Qt::SizeAllCursor);
			}
		} else if(bSelecting){
			// create selection rect...
			int xs,ys,xe,ye;
			if(m_MousePressedLeftPos.x()<xpos){
				xs=m_MousePressedLeftPos.x();
				xe=xpos;
			}
			else{
				xs=xpos;
				xe=m_MousePressedLeftPos.x();
			}
			if(m_MousePressedLeftPos.y()<ypos){
				ys=m_MousePressedLeftPos.y();
				ye=ypos;
			}
			else{
				ys=ypos;
				ye=m_MousePressedLeftPos.y();
			}
			QRect r(xs,ys,xe-xs,ye-ys);
			m_SelectionRect=r;
	
		}

		event->accept();
		repaint();

		return;
	}
	else{
		m_bScrolling = false;

		if(m_bPreviewOnMouseOver&&isIdle()){
			// get item at position
			thumbnail *pItem=getPictureAt(event->pos());

			emit imageHovered(pItem);
		}
	}

	event->ignore();

}

void imageSortWidget::mouseReleaseEvent(QMouseEvent *event) 
{
	m_bScrolling = false;
	m_bDragEnabled = false;

	// V4.2: we don't process mouse events if not idle state
	// reason: we have to enable the sort widget even in non-idle states,
	// because the msg widget is disabled otherwise...
	
	if(event->button()==Qt::LeftButton){
		if(!isIdle())
			return;

		const unsigned n=numSelected();
		const bool bColorSort=(m_SortMode==BY_COLOR);
		const bool bSelecting=leftMouseButtonShift();

		thumbnail *pItem=(thumbnail*)(m_GuiController.onMouseUp(event->pos().x(),event->pos().y(),false,true).operator->());

		if(bSelecting){
			// if shift still pressed, use the selection rect, otherwise, forget it
			//if(event->modifiers()|Qt::ControlModifier)
			if(event->modifiers()|Qt::ShiftModifier)
				if(m_SelectionRect.isValid())
					selectionRectChanged(m_SelectionRect,!leftMouseButtonAlt());

			// if nothing (de)selected by the ops above, (de)select image clicked...
			if(n==numSelected()){
				if(pItem&&pItem->visible()){ 
					if(pItem->selected()){
						pItem->deSelect();
						assert(m_uNumSelected);
						--m_uNumSelected;
					}
					else{
						pItem->select();
						++m_uNumSelected;
					}

				}
				if(isIdle()){
					if(pItem)
						emit imageClicked(pItem);
				}
			}
			event->accept();

			if(n!=numSelected())
				emit selectionChanged();
		}
		else {
			// V2.02 de/select on left mouse btn
			// do not show preview border any longer
			if(isIdle()){
				if(pItem&&pItem->visible()){
					if(pItem->selected()){
						pItem->deSelect();
						--m_uNumSelected;
						emit imageClicked(NULL);
					}
					else{
						pItem->select();
						++m_uNumSelected;
						emit imageClicked(pItem);
					}
				}
				else
					emit imageClicked(NULL);

				if(n!=numSelected())
					emit selectionChanged();
			}
		}
	
	}
	else
		event->ignore();

	// reset modifiers
	resetLeftMouseButtonKeys();
	// make selection rect invalid
	m_SelectionRect.setRect(0,0,0,0);
	
	strSelected.setNum(m_uNumSelected);
	emit setNumSelected(strSelected);
	
	repaint();
}

void imageSortWidget::zoomIn() 
{
	//m_dZoomFactor *= 1.1;
	//
	//if (m_dZoomFactor > 200)
	//	m_dZoomFactor = 200;
	//
	//m_GuiController.onZoom(m_dZoomFactor);
	m_GuiController.onZoomIn();
}

void imageSortWidget::zoomOut() 
{
	//m_dZoomFactor/=1.1;

	//if(m_dZoomFactor<1)
	//	m_dZoomFactor=1;

	//m_GuiController.onZoom(m_dZoomFactor);
	m_GuiController.onZoomOut();

}


//void imageSortWidget::downloadSimilars()
//{
//	// don't show anything in the preview any longer...
//	emit imageClicked(NULL);
//
//	// TOOV3: put all this stuff in a member prepareLoading(sortMode)...
//
//	// if our worker is loading or sorting, abort it w/o emitting a signal
//	// but wait for the abortion to be completed
//	// note if the worker isWaiting(), this is NOP
//	// note if the worker isSaving(), this does not abort
//	// TODO: what happens if the worker isSaving() ???
//	// should be a wait for completion...
//	m_Worker.abortSilently(true);
//	// note this should work, but sometimes the v1 app crashed when 
//	// changing the folder while sorting...
//	// the reason is that the slot newColorSort() is called.
//	// since this is a 'queued slots' called across thread boundaries
//	// the slot call probably is still in the queue after the worker aborted...
//	// there is a workaround in newColorSort()...
//	if(m_Worker.isRunning()&&!m_Worker.isWaiting()){
//		m_WaitMutex.lock();
//		m_WaitCondition.wait(&m_WaitMutex);
//		m_WaitMutex.unlock();
//	}
//	// note this is called in the widgets thread, no need to lock here
//	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
//	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
//	// do we really need a list of thumbnails OR can we use a list
//	// of thumbnail* ? - the latter would be MUCH faster and less mem consuming...
//	while(it!=itEnd){
//		const thumbnail& rThumb=(const thumbnail&)(**it);
//		if(rThumb.selected()){
//			m_DynamicSearcher.addTarget(*(*it));
//		}
//		++it;
//	}
//
//	m_DynamicSearcher.resetCallbackCounter();
//
//	reset();
//	resetVisualization(true);
//	setSortMode(BY_SIMILARITY_LIST,true);
//	
//	// do we really pass the list of selected thumb BY VALUE???
//	m_Worker.loadFromNet(true,isYahooSearch());
//}
//
void imageSortWidget::setQuery(const QString& q)
{
	m_Worker.setQuery(q);
}

void imageSortWidget::download()
{
	// sepp: don't show anything in the preview any longer...
	emit imageClicked(NULL);

	// sepp: copied the following code from load(), this should be ok here, too
	// TOOV3: put all this stuff in a member prepareLoading(sortMode)...

	// if our worker is loading or sorting, abort it w/o emitting a signal
	// but wait for the abortion to be completed
	// note if the worker isWaiting(), this is NOP
	// note if the worker isSaving(), this does not abort
	// TODO: what happens if the worker isSaving() ???
	// should be a wait for completion...
	m_Worker.abortSilently(true);
	// note this should work, but sometimes the v1 app crashed when 
	// changing the folder while sorting...
	// the reason is that the slot newColorSort() is called.
	// since this is a 'queued slots' called across thread boundaries
	// the slot call probably is still in the queue after the worker aborted...
	// there is a workaround in newColorSort()...
	if(m_Worker.isRunning()&&!m_Worker.isWaiting()){
		m_WaitMutex.lock();
		m_WaitCondition.wait(&m_WaitMutex);
		m_WaitMutex.unlock();
	}

	reset();
	// since we want to repaint while loading, reset visualization
	resetVisualization();

	setSortMode(BY_NAME,true);
	m_Worker.loadFromNet(false,isYahooSearch());

}


void imageSortWidget::searchDisk(const searchOptions& options) {
	setState(PREPARE_HARDDISK_SEARCH);
	//setState(HARDDISK_SEARCH);
	hideMsgWidgets();
	m_bNoRepaint=true;
	repaint();
	m_bHardDiskSearch = true;
	m_Worker.setSearchOptions(options);
	m_pMsgWidget->reset();
	m_pMsgWidget->setText(tr("Please select a search folder from the explorer!"));
	QPushButton *pb = m_pMsgWidget->addButton(tr("Stop"));
	connect(pb, SIGNAL(clicked()), this, SLOT(searchDiskAborted()) );

	m_pMsgWidget->show();
}



void imageSortWidget::searchDiskAborted() 
{
	setState(IDLE);
	m_bNoRepaint=false;
	m_bHardDiskSearch=false;
	dirsChangeAborted();
}


/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Startet die Bildsuche mit Suchoptionen für ausgewählte 
*   Suchquelle (Yahoo, flickr, Hard disk).
*/
void imageSortWidget::download(searchOptions& options)
{
	m_bNoRepaint=false;
	if(options.m_Source != 0 && options.m_Keyword.isEmpty()) {
		// Dialog anzeigen
		m_pMsgWidget->reset();
		m_pMsgWidget->setText(tr("Please enter a keyword !"));
		QPushButton *pb = m_pMsgWidget->addButton(tr("Stop"));
		connect(pb, SIGNAL(clicked()), this, SLOT(hideMsgWidgets()));
		m_pMsgWidget->show();
		return;
	}
	// Setzen der Referenzbilder
	QLinkedList<thumbnail>& images = options.m_Thumbnails;
	QVector<QRgb>& colors = options.m_Colors;

	m_DynamicSearcher.clearTargets();


	// Referenzbilder für den ausgewählten visuellen Filter auslesen

	// note the new vsearch lib can search for images AND colors
	// we do this here, too, although our 'tabbed' user interface for this is not the best

	// add target images, if any (this may contain a sketch image)
	QLinkedList<thumbnail>::iterator it = images.begin();
	QLinkedList<thumbnail>::iterator itEnd = images.end();
	while(it!=itEnd){
		assert(it->isValid());
		// it->validate();
		m_DynamicSearcher.addTarget((*it));
		++it;
	}
	// add colors, if any
	QRgb rgb;
	for(unsigned i=0;i<colors.size();++i){
		rgb=colors.at(i);			
		m_DynamicSearcher.addTarget(qRed(rgb),qGreen(rgb),qBlue(rgb));
	}

	m_DynamicSearcher.resetCallbackCounter();

	// Setzen der Suchoptionen im Worker
	m_Worker.setSearchOptions(options);

	hideMsgWidgets();
	//m_progressBar->reset();
	//m_progressBar->setMaximum(m_Worker.numQueries());
	//m_progressBar->show();
	m_Worker.useSearchOptions(true);

	// Suchquelle Hard disk
	if(options.m_Source==0) {
		// go through load() as all other disk loading operations
		// note we may now load images from the folders we already have loaded,
		// but we want to filter now. thus, set the dirs dirty
		m_Worker.dirs().setDirty();

		//load();
		//emit imageClicked(NULL);

		//m_Worker.abortSilently(true);
		//if(m_Worker.isRunning()&&!m_Worker.isWaiting()){
		//	m_WaitMutex.lock();
		//	m_WaitCondition.wait(&m_WaitMutex);
		//	m_WaitMutex.unlock();
		//}

		//reset();
		//// since we want to repaint while loading, reset visualization
		//resetVisualization();

		//setSortMode(BY_COLOR,true);
		reset();
		resetVisualization(true);

		m_Worker.setQuery(options.m_Keyword);
		m_Worker.setSearchOptions(options);
		m_Worker.useSearchOptions(true);

		if(m_DynamicSearcher.numTargets()) {
			m_bHarddiskLoadSimilar = true;
			setSortMode(BY_SIMILARITY_LIST,true);
			m_Worker.search(true);
		} else {
			//setSortMode(BY_COLOR,true);
			setSortMode(BY_DISK_LOAD,true);
			m_Worker.search(false);
		}
		//qDebug() << "load from disk";
	} 
	//else {
		// V4.3: no longer any download from the net:
		// - yahoo api is turned off
		// - flickr api allows just 30 image sto be displayed
		// we may build in fotolia, if possible
	//	reset();
	//	resetVisualization(true);

	//	m_Worker.setQuery(options.m_Keyword);
	//	m_Worker.useSearchOptions(true);
	//	// Internet-Suchquelle setzen
	//	// V4.3: Yahoo image search service turned off :(
	//	//if(options.m_Source==1)
	//	//	setInetSearchService(YAHOO);
	//	//else if(options.m_Source==2)
	//	//	setInetSearchService(FLICKR);
	//	setInetSearchService(FLICKR);

	//	if(m_DynamicSearcher.numTargets()) {
	//		m_bInetLoadSimilar = true;
	//		setSortMode(BY_SIMILARITY_LIST,true);
	//		m_Worker.loadFromNet(true,isYahooSearch());
	//	} else {
	//		m_bInetLoad = true;
	//		setSortMode(BY_NET_LOAD,true);
	//		m_Worker.loadFromNet(false,isYahooSearch());
	//	}
	//}
}


//void imageSortWidget::searchSimilars()
//{
//	// sepp: don't show anything in the preview any longer...
//	emit imageClicked(NULL);
//
//	// sepp: copied the following code from load(), this should be ok here, too
//	// TOOV3: put all this stuff in a member prepareLoading(sortMode)...
//
//	// if our worker is loading or sorting, abort it w/o emitting a signal
//	// but wait for the abortion to be completed
//	// note if the worker isWaiting(), this is NOP
//	// note if the worker isSaving(), this does not abort
//	// TODO: what happens if the worker isSaving() ???
//	// should be a wait for completion...
//	m_Worker.abortSilently(true);
//	// note this should work, but sometimes the v1 app crashed when 
//	// changing the folder while sorting...
//	// the reason is that the slot newColorSort() is called.
//	// since this is a 'queued slots' called across thread boundaries
//	// the slot call probably is still in the queue after the worker aborted...
//	// there is a workaround in newColorSort()...
//	if(m_Worker.isRunning()&&!m_Worker.isWaiting()){
//		m_WaitMutex.lock();
//		m_WaitCondition.wait(&m_WaitMutex);
//		m_WaitMutex.unlock();
//	}
//
//	// note this is called in the widgets thread, no need to lock here
//	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
//	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
//	// do we really need a list of thumbnails OR can we use a list
//	// of thumbnail* ? - the latter would be MUCH faster and less mem consuming...
//	while(it!=itEnd){
//		const thumbnail& rThumb=(const thumbnail&)(**it);
//		if(rThumb.selected()){
//			m_DynamicSearcher.addTarget(**it);
//		}
//		++it;
//	}
//
//	// m_SimilarItems.resetInsertCalls();
//
//	reset();
//	// since we want to repaint while loading, reset visualization
//	resetVisualization();
//
//	setSortMode(BY_SIMILARITY_LIST,true);
//
//	m_Worker.load(true);
//}
//
//
void imageSortWidget::resetZoom(bool bForceRedraw)
{
	m_dZoomFactor=1; 

	m_GuiController.resetZoom(bForceRedraw);
}

void imageSortWidget::resetVisualization(bool bForceRedraw)
{
	m_GuiController.reset(bForceRedraw);

	//V4.2: arrow cursor only
	//setCursor(Qt::ArrowCursor);
}

#ifndef QT_NO_WHEELEVENT
void imageSortWidget::wheelEvent(QWheelEvent* pe) 
{
	int delta=pe->delta();
	if(delta){
		if (delta > 0)
			zoomIn();
		else 
			zoomOut();

		pe->accept();
	}
	else
		pe->ignore();

}
#endif



// this is a slot called via a queued connection signal from m_Worker
// each time the worker wants a repaint of the widget
void imageSortWidget::repaintRequested()
{
	if(m_SortMode==BY_SIMILARITY_LIST){
		lock();
		copyDownloadedSimilarItems();
		unlock();
	}
	else if(m_SortMode==BY_NET_LOAD){
		m_GuiController.calculate();
	}

	// note numThumbs() locks
	const unsigned size=numThumbs();

	if(!size)
		return;

	unsigned i,x,y;
	
	//m_progressBar->hide();
	//if(!m_progressBar->value()) {
	//	if(m_bInetLoad)
	//		m_progressBar->setMaximum(m_Worker.numQueries());
	//	else if(m_bInetLoadSimilar)
	//		m_progressBar->setMaximum(m_Worker.numSimilarQueries());
	//	else
	//		m_progressBar->setMaximum(m_Worker.cdirs().numFiles());
	//}
	//if(!m_bInetLoadSimilar && !m_bHarddiskLoadSimilar) {
	//	m_progressBar->setValue(size);
	//	m_progressBar->setText(QString("<b>Loading image %1 of %2...</b>").arg(size).arg(m_progressBar->maximum()));
	//}
	
	repaint();
}


void imageSortWidget::updateProgressBar(unsigned int size) {
	//if(m_progressBar->maximum()) {
	//	m_progressBar->setValue(size);
	//	m_progressBar->setText(QString("<b>Scanning image %1 of %2...</b>").arg(size).arg(m_progressBar->maximum()));
	//}
}


void imageSortWidget::copyDownloadedSimilarItems()
{
	allocate(m_DynamicSearcher.dynamicSearchResults().size());
	std::multimap<float,refPtr<controllerImage> >::iterator it=m_DynamicSearcher.dynamicSearchResults().begin();
	std::multimap<float,refPtr<controllerImage> >::iterator itEnd=m_DynamicSearcher.dynamicSearchResults().end();
	while(it!=itEnd){
		m_ControllerImages.push_back((*it).second);
		//m_SortedByName.push_back((*it).second);
		//m_SortedByDate.push_back((*it).second);
		//m_SortedBySize.push_back((*it).second);
		++it;
	}

}


// slot called by the color sorted thread on each som callback
void imageSortWidget::newColorSort()
{
	// this is a workaround for the crash which sometimes happens
	// when a user changes the folder during sort...
	// what seems to happen is that this queued slot is called
	// after the worker has been aborted silently in imageSortMainWindow::folderChanged()
	// we will have a crash then in onDrawNewSort()
	if(m_SortMode!=BY_COLOR)
		return;

	// we got a color sort now
	m_bHasColorSort=true;

	repaint();

}

void imageSortWidget::setSortMode(sortMode mode,bool bForceRedraw)
{
	if(m_SortMode==mode){
		if(m_SortMode==BY_COLOR&&needsResort()){
			m_Worker.sort();
			m_bNeedsResort=false;
			return;
		}
		else if(bForceRedraw)
			repaint();

		return;
	}

	m_SortMode=mode;

	emit sortModeChanged();

	if(m_SortMode==BY_COLOR&&needsResort()){
		m_GuiController.setMode(SORTED,false);
		m_Worker.sort();
		m_bNeedsResort=false;
		// take care: if you don't return here, there will be either strange crashes
		// or a dead lock... 
		// this is because imageSortWorker emits a signal which calls slot onDrawNewSort() as well
		return;
	}
	else{
		switch(m_SortMode){
			case BY_NAME:
				// note this is the trick: we set the m_SortedByName vector to the sorter
				// for the color sort, this is of no effect, since the refPtrs<> are the same...
				m_GuiController.setMode(SORTER_INPUT,false);
				m_Sorter.setImages(&m_SortedByName);
				m_GuiController.calculate(bForceRedraw);
				break;
			case BY_DATE:
				m_GuiController.setMode(SORTER_INPUT,false);
				m_Sorter.setImages(&m_SortedByDate);
				m_GuiController.calculate(bForceRedraw);
				break;
			case BY_SIZE:
				m_GuiController.setMode(SORTER_INPUT,false);
				m_Sorter.setImages(&m_SortedBySize);
				m_GuiController.calculate(bForceRedraw);
				break;
			case BY_SIMILARITY_PYRAMID:
				m_GuiController.setMode(PYRAMID_DYNAMIC_SEARCH_RESULTS,bForceRedraw);
				break;
			case BY_SIMILARITY_LIST:
				m_GuiController.setMode(LIST_DYNAMIC_SEARCH_RESULTS,bForceRedraw);
				break;
			case BY_DISK_LOAD:
			case BY_NET_LOAD:
				m_Sorter.setImages(&m_ControllerImages);
				m_GuiController.setMode(SORTER_INPUT,false);
				break;
			case BY_COLOR:
				m_Sorter.setImages(&m_ControllerImages);
				m_GuiController.setMode(SORTED,bForceRedraw);
				break;

		}
	}

}

void imageSortWidget::abortLoadOrSort()
{
	hideMsgWidgets();
	
	if(isIdle())
		emit deSelectDirs();
	//	m_Worker.clearDirs();
	else
		m_Worker.abort();
}

void imageSortWidget::escPressed()
{
	if(!isPreparingHardDiskSearch())
		abortLoadOrSort();
	//else{
	//	stopSearchDisk();
	//}

}

bool imageSortWidget::load(bool disksearch)
{
	if(!m_Worker.cdirs().dirty())
		return false;

	emit imageClicked(NULL);

	// if our worker is loading or sorting, abort it w/o emitting a signal
	// but wait for the abortion to be completed
	// note if the worker isWaiting(), this is NOP
	// note if the worker isSaving(), this does not abort
	// TODO: what happens if the worker isSaving() ???
	// should be a wait for completion...
	m_Worker.abortSilently(true);
	// note this should work, but sometimes the v1 app crashed when 
	// changing the folder while sorting...
	// the reason is that the slot newColorSort() is called.
	// since this is a 'queued slots' called across thread boundaries
	// the slot call probably is still in the queue after the worker aborted...
	// there is a workaround in newColorSort()...
	if(m_Worker.isRunning()&&!m_Worker.isWaiting()){
		m_WaitMutex.lock();
		m_WaitCondition.wait(&m_WaitMutex);
		m_WaitMutex.unlock();
	}

	if(m_Worker.dirs().empty()){
		reset();
		repaint();
		assert(false);
		return true;
	}

	reset();
	// since we want to repaint while loading, reset visualization
	resetVisualization();

	setSortMode(BY_DISK_LOAD,true);

	//m_progressBar->reset();
	////m_progressBar->setMaximum(m_Worker.cdirs().numFiles());
	//m_progressBar->show();

	// start the loader thread
	// when the loading is completed or aborted, our loadCompleted() or loadAborted() slot is called
	m_Worker.load();

	// NOTE ever reached?
	return true;
}


void imageSortWidget::loadCompleted()
{
	//m_progressBar->hide();
	//m_progressBar->reset();
	m_bIsNetSort=false;
	m_bIsSimilarSort=false;

	_loadCompleted(true,false);


}

void imageSortWidget::_loadCompleted(bool bShowColorSort,bool bShowSimilarSort)
{

	if(m_bHardDiskSearch) {
		emit deSelectDirs();
		if(!hasImages()){
			setState(IDLE);
			m_pMsgWidget->reset();
			m_pMsgWidget->setText(tr("No images found !"));
			QPushButton *pb = m_pMsgWidget->addButton(tr("OK"));
			connect(pb, SIGNAL(clicked()), SLOT(stopSearchDisk()));
			m_pMsgWidget->show();
			return;
		}
		stopSearchDisk();
	}
	if(!hasImages()){
		setState(IDLE);
		hideMsgWidgets();
		return;
	}
	
	// was loading?
	// assert(m_Worker.loadMode());

	// note this is not really true,
	// but we'd like to refresh the file system in the tree view from time to time...
	// V3 BETA 3: took this out, users may refresh the file system by F5
	// emit refreshFileSystem();

	// note the following operations are quite fast,
	// thus there is no real need to do this in a separate thread...
	// we spare locks this way...

	// fill the simple sort arrays
	const unsigned nThumbs=numThumbs();
	unsigned i;
	for(i=0;i<nThumbs;++i){
		refPtr<controllerImage> rfp=m_ControllerImages[i];
		if(canDoNameSort()){
			m_SortedByName.push_back(rfp);
		}
		if(canDoDateSort()){
			m_SortedByDate.push_back(rfp);
		}
		if(canDoSizeSort()){
			m_SortedBySize.push_back(rfp);
		}
		//// note if we do a similarity search, the order in the name sort *is* the similarity search
		//// (due to the crude logic here, this has to be reworked...)
		//if(hasSimilaritySort())
		//	m_SortedBySimilarity.push_back(m_SortedByName[i]);

	}

	// do the simple sorts
	if(canDoNameSort())
		sortByName();
	if(canDoDateSort())
		sortByDate();
	if(canDoSizeSort())
		sortBySize();

	// do color sort if auto mode on
	// note if the color sort is valid (from loading) and we don't want to ignore that, 
	// the loaded color sort is shown
	// otherwise, the color sort is done and the thumbs db is saved
	if(bShowColorSort){
		sortByColor();
	}
	else{
		if(bShowSimilarSort){
			setSortMode(BY_SIMILARITY_LIST,true);
		}
		m_bNeedsResort=true;
		setState(IDLE);
	}

	if(m_bIsNetSort){
		strNumFiles.setNum(numThumbs());
		emit setStatusWidget("-", strNumFiles);
		emit setNumSelected("0");
	}
	else{
		strNumDirs.setNum(m_Worker.cdirs().size());
		strNumFiles.setNum(m_ControllerImages.size());
		emit setStatusWidget(strNumDirs, strNumFiles);
		m_uNumSelected=0;
		emit setNumSelected("0");
	}
	
	hideMsgWidgets();
}

void imageSortWidget::updateStatusBarRequested()
{
	lock();
	strNumDirs.setNum(m_Worker.cdirs().cleanDirs().size());
	strNumFiles.setNum(m_ControllerImages.size());
	unlock();
	emit setStatusWidget(strNumDirs, strNumFiles);
}

void imageSortWidget::updateStatusBar2Requested(unsigned dirs,unsigned files)
{
	strNumDirs.setNum(dirs);
	strNumFiles.setNum(files);
	emit setStatusWidget(strNumDirs, strNumFiles);
}

void imageSortWidget::resort()
{

	if(hasMovedOrDeleted())
		cleanupMovedOrDeleted();

	// we propably don't have any more images to sort...
	if(!numThumbs()){
		repaint();
		return;
	}

	m_bNeedsResort=true;

	if(m_SortMode==BY_COLOR){
		m_Worker.sort();
		m_bNeedsResort=false;
	}
	else
		repaint();
}

void imageSortWidget::sortByColor()
{
	// something to do?
	if(!hasImages())
		return;

	resetVisualization(false);
	setSortMode(BY_COLOR,false);

	// compute new color sort

	m_Worker.sort();

	return;
}

void imageSortWidget::selectionRectChanged(const QRect& selectionRect,bool bSelectMode)
{
	assert(selectionRect.isValid());

	m_uNumSelected=0;
	
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		if(rItem.visible()){
			if(rItem.drawnRect().intersects(selectionRect)){
				if(bSelectMode)
					rItem.select();
				else
					rItem.deSelect();
			}
		}
		// keep track of selected items
		if(rItem.selected())
			++m_uNumSelected;

		++it;
	}

	repaint();

}

void imageSortWidget::enablePreviewOnMouseOver(bool bEnable)
{
	m_bPreviewOnMouseOver=bEnable;
}

void imageSortWidget::enableResortOnMoveDelete(bool bEnable)
{
	m_bResortOnMoveDelete=bEnable;
}

void imageSortWidget::loadingStarted()
{
	setState(LOADING);
	// V4.2: msg widgets where shown while loading...
	hideMsgWidgets();
	repaint();
}

void imageSortWidget::loadAborted()
{

	m_bIsNetSort=false;
	m_bIsSimilarSort=false;
	// note the dirs are not loaded, thus (still) dirty
	assert(m_Worker.dirs().dirty());
	setState(DIRSDIRTY);
	reset();
	repaint();
}

void imageSortWidget::loadSimilarAborted()
{
	m_bIsNetSort=false;
	m_bIsSimilarSort=true;

	//undoSelection();
	
	emit deSelectDirs();

	// note aborting a similar load (from disk) is o.k.,
	// we simply stop further loading but take the images we have...
	// anyway, we now may have less images loaded than expected
	// if we call loadCompleted() now, we'll have gaps in the resulting color sort

	// do not automatically show the color sort when loading similar from the net
	_loadCompleted(false,true);
}

void imageSortWidget::loadSimilarCompleted()
{
	m_bIsNetSort=false;
	m_bIsSimilarSort=true;

	emit deSelectDirs();

	//undoSelection();
	// do not automatically show the color sort when loading similar from the net
	_loadCompleted(false,true);
}

void imageSortWidget::inetLoadAborted()
{
	emit deSelectDirs();

	m_bIsNetSort=true;
	m_bIsSimilarSort=false;
	// note aborting a load from the net is o.k.,
	// we simply stop further loading but take the images we have...
	// anyway, we now may have less images loaded than expected
	// if we call loadCompleted() now, we'll have gaps in the resulting color sort

	_loadCompleted(true,false);
}

void imageSortWidget::inetLoadFailed(QString errStr)
{
	emit deSelectDirs();

	m_bIsNetSort=false;
	m_bIsSimilarSort=false;
	QMessageBox::warning(this,tr("Internet search failed"),errStr);
	// already done in worker
	// reset();
	_loadCompleted(false,false);
}
void imageSortWidget::inetLoadCompleted()
{
	emit deSelectDirs();

	m_bIsNetSort=true;
	m_bIsSimilarSort=false;
	// NOTE this is a bad hack, really:
	// we tried to download N images, but probably there are only M(<N) results for our query
	// (the xml file delivered in downloadfile() tells the number of overall reults...)
	// in that case, we'll have too much free space in the color sort view
	// the bad hack for now is to behave like the loading was ***ABORTED***.
	// in the final non beta release we'll shall have a better non-anuj logic here:
	// 1. issue the first yahoo request
	// 2. read the total available results
	// 2a. if bigger than the requested amount, allocate the requested amount
	// 2b. if smaller than the requested amount, allocate the total available amount
	// 3. Download
	// 4. check amount we have in the end and probably reallocate....

	_loadCompleted(true,false);
}

void imageSortWidget::inetLoadSimilarAborted()
{
	emit deSelectDirs();

	m_bIsNetSort=true;
	m_bIsSimilarSort=true;
	// note aborting a load from the net is o.k.,
	// we simply stop further loading but take the images we have...
	// anyway, we now may have less images loaded than expected
	// if we call loadCompleted() now, we'll have gaps in the resulting color sort

	// do not automatically show the color sort when loading similar from the net
	_loadCompleted(false,true);

	// update gui sort button
	emit sortModeChanged();

}

void imageSortWidget::inetLoadSimilarCompleted()
{
	emit deSelectDirs();

	m_bIsNetSort=true;
	m_bIsSimilarSort=true;
	// do not automatically show the color sort when loading similar from the net
	_loadCompleted(false,true);

	// update gui sort buttons...
	emit sortModeChanged();

}

void imageSortWidget::colorSortStarted()
{
	m_bNoRepaint=true;
	setState(SORTING);
}

void imageSortWidget::colorSortCompleted()
{
	m_bNoRepaint=false;
	// show last color sort
	newColorSort();


	setState(IDLE);
}

void imageSortWidget::colorSortAborted()
{
	m_bNoRepaint=false;
	// show last color sort
	newColorSort();


	setState(IDLE);
}


void imageSortWidget::setState(state s)
{
	m_State=s;
	//V 4.2: no longer used
	/*	
	switch(m_State){
		case CHANGING_DIRS:
			m_StateString=tr("Scanning directories... Press ESC to abort...");
			break;
		case LOADING:
			// note numThumbs() grows while loading
			m_StateString=tr("loading ");
			m_StateString+=QString::number(numThumbsExpected());
			m_StateString+=tr(" images... Press ESC to abort...");
			break;
		case SORTING:
			m_StateString=tr("sorting ");
			m_StateString+=QString::number(numThumbs());
			m_StateString+=tr(" images by color...");
			m_StateString+=tr(" Press ESC to abort...");
			break;
		case DIRSDIRTY:
			m_StateString=tr("Double click main area or press F5 to load...");
			break;
		case IDLE:
		default:
			m_StateString=tr("Ready...");
			break;

	}
	 */
	emit stateChanged();
}

void imageSortWidget::createContextMenu()
{
	m_pCopySelected=new QAction(tr("Copy selected images"),this);
	connect(m_pCopySelected,SIGNAL(triggered()),this,SLOT(copySelected()));
	m_ContextMenu.addAction(m_pCopySelected);

	m_pMoveSelected=new QAction(tr("Move selected images"),this);
	connect(m_pMoveSelected,SIGNAL(triggered()),this,SLOT(moveSelected()));
	m_ContextMenu.addAction(m_pMoveSelected);

	m_pDeleteSelected=new QAction(tr("Delete selected images"),this);
	connect(m_pDeleteSelected,SIGNAL(triggered()),this,SLOT(deleteSelected()));
	m_ContextMenu.addAction(m_pDeleteSelected);

	m_ContextMenu.addSeparator();

	m_pResortAction=new QAction(tr("Resort"),this);
	connect(m_pResortAction,SIGNAL(triggered()),this,SLOT(resort()));
	m_ContextMenu.addAction(m_pResortAction);

	m_ContextMenu.addSeparator();

	m_pOpenSelected=new QAction(tr("Open selected images"),this);
	connect(m_pOpenSelected,SIGNAL(triggered()),this,SLOT(openSelected()));
	m_ContextMenu.addAction(m_pOpenSelected);

	m_pCopySelectedToClipboard=new QAction(tr("Copy selected image to clipboard"),this);
	connect(m_pCopySelectedToClipboard,SIGNAL(triggered()),this,SLOT(copySelectedToClipboard()));
	m_ContextMenu.addAction(m_pCopySelectedToClipboard);

	m_ContextMenu.addSeparator();

	m_pSelectAll=new QAction(tr("Select all"),this);
	connect(m_pSelectAll,SIGNAL(triggered()),this,SLOT(selectAll()));
	m_ContextMenu.addAction(m_pSelectAll);

	m_pUndoSelection=new QAction(tr("Clear selection"),this);
	connect(m_pUndoSelection,SIGNAL(triggered()),this,SLOT(undoSelection()));
	m_ContextMenu.addAction(m_pUndoSelection);

	m_pInvertSelection=new QAction(tr("Invert selection"),this);
	connect(m_pInvertSelection,SIGNAL(triggered()),this,SLOT(invertSelection()));
	m_ContextMenu.addAction(m_pInvertSelection);
}

void imageSortWidget::undoSelection()
{
	if(hasSelection()){
		std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
		while(it!=itEnd){
			((thumbnail&)(**it)).deSelect();
			++it;
		}
		m_uNumSelected=0;
		//emit selectionChanged();
		emit setNumSelected("0");
		repaint();
	}
}

void imageSortWidget::openSelected()
{
	if(hasSelection()){
		// note ask before opening more than 10 image
		if(!m_bNoWarnings&&numSelected()>10){
			QString qst=QString(tr("Do you really want to open %1 images?")).arg(numSelected());
			if(QMessageBox::question(this,
								  tr("Confirm open"),	
								  qst,
								  QMessageBox::Yes,
								  QMessageBox::No)!=QMessageBox::Yes)
				return;

		}
		std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
		QByteArray filestr,dirstr;
		while(it!=itEnd){
			const thumbnail& rItem=(const thumbnail&)(**it);
			if(rItem.selected()){
				QUrl toOpen;
				if(rItem.isLoadedFromNet()){
					if(rItem.isFromYahoo())
						// new V4: open the web page, not the image...
						toOpen.setUrl(rItem.m_YahooURLs.m_RefererUrl);
					else
						toOpen.setUrl(rItem.m_flickrURLs.clickURL());

				}
				else
					//toOpen=QUrl::fromLocalFile(rItem.m_FileInfo.absoluteFilePath());
					toOpen=QUrl::fromLocalFile(rItem.m_FileName);

				// note if this is a net url, it takes much time on xp for opening the browser.
				// during this time, the gui is blocked. this is bad. we have to do this in a separate thread, i think.
				// this thread will delete itself when finished:
				desktopServiceOpenUrlThread* pFireAndForget=new desktopServiceOpenUrlThread(toOpen);
				pFireAndForget->start();
			}
			++it;
		}
	}
}

void imageSortWidget::copySelectedToClipboard()
{
	if(numSelected()==1){
		bool bDone=false;
		QClipboard *clipboard = QApplication::clipboard();
		if(clipboard){
			std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
			std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();

			while(it!=itEnd){
				const thumbnail& rItem=(const thumbnail&)(**it);
				if(rItem.selected()){
					// note don't copy thumb, but original
					//QImage img(rItem.m_FileInfo.absoluteFilePath());
					QImage img(rItem.m_FileName);
					if(!img.isNull()){
						clipboard->setImage(img);
						bDone=true;
					}
					if(!bDone)
						QMessageBox::information(this,tr("Clipboard Operation failed"),tr("Could not copy image to clipboard"));
					
					return;
				}
				++it;
			}
		}
	}
}

void imageSortWidget::deleteSelected()
{
	if(hasSelection()){
		// TODOV1.1
		// how to translate the standard buttons text ???
		if(!m_bNoWarnings){
			if(QMessageBox::question(this,
								  tr("Confirm Deletion"),	
								  tr("Do you really want do delete the selected files?\nNote: The files will not be moved to the recycle bin..."),
								  QMessageBox::Yes,
								  QMessageBox::No)!=QMessageBox::Yes)
				return;
		}

		emit startWorking();

		std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
		QString filestr;
		QSet<QString> srcDirs;
		QFileInfo finfo;
		unsigned numDeleted=0;
		while(it!=itEnd){
			thumbnail& rItem=(thumbnail&)(**it);
			if(rItem.selected()){
				finfo.setFile(rItem.m_FileName);
				filestr=finfo.fileName();
				QDir dir=finfo.absoluteDir();
				if(dir.remove(filestr)){
					rItem.setMovedOrDeleted();
					++numDeleted;
					srcDirs.insert(rItem.m_FileName);
				}
			}
			++it;
		}

		emit stoppedWorking();

		if(numSelected()!=numDeleted){
			QString str=tr("Deletion failed for ");
			str+=QString::number(numSelected()-numDeleted);
			str+=tr(" files...");
			QMessageBox::warning(this,
								 tr("Deletion failed"),
								 str,
								 QMessageBox::Ok,0);
		}

		QSet<QString>::const_iterator it2=srcDirs.constBegin();
		QSet<QString>::const_iterator it2End=srcDirs.constEnd();
		while(it2!=it2End){
			emit startWorking();
			m_Worker.dirs().updateDir(*it2);
			emit stoppedWorking();
			emit refreshFileSystemPath(*it2);
			++it2;
		}

		assert(m_uNumSelected>=numDeleted);
		m_uNumSelected-=numDeleted;
		m_nMovedOrDeleted+=numDeleted;
		if(numDeleted)
			emit imageClicked(NULL);

		if(numDeleted&&m_bResortOnMoveDelete)
			resort();
		else{			
			// note calling setState() updates the main window gui
			// since we may have no longer any selection, we need to do this,
			// otherwise the 'Find Similar' Buttons are still active...
			// (not needed if resort(), because this is done by the worker, setSate is called here
			// automatically...
			setState(IDLE);
			repaint();
		}

	}
}
void imageSortWidget::cleanupMovedOrDeleted()
{
	if(hasMovedOrDeleted()){
		// clear all vectors but one
		m_ControllerImages.clear();
		m_SortedByDate.clear();
		m_SortedBySize.clear();

		// push the refPtrs which are not moved or deleted to one of the vectors above
		std::vector<refPtr<controllerImage> >::iterator it=m_SortedByName.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_SortedByName.end();
		unsigned numDeleted=0;
		int index;
		while(it!=itEnd){
			const thumbnail& rItem=(thumbnail&)(**it);
			if(!rItem.isMovedOrDeleted())
				m_ControllerImages.push_back(*it);
			++it;
		}

		// clear the controllerImages
		m_SortedByName.clear();

		// copy from the vector filled
		m_SortedByDate=m_ControllerImages;
		m_SortedBySize=m_ControllerImages;
		m_SortedByName=m_ControllerImages;

		if(canDoSizeSort())
			sortBySize();
		if(canDoDateSort())
			sortByDate();
		// no name sort needed, still sorted

		// TODOV5: what if we have deleted from the similar sort???
		std::multimap<float,refPtr<controllerImage> >::iterator it2=m_DynamicSearcher.dynamicSearchResults().begin();
		std::multimap<float,refPtr<controllerImage> >::iterator it2End=m_DynamicSearcher.dynamicSearchResults().end();
		while(it2!=it2End){
			const thumbnail& rItem=(thumbnail&)(*((*it2).second));
			if(rItem.isMovedOrDeleted())
				m_DynamicSearcher.dynamicSearchResults().erase(it2);
			// does erase() invalidates the iterator???
			++it2;
		}
		// we have made a reallocation:
		m_uNumThumbsExpected=numThumbs();
	}
	m_nMovedOrDeleted=0;
}

void imageSortWidget::moveSelected()
{
	if(hasSelection()){
		// we choose the first of our working dirs here as default
		QString destDir=QFileDialog::getExistingDirectory(this,tr("Select Destination Directory"),m_Worker.cdirs().firstDir());

		// canceled?
		if(destDir.isNull())
			return;

		// moving inside the dirs we have currently loaded is not impossible,
		// but we need to update the sort cache then
		// thus, for now, we forbid this:
		if(m_Worker.cdirs().contains(destDir)){
			QMessageBox::critical(this,
								 tr("Error"),
								 tr("Cannot move to directories currently loaded"),
								 QMessageBox::Ok,0);
			return;
		}

		emit startWorking();

		std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
		QString oldfilestr;
		QString newfilestr;
		QString uniqueFileNameStr;
		unsigned numMoved=0;
		QSet<QString> srcDirs;
		QFileInfo finfo;
		while(it!=itEnd){
			thumbnail& rItem=(thumbnail&)(**it);
			if(rItem.selected()){
				//oldfilestr=rItem.m_FileInfo.absoluteFilePath();
				oldfilestr=rItem.m_FileName;
				// note: Qt will change '/' to the separator required on the 
				// platform used...
				finfo.setFile(rItem.m_FileName);
				if(uniqueFileName(&uniqueFileNameStr,finfo,destDir)){
					newfilestr=destDir+"/"+uniqueFileNameStr;
					assert(oldfilestr!=newfilestr);
					if(finfo.absoluteDir().rename(oldfilestr,newfilestr)){
						rItem.setMovedOrDeleted();
						++numMoved;
						srcDirs.insert(rItem.m_FileName);
					}

				}

			}
			++it;
		}
		
		emit stoppedWorking();

		if(numSelected()!=numMoved){
			QString str=tr("Moving failed for ");
			str+=QString::number(numSelected()-numMoved);
			str+=tr(" files...\nPossible reason: destination files exist...");
			QMessageBox::warning(this,
								 tr("Moving failed"),
								 str,
								 QMessageBox::Ok,0);
		}

		emit startWorking();
		QSet<QString>::const_iterator it2=srcDirs.constBegin();
		QSet<QString>::const_iterator it2End=srcDirs.constEnd();
		while(it2!=it2End){
			m_Worker.dirs().updateDir(*it2);
			emit refreshFileSystemPath(*it2);
			++it2;
		}
		emit stoppedWorking();

		assert(numSelected()>=numMoved);
		m_uNumSelected-=numMoved;
		m_nMovedOrDeleted+=numMoved;
		if(numMoved){
			emit refreshFileSystemPath(destDir);
			emit imageClicked(NULL);
		}

		if(numMoved&&m_bResortOnMoveDelete)
			resort();
		else{
			// see comment in deleteSelected()
			setState(IDLE);
			repaint();
		}
	}
}

void imageSortWidget::copySelected()
{
	if(hasSelection()){
		// we choose the first of our working dirs here as default
		QString destDir=QFileDialog::getExistingDirectory(this,tr("Select Destination Directory"),m_Worker.cdirs().firstDir());

		// canceled?
		if(destDir.isNull())
			return;

		// what shall we do here if a user has chosen any of the dirs we have
		// currently loaded? this is not easy: we could just forbid this and thus return
		// but since we may have loaded more than one dir, allowing this might be useful, anyway.

		// thus, we may the following:
		// if an image shall be copied to a dest dir which does not belong to the dirs loaded, 
		// we copy it
		// if an image shall be copied to a dest dir which does belong to the dirs loaded, but 
		// does not reside in that dir, we copy it and set the dest dir dirty
		// the problem is: we have to reload and resort then...
		// if an image shall be copied to a dest dir which does belong to the dirs loaded, but 
		// is already from that dir, we do nothing (this will result in an error message)

		// anyway, for now, we forbid this:
		if(m_Worker.cdirs().contains(destDir)){
			QMessageBox::critical(this,
								 tr("Error"),
								 tr("Cannot copy to directories currently loaded"),
								 QMessageBox::Ok,0);
			return;
		}


		std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
		std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
		QString oldfilestr;
		QString uniqueNewFileStr;
		QFileInfo finfo;
		unsigned numCopied=0;
		while(it!=itEnd){
			thumbnail& rItem=(thumbnail&)(**it);
			if(rItem.selected()){
				oldfilestr=rItem.m_FileName;
				finfo.setFile(oldfilestr);
				// note: Qt will change '/' to the separator required on the 
				// platform used...
				if(uniqueFileName(&uniqueNewFileStr,finfo,destDir))
					if(QFile::copy(oldfilestr,destDir+"/"+uniqueNewFileStr))
						++numCopied;
			}
			++it;
		}
		if(numSelected()!=numCopied){
			QString str=tr("Copying failed for ");
			str+=QString::number(numSelected()-numCopied);
			str+=tr(" files...");
			QMessageBox::warning(this,
								 tr("Copy failed"),
								 str,
								 QMessageBox::Ok,0);
		}

		assert(numSelected()>=numCopied);

		if(numCopied)
			emit refreshFileSystemPath(destDir);
	}
}
void imageSortWidget::invertSelection()
{
	m_uNumSelected=0;
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		if(rItem.selected())
			rItem.deSelect();
		else
			rItem.select();

		if(rItem.selected())
			++m_uNumSelected;

		++it;
	}
	strSelected.setNum(m_uNumSelected);
	emit setNumSelected(strSelected);
	// emit selectionChanged();
	repaint();
}

void imageSortWidget::selectAll()
{
	m_uNumSelected=0;
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		rItem.select();
		++it;
	}
	m_uNumSelected=m_ControllerImages.size();
	// emit selectionChanged();
	strSelected.setNum(m_uNumSelected);
	emit setNumSelected(strSelected);
	repaint();
}

void imageSortWidget::contextMenuEvent(QContextMenuEvent *p)
{
	const unsigned nThumbs=numThumbs();

	// note we may have all images sorted out, allow the context menu in this case...
	if((!nThumbs)||p->reason()!=QContextMenuEvent::Mouse){
		p->ignore();
		return;
	}

	// m_pResortAction->setEnabled(!m_bResortOnMoveDelete&&hasMovedOrDeleted());
	m_pResortAction->setEnabled(true);
	
	m_pOpenSelected->setEnabled(hasSelection());
	
	m_pCopySelectedToClipboard->setEnabled(numSelected()==1&&!m_bIsNetSort);

	m_pDeleteSelected->setEnabled(hasSelection()&&!m_bIsNetSort);

	m_pMoveSelected->setEnabled(hasSelection()&&!m_bIsNetSort);

	m_pCopySelected->setEnabled(hasSelection()&&!m_bIsNetSort);
	m_pUndoSelection->setEnabled(hasSelection());

	m_ContextMenu.popup(p->globalPos());
	p->accept();
}

void imageSortWidget::splashWindowShown()
{
	m_bShowSplash=false;
	repaint();
}

void imageSortWidget::undoPreSelection()
{
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();

	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		rItem.dePreSelect();
		++it;
	}
	repaint();
	return;
}
void imageSortWidget::undoPreDeSelection()
{
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();

	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		rItem.dePreDeSelect();
		++it;
	}
	repaint();
	return;
}

void imageSortWidget::selectPreSelected()
{
	m_uNumSelected=0;
	
	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();

	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		if(rItem.preSelected())
			rItem.select();
		if(rItem.selected())
			++m_uNumSelected;
		++it;
	}
	repaint();
	return;
}

void imageSortWidget::deSelectPreDeSelected()
{
	m_uNumSelected=0;

	std::vector<refPtr<controllerImage> >::iterator it=m_ControllerImages.begin();
	std::vector<refPtr<controllerImage> >::iterator itEnd=m_ControllerImages.end();
	while(it!=itEnd){
		thumbnail& rItem=(thumbnail&)(**it);
		if(rItem.preDeSelected())
			rItem.deSelect();
		if(rItem.selected())
			++m_uNumSelected;
		++it;
	}
	repaint();
	return;
}

void imageSortWidget::keyPressEvent(QKeyEvent* p)
{
	if(p->key()==Qt::Key_Escape){
		escPressed();
		//abortLoadOrSort();
		p->accept();
		return;
	}

	//if(p->key()==Qt::Key_F5){
	//	if(!load())
	//		emit refreshFileSystem();
	//	p->accept();
	//	return;
	//}

	//if(p->key()==Qt::Key_F8){
	//	reportSelected();
	//	p->accept();
	//	return;
	//}

	Qt::KeyboardModifiers mod=p->modifiers();

	// bitwise OR doesnt make sense, it would return true always
	//if((mod|Qt::ShiftModifier)||(mod|Qt::ControlModifier)){
	if((mod&Qt::ShiftModifier)||(mod&Qt::ControlModifier)){
		//setCursor(Qt::ArrowCursor);
		m_bDragEnabled = false;
		//m_bKeyPressed = true;
		p->accept();
		return;
	}
	p->ignore();
}

void imageSortWidget::keyReleaseEvent(QKeyEvent* p)
{
	// V2.02 display arrow cursor as standard cursor
	// display closed hand cursor when dragging
	// setCursor(Qt::OpenHandCursor);
	// NEW: V4 display SizeAllCursor as standard cursor
	//setCursor(Qt::SizeAllCursor);
	//m_bKeyPressed = false;
	p->ignore();
}

void imageSortWidget::changeDirs(const QSet<QString>& dirsToRemove,const QSet<QString>& dirsToAdd)
{
#pragma message("test if we need to lock here...")
	
	// note this runs in this thread, not in the worker
	// may take some time if dir is big...
	setState(CHANGING_DIRS);
	repaint();

	m_Worker.changeDirs(dirsToRemove,dirsToAdd,m_bEnableRecursiveLoad);
	//loadImgButton->setVisible(true);
	
	//m_pMsgWidget->setVisible(true);
	emit setStatusWidget("-", "-");
	emit setNumSelected("-");
	// V4.2 use arrow cursor only
	//setCursor(Qt::ArrowCursor);
}

void imageSortWidget::dirsChanged()
{
	if(!m_bHardDiskSearch) {
		setState(IDLE);
		if(m_bDoAutoLoad){
			load();
			enableAutoLoad(false);
			return;
		}

		if(m_Worker.cdirs().dirty()) {
			setState(DIRSDIRTY);
			m_pMsgStart->show();
			emit setStatusWidget("-", "-");
			emit setNumSelected("-");
		} else{
			setState(IDLE);
			// tell the main window to highlight the correct sort button
			emit sortModeChanged();
			//loadImgButton->setVisible(false);

			if(!m_bInetLoad && !m_bInetLoadSimilar){
				strNumDirs.setNum(m_Worker.cdirs().size());
				strNumFiles.setNum(m_ControllerImages.size());
				emit setStatusWidget(strNumDirs, strNumFiles);
				m_uNumSelected=0;
				emit setNumSelected("0");
				strNumFiles.setNum(m_Worker.cdirs().numFiles());
			}

			hideMsgWidgets();
		}
	} else {
		if(m_Worker.cdirs().size()) {
			//// THIS is a bad hack: if we preparefor searching disks, we do not want
			//// a repaint of the current image set...
			//if(m_State!=PREPARE_HARDDISK_SEARCH)
			
			//setState(IDLE);
			// V4.2: we need a new state here...
			setState(PREPARE_HARDDISK_SEARCH);
			hideMsgWidgets();

			if(!m_Worker.cdirs().numFiles()) {
				m_pMsgWidget->reset();
				m_pMsgWidget->setText(tr("Selected folder does not contain any images.\nPlease select another search folder."));
				QPushButton *pb = m_pMsgWidget->addButton(tr("Stop"));
				//connect(pb, SIGNAL(clicked()), this, SLOT(searchFolder()));
				//QPushButton *pb = m_pMsgWidget->addButton("Search in folder");
				connect(pb, SIGNAL(clicked()), this, SLOT(searchDiskAborted()));
				m_pMsgWidget->show();
			} else {
				m_pMsgWidget->reset();
#if defined(Q_WS_WIN)
				m_pMsgWidget->setText(QString(tr("Folder contains %1 images.\n\nUse CTRL- or Shift-click to add more folders.")).arg(m_Worker.cdirs().numFiles()));
#else
				m_pMsgWidget->setText(QString(tr("Folder contains %1 images.\n\nUse CMD- or Shift-click to add more folders.")).arg(m_Worker.cdirs().numFiles()));
#endif
				QPushButton *pb2 = m_pMsgWidget->addButton(tr("Search images"));
				pb2->setMinimumWidth(150);
				connect(pb2, SIGNAL(clicked()), this, SLOT(searchFolder()));
				QPushButton *pb1 = m_pMsgWidget->addButton(tr("Stop"));
				connect(pb1, SIGNAL(clicked()), this, SLOT(searchDiskAborted()));
				m_pMsgWidget->show();
			}
		}
		//download(m_Worker.getSearchOptions());
	}
	// note paintEvent() will print a message if the dirs are dirty now
	// thus:
	repaint();
}


void imageSortWidget::searchFolder() {
	setState(HARDDISK_SEARCH);
	hideMsgWidgets();
#if defined(__GNUC__)
	searchOptions opts=m_Worker.getSearchOptions();
	download(opts);
#else
	download(m_Worker.getSearchOptions());
#endif
}


void imageSortWidget::dirsChangeAborted()
{
	hideMsgWidgets();
	// tell the explorer pane to select the last loaded dirs
	if(m_bEnableRecursiveLoad)
		emit lastLoadedDirs(m_Worker.dirs().topDirs());
	else
		emit lastLoadedDirs(m_Worker.dirs().cleanDirs());

	setState(IDLE);
	// tell the main window to highlight the correct sort button
	emit sortModeChanged();

	// note paintEvent() will print a message if the dirs are dirty now
	// thus:
	repaint();
}



void imageSortWidget::dirCreatedByWorker(const QString& dir)
{
	// should not be called anymore:

	assert(false);
	emit refreshFileSystemPath(dir);
	emit dirCreated(dir);

}


void imageSortWidget::setSearchOptions(const QString& query,unsigned nQueries,unsigned nSimilarQueries,unsigned nShowSimilars,unsigned numNetSearchThreads)
{
	m_Worker.setQuery(query);
	m_Worker.setNumQueries(nQueries);
	// do not have less similarity queries than queries
	m_Worker.setNumSimilarQueries(qMax(nQueries,nSimilarQueries));
	// do not have more similarity queries shown than we have actually
	m_Worker.setNumSimilarShown(qMin(nSimilarQueries,nShowSimilars));
	// do not have more similarity queries shown than we have actually
	m_Worker.setNumNetSearchThreads(numNetSearchThreads);
}

bool imageSortWidget::uniqueFileName(QString* pDstStr,const QFileInfo& finfo,const QDir& dir)
{
	if(!dir.exists(finfo.fileName())){
		*pDstStr=finfo.fileName();
		return true;
	}

	// test for 100 copies, no reason to run in endless loop here...
	for(int i=2;i<100;++i){
		QString str=finfo.baseName()+"_"+QString::number(i)+"."+finfo.completeSuffix();
		if(!dir.exists(str)){
			*pDstStr=str;
			return true;
		}
	}

	return false;
}

void imageSortWidget::setInetSearchService(int i)
{
	m_inetSearchService=(imageSortWidget::InetSearchService)i;
}

void imageSortWidget::disableWarnings(bool b)
{
	m_bNoWarnings=b;
}


void imageSortWidget::hideMsgWidgets() {
	m_pMsgWidget->hide();
	m_pMsgStart->hide();
	//m_progressBar->hide();
}


//void imageSortWidget::enableMsgWidgets() {
//	m_pMsgWidget->setEnabled(true);
//	m_pMsgStart->setEnabled(true);
//}

void imageSortWidget::stopSearchDisk() {
	//setState(IDLE);
	m_bHardDiskSearch = false;
	m_Worker.useSearchOptions(false);
	m_Worker.dirs().setDirty();
	hideMsgWidgets();
	

}

void imageSortWidget::repaintRequestedStaticCB(void *pUserData)
{
	assert(pUserData);
	((imageSortWidget*)(pUserData))->repaintRequestedBySortController();
}


void imageSortWidget::repaintRequestedBySortController()
{
	repaint();
}

