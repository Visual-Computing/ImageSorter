#include <limits>
#include "imagesortworker.h"
#include "imagesortwidget.h"
#include "imageio.h"
#include <float.h>
#include "synchttp.h"
#include <QDir>
#include "yahooimagesearchv1querydispatcher.h"
#include "yahooimgsearchv1xmlreader.h"
#include "flickrimgsearchquerydispatcher.h"
#include "flickrimgsearchxmlreader.h"
#include "urlthumbloaddispatcher.h"
#include <QtGlobal>
#include "exifthumbnailextractor.h"
#include "alphaoperators.h"
#include "thumbnailcalculator.h"
//#include <iostream>

imageSortWorker::imageSortWorker(QObject *pParent) : QThread(pParent)
{
	m_pImageSortWidget=NULL;
	m_bAbort=false;
	m_bRestart=false;
	m_bDestroy=false;
	m_bAbortSilently=false;
	m_bWaiting=false;
	m_cbInterval=5;
	m_RunMode=NOMODE;
	m_bMainWaits=false;
	m_LoadFailureImage=QImage(":/loadFailureImage");
	m_bUseSearchOptions=false;
	m_uNumNetSearchThreads=10;
}
void imageSortWorker::init(imageSortWidget* p)
{
	assert(p);
	m_pImageSortWidget=p;
	// bool bOk=m_ThumbsCache.init("thumbscache","ist","ist");
	//assert(bOk);
	// V5: this was way too much, redrawing each time an image is inserted floodes the diplay with redraw events.
	// we do this in the worker now...
	// we want to redraw each time the dynamicSearcher has inserted an images
	// m_pImageSortWidget->m_DynamicSearcher.registerSearchAdvancedCallback((isortisearch::pxCallback)repaintRequestedStaticCB,(void*)this,1,true);
}

imageSortWorker::~imageSortWorker()
{
	stop();
	m_Dirs.clear();
}

void imageSortWorker::stop()
{
	if(isRunning()){
		if(!isWaiting())
			abortSilently();

		while(!isWaiting())
			;

		m_Mutex.lock();
		m_bDestroy=true;
		m_WaitCondition.wakeOne();
		m_Mutex.unlock();

		wait();
	}

}
void imageSortWorker::load(bool bSimilars)
{
	if(!m_pImageSortWidget||m_Dirs.empty()){
		assert(false);
		return;
	}

	QMutexLocker(&(this->m_Mutex));
	m_bAbort=false;
	if(bSimilars)
		m_RunMode=LOAD_SIM_FROM_DISK;
	else
		m_RunMode=LOAD_DIRS;
	
	if(!isRunning()){
		start();
	} 
	else{
		m_bRestart=true;
		m_WaitCondition.wakeOne();
	}
	return;

}

/*
*	Hinzugefügt von Claudius Brämer und David Piegza
*
*   Methode zum Starten der Festplattensuche
*/
void imageSortWorker::search(bool bSimilars) {
	if(!m_pImageSortWidget||m_Dirs.empty()){
		assert(false);
		return;
	}

	m_Dirs.setDirty();
	
	load(bSimilars);
}

void imageSortWorker::changeDirs(const QSet<QString>& dirsToRemove,const QSet<QString>& dirsToAdd,bool bSubDirs)
{
	if(!m_pImageSortWidget){
		assert(false);
		return;
	}

	QMutexLocker(&(this->m_Mutex));
	m_bAbort=false;
	m_DirsToRemove.clear();
	m_DirsToRemove=dirsToRemove;
	m_DirsToAdd.clear();
	m_DirsToAdd=dirsToAdd;
	m_bSubDirs=bSubDirs;
	m_RunMode=CHANGE_DIRS;

	if(!isRunning()){
		start();
	} 
	else{
		m_bRestart=true;
		m_WaitCondition.wakeOne();
	}

	return;

}

//void imageSortWorker::clearDirs()
//{
//	if(!m_pImageSortWidget){
//		assert(false);
//		return;
//	}
//
//	QMutexLocker(&(this->m_Mutex));
//	m_bAbort=false;
//	m_RunMode=CLEAR_DIRS;
//
//	if(!isRunning()){
//		start();
//	} 
//	else{
//		m_bRestart=true;
//		m_WaitCondition.wakeOne();
//	}
//
//	return;
//
//}

void imageSortWorker::_change_dirs()
{
	// if we abort, we may restore the state
	imageSortDirs oldDirs(m_Dirs);
	
	QSet<QString>::iterator it=m_DirsToRemove.begin();
	QSet<QString>::const_iterator itEnd=m_DirsToRemove.constEnd();
	while(it!=itEnd){
		m_Dirs.removeDir(*it);
		++it;
	}
	bool bAbort=false;
	it=m_DirsToAdd.begin();
	itEnd=m_DirsToAdd.constEnd();
	while(it!=itEnd){
		m_Dirs.addDir(*it,m_bSubDirs,true,&m_bAbort,&(m_pImageSortWidget->m_Mutex));
		lock();
		bAbort=m_bAbort;
		unlock();
		if(bAbort)
			break;
		++it;
	}
	if(bAbort)
		m_Dirs=oldDirs;

	m_DirsToRemove.clear();
	m_DirsToAdd.clear();
	return;
}

void imageSortWorker::loadFromNet(bool bLoadSimilars,bool bFromYahoo)
{
	//We have to replace every space with + in query string, otherwise image search will fail
	m_Query.replace(QString(" "), QString("+"));

	QMutexLocker(&(this->m_Mutex));
	m_bAbort=false;
	m_bYahooImgSearch=bFromYahoo;

	if(bLoadSimilars)
		m_RunMode=LOAD_SIM_FROM_NET;
	else
		m_RunMode=LOAD_FROM_NET;

	if(!isRunning()){
		start();
	} 
	else{
		m_bRestart=true;
		m_WaitCondition.wakeOne();
	}

	return;

}

void imageSortWorker::sort(unsigned cbInterval)
{
	if(!m_pImageSortWidget){
		assert(false);
		return;
	}

	m_cbInterval=cbInterval;
	QMutexLocker(&(this->m_Mutex));
	m_RunMode=SORT;
	m_bAbort=false;

	if(!isRunning()){
		start();
	} 
	else{
		m_bRestart=true;
		m_WaitCondition.wakeOne();
	}

	return;

}



void imageSortWorker::run()
{
	if(!m_pImageSortWidget)
		return;
	
	// just an abbrev:
	imageSortWidget& sortWidget=*m_pImageSortWidget;

	while(true){
		// check if we are about to be destroyed (set by ctor)
		if(m_bDestroy){
			return;
		}
		bool bLoaded;
		switch(m_RunMode){
			case CHANGE_DIRS:
				_change_dirs();
				break;
			//case CLEAR_DIRS:
			//	m_Dirs.clear();
			//	break;
			case LOAD_DIRS:
				m_bLoadSimilars=false;
				_load_dirs();
				break;
			case SORT:
				_sort();
				break;
			case LOAD_FROM_NET:
				m_bLoadSimilars=false;
				_load_from_net();
				break;
			case LOAD_SIM_FROM_NET:
				m_bLoadSimilars=true;
				_load_from_net();
				break;
			case LOAD_SIM_FROM_DISK:
				m_bLoadSimilars=true;
				_load_dirs();
				break;
			default:
				assert(false);
				stop();
				return;
		}

		switch(m_RunMode){
			//case CLEAR_DIRS:
			case CHANGE_DIRS:
				if(m_bAbort){
					if(!m_bAbortSilently){
						emit dirsChangeAborted();
					}
					else{
						if(m_bMainWaits){
							m_pImageSortWidget->m_WaitCondition.wakeAll();
						}
					}
				}
				else
					emit dirsChanged();
				break;
			case LOAD_DIRS:
				if(m_bAbort){
					if(!m_bAbortSilently){
						 emit loadAborted();
					}
					else{
						if(m_bMainWaits){
							m_pImageSortWidget->m_WaitCondition.wakeAll();
						}
					}
				}
				else
					emit loadCompleted();
				break;
			case LOAD_FROM_NET:
				if(m_bHttpError){
					emit inetLoadFailed(m_HttpErrorString);
					m_bHttpError=false;
					m_HttpErrorString="";
				}
				else if(m_bAbort){
					if(!m_bAbortSilently){
						emit inetLoadAborted();
					}
					else{
						if(m_bMainWaits){
							m_pImageSortWidget->m_WaitCondition.wakeAll();
						}
					}
				}
				else
					emit inetLoadCompleted();
				break;
			case LOAD_SIM_FROM_DISK:
				if(m_bAbort){
					if(!m_bAbortSilently){
						// TODOV3 this leads to a slot which reallocates draw arrays, although this is not needed here...
						emit loadSimilarAborted();
					}
					else{
						if(m_bMainWaits){
							m_pImageSortWidget->m_WaitCondition.wakeAll();
						}
					}
				}
				else
					emit loadSimilarCompleted();
				break;
			case LOAD_SIM_FROM_NET:
				if(m_bHttpError){
					emit inetLoadFailed(m_HttpErrorString);
					m_bHttpError=false;
					m_HttpErrorString="";
				}
				else if(m_bAbort){
					if(!m_bAbortSilently){
						// TODOV3 this leads to a slot which reallocates drawarrays, although this is not needed here...
						emit inetLoadSimilarAborted();
					}
					else{
						if(m_bMainWaits){
							m_pImageSortWidget->m_WaitCondition.wakeAll();
						}
					}
				}
				else
					emit inetLoadSimilarCompleted();
				break;
			case SORT:
				if(m_bAbort){
					if(!m_bAbortSilently)
						emit sortAborted();
				}
				else
					emit sortCompleted();
				break;
			default:
				break;
		}

		// go to sleep until the restart flag is set (by main thread)
        m_Mutex.lock();
		if(!m_bRestart){
			m_bWaiting=true;
			sortWidget.m_WaitCondition.wakeAll();
            m_WaitCondition.wait(&(this->m_Mutex));
		}
		m_bWaiting=false;
		m_bAbort=false;
		m_bAbortSilently=false;
        m_bRestart=false;
        m_Mutex.unlock();
	}

}

void imageSortWorker::_load_dirs()
{
	if(!m_bLoadSimilars){
		if(m_Dirs.empty()||!m_Dirs.dirty()){
			assert(false);
			return;
		}
	}
	else{
		// we want to redraw each time the dynamicSearcher has inserted a 10th of the images we want to see
		m_pImageSortWidget->m_DynamicSearcher.registerSearchAdvancedCallback((isortisearch::pxCallback)repaintRequestedStaticCB,(void*)this,m_uNumSimilarShown/10,true);
		m_uDirsScannedForSimilars=m_uFilesScannedForSimilars=0;
	}
		

	// note this is a bit dangerous here
	// we have to set the dirs dirty, but the widget will
	// display the 'dbl click to load' message then, if we don't do
	// a workaround. the workaround is in the widget...
	m_Dirs.setDirty();

	// a user may choose dirs w/o files, nothing to complain...
	if(!m_Dirs.numFiles())
		return;
	
	// allocate storage in the parent widget
	// note we may have to lock() the widget
	m_pImageSortWidget->lock();
	if(!m_bLoadSimilars)
		m_pImageSortWidget->allocate(m_Dirs.numFiles());
	else{
		// note wether or not the user may has requested more similars to be shown, we cannot
		// load more than we have...
		unsigned u=qMin(m_uNumSimilarShown,m_Dirs.numFiles());
		// not needed:
		// m_pImageSortWidget->allocate(u);
		// NOTE this is a bit of a hack: the number of images loaded is shown in the message bar...
		m_pImageSortWidget->m_uNumThumbsExpected=m_Dirs.numFiles();
		m_pImageSortWidget->m_DynamicSearcher.clearResults();
		m_pImageSortWidget->m_DynamicSearcher.setResultSize(u);
	}
	m_pImageSortWidget->unlock();

	emit loadingStarted();

	// load the dirs
	bool bAbort;
	QMap<QString,QFileInfoList>::const_iterator it=m_Dirs.filesPerDir().constBegin();
	QMap<QString,QFileInfoList>::const_iterator itEnd=m_Dirs.filesPerDir().constEnd();
	while(it!=itEnd){
		// load the dir
		_load_dir(it.key(),it.value());
		if(m_bLoadSimilars){
			++m_uDirsScannedForSimilars;
			emit updateStatusBar2(m_uDirsScannedForSimilars,m_uFilesScannedForSimilars);
		}
		else
			emit updateStatusBar();
		++it;

		// check if to abort
		lock();
		bAbort=m_bAbort;
		unlock();

		if(bAbort){
			m_Dirs.setDirty();
			break;
		}

	}

	// tell the widget if color sort is invalid
	// note that if aborted, the widget will receive a signal in return of this function... 
	m_pImageSortWidget->lock();
	m_pImageSortWidget->m_bHasColorSort=false;
	m_pImageSortWidget->unlock();

	//if(m_bLoadSimilars)
	//	m_Dirs.clear();

	if(m_bLoadSimilars)
		m_uDirsScannedForSimilars=m_uFilesScannedForSimilars=0;		

	m_bLoadSimilars=false;
}


bool imageSortWorker::_load_dir(const QString& dir,const QFileInfoList& files)
{
const unsigned toShowTenth = m_uNumSimilarShown/10;

	imageSortWidget& sortWidget=*m_pImageSortWidget;

	// we propably can ommit these locks...
	//sortWidget.lock();
	//QLinkedList<thumbnail>& sortItems=sortWidget.m_SortItems;
	//QVector<thumbnail*>& sortedByName=sortWidget.m_SortedByName;
	//sortWidget.unlock();

	QString str,key,ext;
	bool bLoaded,bLoadedFromCache;
	// the cache is 'smaller', if there are image files in the dir
	// which are not in the cache, in this case, the cache has to be updated
	// bool bCachedThumbsSmaller=false;
	// the cache is 'bigger', if there are images in the cache 
	// which are not in the dir, in this case, the cache has to be updated,

	// note we have to take care when we filter the images in this dir against
	// file extensions, dimensions, color/bw or size, we will always have less images than 
	// contained in the cache...
	// bool bCachedThumbsBigger=false;
	// Note if for any reason the thumbnail data in the cache is out of date
	// (i.e. bigger/smaller feature data), rewrite the cache
	// bool bCacheOutVersioned=false;
	bool bThumbOutVersioned=false;
	unsigned nAdded=0;

	// do we filter on the file extension?
	const bool bDoFileExtFilter=m_bUseSearchOptions&&!m_searchOptions.m_Format.isEmpty();
	/*bool bDoOrientationFilter=(m_searchOptions.m_Orientation>0);
	bDoOrientationFilter|=(m_searchOptions.m_Size>0);
	bDoOrientationFilter|=(m_searchOptions.m_Coloration>0);
	bDoOrientationFilter|=(!m_searchOptions.m_Keyword.isEmpty());*/

	//bDoFileExtFilter&=(m_searchOptions.m_Orientation>0);

	// new V4: small and medium image size (large is everything bigger than medium)
	// TODOV5: make this options or parameters
	const unsigned int smallSize=400;
	const unsigned int mediumSize=1400;

	// walk the files in the folder,
	// check if in cached thumbs, otherwise load
	for(unsigned i=0;i<files.size();++i){
		// check if to be aborted
		bool bAbort=false;

		lock();
		bAbort=m_bAbort;
		unlock();

		if(bAbort){
			// loading stopped: set dir dirty...
			m_Dirs.setDirty(dir);
			// ...but return true, the return value will tell if the cache is dirty...
			// TODOV3: is this correct?
			// i think we'd better return a variable then which gives the cache dirty state
			return true;
		}

		bLoaded=bLoadedFromCache=false;

		// new behaviour: try to load symbolic links or shortcuts
		bool bIsLink=files[i].isSymLink();
		if(bIsLink)
			str=files[i].symLinkTarget();
		else
			str=files[i].absoluteFilePath();
		ext=files[i].completeSuffix();
		
		// new V4: if we have a filename keyword filter, use it here
		if(m_bUseSearchOptions&&!m_searchOptions.m_Keyword.isEmpty()){
			if(!files[i].fileName().contains(QRegExp(m_searchOptions.m_Keyword, Qt::CaseInsensitive)))
				continue;
		}

		// new V4: file format filter
		QString lowerExt=ext.toLower();
		if(bDoFileExtFilter){
			if(m_searchOptions.m_Format == "jpeg") {
				if(lowerExt != "jpeg" && lowerExt != "jpg" )
					continue;
			}
			else if(m_searchOptions.m_Format == "tif") {
				if(lowerExt != "tif" && lowerExt != "tiff" )
					continue;
			}
			else if(m_searchOptions.m_Format!=lowerExt)
				continue;
		}

		// create a QString as a key to insert into the hash...
		//key=str;
		//key+=files[i].lastModified().toString();
		//key+=QString::number(files[i].size());

		thumbnail thumb;
		thumbnail* p;
//#ifdef _DEBUG
//		// create a test image to test the feature data calculation
//		bool bOnce=false;
//		if(!bOnce){
//			QImage img(256,80,QImage::Format_RGB32);
//			unsigned i,j;
//			QRgb val;
//			// 20 lines greysacle
//			for(i=0;i<20;++i)
//				for(j=0;j<256;++j){
//					val=qRgb(j,j,j);
//					img.setPixel(j,i,val);
//				}
//			// 20 lines blue inversed
//			for(;i<40;++i)
//				for(j=0;j<256;++j){
//					val=qRgb(0,0,255-j);
//					img.setPixel(j,i,val);
//				}
//			// 20 lines green
//			for(;i<60;++i)
//				for(j=0;j<256;++j){
//					val=qRgb(0,j,0);
//					img.setPixel(j,i,val);
//				}
//			// 20 lines red inversed
//			for(;i<80;++i)
//				for(j=0;j<256;++j){
//					val=qRgb(255-j,0,0);
//					img.setPixel(j,i,val);
//				}
//			thumb.m_Img=img;
//			thumb.setBaseMembers();
//			thumbnailCalculator::calculateFeatureData(&thumb);
//			thumb.validate();
//			bLoaded=bLoadedFromCache=bOnce=true;
//			// write feature data to a file
//			QFile f("featuredata.txt");
//			f.open(QIODevice::WriteOnly|QIODevice::Text);
//			QDataStream stream(&f);
//			const char* pData=thumb.featureData();
//			QString str;
//			for(i=0;i<thumb.featureDataSize();++i){
//				str=QString("featureData[%1]=%2\n").arg(i,2,10).arg((int)(*pData++),4,10);
//				stream << str.toLocal8Bit() ;
//			}
//			f.close();
//		}
//
//		if(!bLoaded)
//#endif
		// try to load from cache
		bLoaded=bLoadedFromCache=m_ThumbsCache.load(str,thumb);
		if(bLoaded){
			thumb.validate();
			// assert(!(thumb.m_JpegBinaryData.isEmpty()));
			// note if the thumb is out of version
			// (currently: older feature data)
			// update it and rewrite the thumbs cache

			// V4.3 we do not do this any longer, if a thumbnail in the cache is out of version now
			// we delete it and load the original instead of recalculating the feature data
			// reason: we do not want to write thumbnails back to the cache when they have been loaded from the cache
			// for this, we have to keep the jpeg binary data (otherwise we have to do the jpeg compression again and agin, 
			// degrading image quality). however, we need the jpeg binary data for this purpose *only*,
			// thus we can save memory footprint....

			//if(thumb.isOutVersioned()){
			//	// note this is IGNORE_ALPHA mode
			//	thumbnailCalculator::calculateFeatureData(&thumb);
			//	thumb.setNotOutVersioned();
			//	bool b=m_ThumbsCache.save(thumb);
			//	assert(b);
			//	assert(thumb.featureDataValid());
			//}
		}
		
		// if not loaded from cache up to now, load original
		if(!bLoaded && (str.endsWith(".jpg",Qt::CaseInsensitive)||str.endsWith(".jpeg",Qt::CaseInsensitive)) ){
			// try to extract exif thumbnail
			bLoaded=_load_jpeg_exif_thumbnail(str,&thumb);

			// read jpeg using our own code (faster than QImageReader)
			// this may fail if the image is very small...
			if(!bLoaded)
				bLoaded=_load_jpeg(str,&thumb);
			
			// _load_jpeg() does no upscale, so it might fail on small jpegs:
			if(!bLoaded)
				bLoaded=_load_qimage(str,&thumb);
		}
		else if(!bLoaded){
			bLoaded=_load_qimage(str,&thumb);
		}

		// new V4: test if correct color/bw/orientation/size
		if(bLoaded&&m_bUseSearchOptions) {
			const unsigned w=thumb.m_Img.width();
			const unsigned h=thumb.m_Img.height();
			// landscape or portrait?
			if(m_searchOptions.m_Orientation==1 && w >= h)
				continue;
			else if(m_searchOptions.m_Orientation==2 && w <= h)
				continue;

			// image size small, medium or large?
			// TODOV4: the sum is a strange criteria...
			const unsigned int imageSize=w + h;
			if(m_searchOptions.m_Size==1 && imageSize > smallSize )
				continue;
			else if(m_searchOptions.m_Size==2 && (imageSize <= smallSize || imageSize > mediumSize) )
				continue;
			else if(m_searchOptions.m_Size==3 && imageSize < mediumSize )
				continue;

			// color or greyscale?
			if(m_searchOptions.m_Coloration==1 && thumb.m_Img.isGrayscale())
				continue;
			else if(m_searchOptions.m_Coloration==2 && !thumb.m_Img.isGrayscale())
				continue;
		}

		if(!bLoaded){
			// set the loadFailure image as placeholder
			if(!m_bLoadSimilars){
				thumb.m_Img=m_LoadFailureImage;
				// assert(thumb.m_JpegBinaryData.isEmpty());
				thumb.setNotLoaded();
				// note shall we cash unloaded images with the placeholder?
			}
			// we search similar images, so just:
			else{
				continue;
			}

		}

		thumb.m_FileName=str;
		thumb.m_FileSize=files[i].size();
		thumb.m_LastModified=files[i].lastModified();

		thumb.validate();

		if(!bLoadedFromCache){
			bool b=thumb.setBaseMembers();
			assert(b);
			// note this is ALPHA_IGNORE mode
			thumbnailCalculator::calculateFeatureData(&thumb);
			// new V4: 
			// if the image has tranparency, paste it on a checker board HERE
			// calculateFeatureData() BEFORE
			if(!bLoadedFromCache&&thumb.m_Img.hasAlphaChannel()){
				pasteOnCheckerBoard(&(thumb.m_Img));
			}
			// write to cache
			// assert(thumb.m_JpegBinaryData.isEmpty());
			b=m_ThumbsCache.save(thumb);
			assert(b);
		}
		// V5: do we need this?
		//else
		//	thumb.validateFeatureData();

		// create a refPtr<controllerImage>
		// note that this still holds a thumbnail*...
		thumbnail* pThumb=new thumbnail(thumb);
		isortisearch::controllerImage* pImg=(isortisearch::controllerImage*)pThumb;
		isortisearch::refPtr<isortisearch::controllerImage> refp(pImg);
		// need to lock this:
		if(!m_bLoadSimilars){
			sortWidget.lock();
			sortWidget.m_ControllerImages.push_back(refp);
			sortWidget.unlock();
		}
		else{
			//sortWidget.lock();
			// nopte locks...
			sortWidget.m_DynamicSearcher.dynamicSearch(refp);
			//sortWidget.unlock();

		}

		++nAdded;
		
		if(!m_bLoadSimilars){
			// V5: each 20 images
			if(!(sortWidget.numThumbs()%20)) {
				// TODOV5: what is this????
				// we don't really need this any longer. In prior versions, we had the draw arrays
				// which had to be re- allocated here
				//if(m_bUseSearchOptions){
				//	//sortWidget.lock();
				//	sortWidget.allocate(nAdded,true);
				//	//sortWidget.unlock();
				//}
				emit repaintWidget();
				emit updateStatusBar();

			}
		}
		else{
			// let the main thread draw images each time 1/10 of the desired images is checked
			// note the famous anuj issued a paint request every time an image was checked
			// these are far too much, because the search itself is faster...
			// i don't really know what the qt paint engine does with all these request, but at
			// least the signals flooded qt's event dispatcher, thus leaving the app blocked
			// until all signals handled...
			++m_uFilesScannedForSimilars;

			if(!toShowTenth||!(nAdded%toShowTenth)){
				// V5: done through the callback of the dynamicSearcher
				// ok, calling emit here is faster, but we want to *test* the libraries
				// emit repaintWidget();
				emit updateStatusBar2(m_uDirsScannedForSimilars,m_uFilesScannedForSimilars);
			}
		}
	}
	

	// loaded, thus set dir clean
	// note: not when we load similars, because this is something completely different: we have 
	// not loaded the complete dir, but only the n most similar images...
	if(m_bLoadSimilars)
		m_Dirs.setDirty(dir);
	else
		m_Dirs.setClean(dir);


	// TODOV5: what is this?
	// we don't really need this any longer. In prior versions, we had the draw arrays
	// which had to be re- allocated here
	//if(m_bUseSearchOptions)
	//	sortWidget.allocate(nAdded,true);

	// repaint all loaded...
	emit repaintWidget();

	return true;
}

bool imageSortWorker::_load_jpeg_exif_thumbnail(const QString& str,thumbnail* pThumb)
{
	// not really needed:
	if(!pThumb)
		return false;
	
	exifThumbnailExtractor ex;

	imageIO::jpegLoader::exifThumbData* pData=ex.extractExifThumbnail(str.toStdString());

	if(!pData)
		return false;

	// get the size of the original image
	imageIO::jpegLoader* pLoader;
	imageIO imgIO;
	try{
		pLoader=imgIO.jpegGetLoader(pData->_pdata,pData->_sz);
		assert(pLoader);
		const unsigned w=pLoader->inWidth();
		const unsigned h=pLoader->inHeight();

		// if the original image is smaller than our thumb size, 
		// we load it with the QImageReader
		if(max(w,h)<=m_pImageSortWidget->thumbSize()){
			pLoader->close();
			// V4.2
			delete pLoader;
			return false;
		}

		// note load grayscale images as rgb...
		pLoader->setOutColorSpace(1);
		pLoader->setFastMode();
		pLoader->setDctMode(2);

		// scale down, but make sure the smaller side is at least 8 pixels after downscale and the bigger is bigger than 40 pixels
		// see size constraint of isortisearch::image::calculateFeatureData()
		if(!pLoader->scaleBelow(m_pImageSortWidget->thumbSize(),40,8)){
			pLoader->close();
			// V4.2:
			delete pLoader;
			return false;
		}

		// make a thumbnail QImage, keep aspect ratio
		const unsigned sw=pLoader->outWidth();
		const unsigned sh=pLoader->outHeight();

		// note take care: QImage format is FFRRGGBB, whether we choose Format_RGB32 or
		// Format_ARGB32. however, the alpha channel is not applied with Format_RGB32
		// note also that when applied, drawing is slowed down 
		QImage temp(sw,sh,QImage::Format_RGB32);
		byteImage bImg;
		bImg.create(sw,sh,4,0,0,0,true,temp.bits());
		// note: Qt 32bit QImages have format ARGB

		// however, if we access the image pixels directly, we have to take care on little/big endian:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		// here, we access BGRA due to little endian
		bImg.setPreChannels(3);
		unsigned char white=255;
		bImg.fill(&white);
		bImg.setPreChannels(0);
		bImg.setPostChannels(1);
		bImg.setColorSpace(byteImage::colorSpaceBGR);
#else	
		bImg.setPostChannels(3);
		unsigned char white=255;
		bImg.fill(&white);
		bImg.setPostChannels(0);
		bImg.setPreChannels(1);
		bImg.setColorSpace(byteImage::colorSpaceRGB);
#endif	

		// throw on error, is catched below...
		// V4.2: note pLoader is deleted in this function...
		imgIO.jpegLoad(pLoader,&bImg);

		// geaendert Kai
		// old:
		pThumb->m_Img=temp;

	}
	catch(...){
		delete pData;
		return false;
	}


	delete pData;

	return true;
}

bool imageSortWorker::_load_jpeg(const QString& str,thumbnail* pThumb)
{
	// not really needed:
	if(!pThumb)
		return false;
	// get the size of the original image
	imageIO::jpegLoader* pLoader;
	imageIO imgIO;
	try{
		pLoader=imgIO.jpegGetLoader(str.toStdString());
		assert(pLoader);
		const unsigned w=pLoader->inWidth();
		const unsigned h=pLoader->inHeight();

		// if the original image is smaller than our thumb size, 
		// we load it with the QImageReader
		if(max(w,h)<=m_pImageSortWidget->thumbSize()){
			pLoader->close();
			// V4.2:
			delete pLoader;
			return false;
		}

		// note load grayscale images as rgb...
		pLoader->setOutColorSpace(1);
		pLoader->setFastMode();
		pLoader->setDctMode(2);

		// scale down, but make sure the smaller side is at least 8 pixels after downscale and the bigger is bigger than 40 pixels
		// see size constraint of isortisearch::image::calculateFeatureData()
		if(!pLoader->scaleBelow(m_pImageSortWidget->thumbSize(),40,8)){
			pLoader->close();
			// V4.2:
			delete pLoader;
			return false;
		}

		// make a thumbnail QImage, keep aspect ratio
		const unsigned sw=pLoader->outWidth();
		const unsigned sh=pLoader->outHeight();

		// note take care: QImage format is FFRRGGBB, whether we choose Format_RGB32 or
		// Format_ARGB32. however, the alpha channel is not applied with Format_RGB32
		// note also that when applied, drawing is slowed down 
		QImage temp(sw,sh,QImage::Format_RGB32);
		byteImage bImg;
		bImg.create(sw,sh,4,0,0,0,true,temp.bits());
		// note: Qt 32bit QImages have format ARGB
		
		// however, if we access the image pixels directly, we have to take care on little/big endian:
		#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
			// here, we access BGRA due to little endian
			bImg.setPreChannels(3);
			unsigned char white=255;
			bImg.fill(&white);
			bImg.setPreChannels(0);
			bImg.setPostChannels(1);
			bImg.setColorSpace(byteImage::colorSpaceBGR);
		#else	
			bImg.setPostChannels(3);
			unsigned char white=255;
			bImg.fill(&white);
			bImg.setPostChannels(0);
			bImg.setPreChannels(1);
			bImg.setColorSpace(byteImage::colorSpaceRGB);
		#endif	
		
		// throw on error, is catched below...
		// V4.2: this function deletes pLoader
		imgIO.jpegLoad(pLoader,&bImg);
		
		// geaendert Kai
		// old:
		pThumb->m_Img=temp;
			
	}
	catch(...){
		return false;
	}
	return true;

}

bool imageSortWorker::_load_qimage(const QString& str,thumbnail* pThumb)
{ 
	// not really needed:
	if(!pThumb)
		return false;

	QImageReader rdr(str);
	QSize sz=rdr.size();

	if(!sz.isValid())
		return false;

	// BUG FIX V2.03
	// when scaling an image with extreme ratios of width/height or heigth/width
	// one of the dimensions can be 0 afterwards when using QSize, leading to a div by zero below.
	// NOTE that depending on the debugger exceptions settings (MSVC), this does not
	// neccesarily break in the debugger, but definitely crashes in the release mode :(
	// the bug fix is to use QSizeF AND to check for dimensions NEAR 0 afterwards
	QSizeF szf(sz);
	// scale, keep aspect ratio
	szf.scale(m_pImageSortWidget->thumbSize(),m_pImageSortWidget->thumbSize(),Qt::KeepAspectRatio);
	
	// NOTE are these tests for zero ok?
	if(szf.width()==0.0)
		szf.rwidth()=DBL_MIN;
	if(szf.height()==0.0)
		szf.rheight()=DBL_MIN;

	// make sure bigger side is at least 40 pixels wide
	if(szf.width()>=szf.height()){
		if(szf.width()<40.){
			double f=40./szf.width();
			szf.scale(40.,szf.height()*f,Qt::IgnoreAspectRatio);
		}
	}
	else{
		if(szf.height()<40.){
			double f=40./szf.height();
			szf.scale(szf.width()*f,40.,Qt::IgnoreAspectRatio);
		}
	}
	// make sure smaller side is at least 8 pixels wide
	if(szf.width()<=szf.height()){
		if(szf.width()<8.){
			double f=8./szf.width();
			szf.scale(8.,szf.height()*f,Qt::IgnoreAspectRatio);
		}
	}
	else{
		if(szf.height()<8.){
			double f=8./szf.height();
			szf.scale(szf.width()*f,8.,Qt::IgnoreAspectRatio);
		}
	}
	sz=szf.toSize();

	// note funny enough, after having called size() we need to set the filename again...
	// i think this is because the internal device of the reader is deleted automatically...
	rdr.setFileName(str);
	rdr.setScaledSize(sz);

	// read the image
	if(!rdr.canRead())
		return false;
	
	pThumb->m_Img=rdr.read();
	if(pThumb->m_Img.isNull())
		return false;

	// check if 32 bit rgb image, otherwise convert...
	if(pThumb->m_Img.depth()!=32)
		pThumb->m_Img=pThumb->m_Img.convertToFormat(QImage::Format_ARGB32);

	assert(pThumb->m_Img.width()>=8&&pThumb->m_Img.height()>=8);

	return true;
}

void imageSortWorker::_sort()
{
	emit sortingStarted();

	// just an abbrev:
	imageSortWidget& sortWidget=*m_pImageSortWidget;

	//sortWidget.m_GuiController.init(&(sortWidget.m_ControllerImages));
	sortWidget.m_Sorter.setImages(&(sortWidget.m_ControllerImages));
	sortWidget.m_Sorter.sort(true);

	// set and redraw last (probably aborted) sort
	cbNewSort();

	return;
}


void imageSortWorker::cbNewSort()
{
	// V5: in prior versions, we had an array of somNodes here
	// the sort was done on this array
	// this callback copied the nodes x/y values to the sort data then.
	// this is done in the library now...
	
	// thus:
	// now tell the main window to redraw
	emit newSort();
}


void imageSortWorker::_load_from_net()
{
	m_bHttpError=false;
	
	unsigned toAllocate=m_bLoadSimilars?m_uNumSimilarQueries:m_uNumQueries;
	unsigned toShow=m_bLoadSimilars?m_uNumSimilarShown:m_uNumQueries;
	
	m_pImageSortWidget->m_uNumThumbsExpected=toAllocate;
	
	emit loadingStarted();

	// query once and get number of available results...
	QBuffer buf;

	if(m_bYahooImgSearch){
		// this is a sample request url from the yahoo image search api website:
		// http://search.yahooapis.com/ImageSearchService/V1/imageSearch?appid=YahooDemo&query=Corvette&results=2
		SyncHTTP searchhttp("search.yahooapis.com");
		

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
		searchhttp.syncGet("/ImageSearchService/V1/imageSearch?appid=imagesorter&query="+m_Query.toUtf8()+"&results=50&start="+QString::number(1)+options,&buf);
		// note i'd rather like a retry message box here, but we cannot have QWidgets in this thread, they have to be in the main thread...
		if(searchhttp.error()!=QHttp::NoError){
			m_bHttpError=true;
			m_HttpErrorString=searchhttp.errorString();
			return ;
		}

		yahooImgSearchV1XmlReader rd;
		buf.open(QIODevice::ReadOnly);
		// there may be less results as we expect, adjust this here
		bool bOk;
		unsigned totalAvailResults;

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Setzen der Suchoptionen für einzelne Threads
		*/
		rd.setSearchOptions(m_searchOptions);

		rd.read(&buf,&m_YahooImgSearchV1Results,&bOk,&totalAvailResults);
		if(bOk&&totalAvailResults<toAllocate)
			toAllocate=totalAvailResults;
		buf.close();

		// no result?
		// return
		if(!toAllocate){
			// TODO build in error handling...
			return;
		}

		// more queries needed?
		// get them threaded...
		if(m_YahooImgSearchV1Results.size()<toAllocate){
			yahooImgSearchV1QueryDispatcher qdsp;

			qdsp.setWorker(this);
			qdsp.setResults(&m_YahooImgSearchV1Results);
			qdsp.setQuery(m_Query);

			/*
			*	Hinzugefügt von Claudius Brämer und David Piegza
			*
			*   Setzen der Suchoptionen für einzelne Threads
			*/
			qdsp.setSearchOptions(m_searchOptions);

			qdsp.setNumQueries(toAllocate);
			qdsp.setNumQueriesPerThread(50);
			// V5: more threads are faster, i think...
			qdsp.setNumThreads(10);
			qdsp.start();
			while(qdsp.wait(10)==false)
				QApplication::processEvents();

			// got an error?
			if(qdsp.hasHttpError()){
				m_bHttpError=true;
				m_HttpErrorString=qdsp.httpErrorString();
				return;
			}
		
		}
		// got too much? remove elements at the back of the fifo...
		while(m_YahooImgSearchV1Results.size()>toAllocate)
			m_YahooImgSearchV1Results.removeLast();

		// maybe got less (aborted)? adjust...
		toAllocate=qMin((unsigned)m_YahooImgSearchV1Results.size(),toAllocate);
	}
	else{
		// this is a sample request url from the flickr api website:
		// URL: http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=d130fefc4916a1b32dea35decf4f3cfe&text=sonnenblume&per_page=50&page=1
		SyncHTTP searchhttp("api.flickr.com");
		searchhttp.syncGet("/services/rest/?method=flickr.photos.search&api_key=dd5fcf9e7c30d0e630f65152df11f9af&text="+m_Query.toUtf8()+"&per_page=50&page=1",&buf);

		// note i'd rather like a retry message box here, but we cannot have QWidgets in this thread, they have to be in the main thread...
		if(searchhttp.error()!=QHttp::NoError){
			m_bHttpError=true;
			m_HttpErrorString=searchhttp.errorString();
			return ;
		}

		flickrImgSearchXmlReader rd;
		buf.open(QIODevice::ReadOnly);
		// there may be less results as we expect, adjust this here
		bool bOk;
		unsigned totalAvailResults;
		rd.read(&buf,&m_flickrImgSearchResults,&bOk,&totalAvailResults);
		if(bOk&&totalAvailResults<toAllocate)
			toAllocate=totalAvailResults;
		buf.close();

		// no result?
		// return
		if(!toAllocate){
			// TODO build in error handling...
			return;
		}

		// more queries needed?
		// get them threaded...
		int sz=m_flickrImgSearchResults.size();
		if(m_flickrImgSearchResults.size()<toAllocate){
			flickrImgSearchQueryDispatcher qdsp;

			qdsp.setWorker(this);
			qdsp.setResults(&m_flickrImgSearchResults);
			qdsp.setQuery(m_Query);
			qdsp.setNumQueries(toAllocate);
			qdsp.setNumQueriesPerThread(50);
			// V5: more threads are faster, i think...
			qdsp.setNumThreads(10);
			qdsp.start();
			while(qdsp.wait(10)==false)
				QApplication::processEvents();

			// got an error?
			if(qdsp.hasHttpError()){
				m_bHttpError=true;
				m_HttpErrorString=qdsp.httpErrorString();
				return;
			}

		}
		// got too much? remove elements at the back of the fifo...
		while(m_flickrImgSearchResults.size()>toAllocate)
			m_flickrImgSearchResults.removeLast();

		// maybe got less (aborted)? adjust...
		toAllocate=qMin((unsigned)m_flickrImgSearchResults.size(),toAllocate);

	}

	// note we load to another widget container here
	if(!m_bLoadSimilars){
		m_pImageSortWidget->lock();
		m_pImageSortWidget->allocate(toAllocate);
		m_pImageSortWidget->unlock();
	}
	else{
		//m_pImageSortWidget->m_SimilarItems.clear();
		//m_pImageSortWidget->m_SimilarItems.setSize(toShow);
		m_pImageSortWidget->lock();
		m_pImageSortWidget->m_DynamicSearcher.clearResults();
		m_pImageSortWidget->m_DynamicSearcher.setResultSize(toShow);
		m_pImageSortWidget->unlock();
	}

	
	// now download the thumbnails...
	urlThumbLoadDispatcher dsp;
	dsp.setWorker(this);
	// V5: num threads is an option now :)
	dsp.setNumThreads(m_uNumNetSearchThreads);
	dsp.setRepaintInterval(qMax(m_uNumNetSearchThreads/3,(unsigned)1));
	dsp.enableSimilaritySearch(m_bLoadSimilars);
	dsp.start();
	while(dsp.wait(10)==false)
		QApplication::processEvents();

	// got errors?
	if(dsp.hasHttpError()){
		// the dispatcher has a multi map containing http error strings stored by url strings
		// we may display them if needed
#pragma message("build in error handling...")
		qWarning() << dsp.httpErrorStrings();
		//assert(false);
	}

	// note if we do not clear the dirs, we can re-load the current search if we have selected another dir, don't do anything and
	// return to the last dir (as usual). this may be not bad, however, we show the last search then, *not* the content of the dir...
	m_Dirs.clear();

}


void imageSortWorker::pasteOnCheckerBoard(QImage* pImg)
{
	assert(pImg);
	assert(pImg->hasAlphaChannel());
	// create a byteImage based on a QImage...
	byteImage byteImg;
	byteImg.create(pImg->width(),pImg->height(),4,0,0,0,true,pImg->bits());
	// note: Qt 32bit QImages have format ARGB
	// however, if we access the image pixels directly, we have to take care on little/big endian:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
	// here, we access BGRA due to little endian
	byteImg.setColorSpace(byteImage::colorSpaceBGRA);
#else
	// if this is mac, add a #if defined(MAC), test what the compiler predefines for this case
	byteImg.setColorSpace(byteImage::colorSpaceARGB);
#endif
	simpleOverCheckerBoard<unsigned char> over;
	// make the alpha channel opaque, so we will see the checkerboard...
	// NOTE: make sure we have calculated feature data before this...
	over.enableOpaqueAlpha(false);
	over.run(&byteImg);
	byteImg.destroy();

}

void imageSortWorker::abort()
{
	// V5: in versions prior to this, the loading code *and* the color sort
	// where aborted by setting the same flag false.
	// for the sort, this is in the library now...

	// do we know what we are running currently?
	if(loadMode()||scanMode()){
		lock();
		m_bAbort=true;
		m_bAbortSilently=false;
		unlock();
	}
	else if(sortMode()){
		m_bAbort=true;
		m_bAbortSilently=false;
		m_pImageSortWidget->m_Sorter.abort();
	}
}

void imageSortWorker::abortSilently(bool bWait)
{
	// V5: in versions prior to this, the loading code *and* the color sort
	// where aborted by setting the same flag false.
	// for the sort, this is in the library now...

	// do we know what we are running currently?
	if(loadMode()||scanMode()){
		lock();
		m_bAbort=true;
		m_bAbortSilently=true;
		m_bMainWaits=bWait;
		unlock();
	}
	else if(sortMode()){
		m_pImageSortWidget->m_Sorter.abort();
		m_bAbort=true;
		m_bAbortSilently=true;
		m_bMainWaits=bWait;
	}
}

void imageSortWorker::repaintRequestedStaticCB(void *pUserData)
{
	assert(pUserData);
	((imageSortWorker*)(pUserData))->repaintRequestedByDynamicSearcher();
}


void imageSortWorker::repaintRequestedByDynamicSearcher()
{
	emit repaintWidget();
}

