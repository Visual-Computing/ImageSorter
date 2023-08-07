#include "imagesortdirs.h"
#include <assert.h>
#include "filefinder.h"
#include "ismutex.h"
// #include <iostream>
imageSortDirs::imageSortDirs() : m_bCacheKeyDirty(true), m_uTotalFiles(0)
{
#pragma message("test all supported image formats...")
	// the image files extensions we can load
	m_ImageExts.append("*.jpg");
	m_ImageExts.append("*.jpeg");
	m_ImageExts.append("*.png");
	m_ImageExts.append("*.gif");
	m_ImageExts.append("*.bmp");
	m_ImageExts.append("*.pbm");
	m_ImageExts.append("*.pgm");
	m_ImageExts.append("*.ppm");
	m_ImageExts.append("*.xbm");
	m_ImageExts.append("*.xpm");
	m_ImageExts.append("*.tif");
	m_ImageExts.append("*.tiff");
	m_ImageExts.append("*.svg");
#pragma message("check out if Qt 4.3.x QDirModel::setNameFilters() differs from Qt 4.2.x (case sensivity)")
	m_ImageExts.append("*.JPG");
	m_ImageExts.append("*.JPEG");
	m_ImageExts.append("*.PNG");
	m_ImageExts.append("*.GIF");
	m_ImageExts.append("*.BMP");
	m_ImageExts.append("*.PBM");
	m_ImageExts.append("*.PGM");
	m_ImageExts.append("*.PPM");
	m_ImageExts.append("*.XBM");
	m_ImageExts.append("*.XPM");
	m_ImageExts.append("*.TIF");
	m_ImageExts.append("*.TIFF");
	m_ImageExts.append("*.SVG");

#if defined(Q_OS_WIN32)
#pragma message("the name filter entry for links is platform dependant")
	m_ImageExts.append("*.jpg.lnk");
	m_ImageExts.append("*.jpeg.lnk");
	m_ImageExts.append("*.png.lnk");
	m_ImageExts.append("*.gif.lnk");
	m_ImageExts.append("*.bmp.lnk");
	m_ImageExts.append("*.pbm.lnk");
	m_ImageExts.append("*.pgm.lnk");
	m_ImageExts.append("*.ppm.lnk");
	m_ImageExts.append("*.xbm.lnk");
	m_ImageExts.append("*.xpm.lnk");
	m_ImageExts.append("*.tif.lnk");
	m_ImageExts.append("*.tiff.lnk");
	m_ImageExts.append("*.svg.lnk");
	m_ImageExts.append("*.JPG.lnk");
	m_ImageExts.append("*.JPEG.lnk");
	m_ImageExts.append("*.PNG.lnk");
	m_ImageExts.append("*.GIF.lnk");
	m_ImageExts.append("*.BMP.lnk");
	m_ImageExts.append("*.PBM.lnk");
	m_ImageExts.append("*.PGM.lnk");
	m_ImageExts.append("*.PPM.lnk");
	m_ImageExts.append("*.XBM.lnk");
	m_ImageExts.append("*.XPM.lnk");
	m_ImageExts.append("*.TIF.lnk");
	m_ImageExts.append("*.TIFF.lnk");
	m_ImageExts.append("*.SVG.lnk");
#endif

	// the raw file extensions we can load by dcraw are set by the user...
	//m_RawImageExts.append("*.raf");
	//m_RawImageExts.append("*.RAF");

	// note this is a bit memory consumption
	// m_NameFilter=m_ImageExts+m_RawImageExts;
	m_NameFilter=m_ImageExts;
}


imageSortDirs::imageSortDirs(const imageSortDirs& rOther) : 
m_ImageExts(rOther.m_ImageExts), 
//m_RawImageExts(rOther.m_RawImageExts),
m_NameFilter(rOther.m_NameFilter),
m_CacheKey(rOther.m_CacheKey),
m_FilesPerDir(rOther.m_FilesPerDir),
m_TopDirs(rOther.m_TopDirs),
m_LinkNames(rOther.m_LinkNames),
m_CleanDirs(rOther.m_CleanDirs),
m_EmptyList(rOther.m_EmptyList)
{
	m_bCacheKeyDirty=rOther.m_bCacheKeyDirty;
	m_uTotalFiles=rOther.m_uTotalFiles;
}

imageSortDirs& imageSortDirs::operator=(const imageSortDirs& rOther)
{
	if(this!=&rOther){
		m_ImageExts=rOther.m_ImageExts; 
		//m_RawImageExts=rOther.m_RawImageExts;
		m_NameFilter=rOther.m_NameFilter;
		m_CacheKey=rOther.m_CacheKey;
		m_FilesPerDir=rOther.m_FilesPerDir;
		m_TopDirs=rOther.m_TopDirs;
		m_LinkNames=rOther.m_LinkNames;
		m_CleanDirs=rOther.m_CleanDirs;
		// not needed, always empty:
		//m_EmptyList=rOther.m_EmptyList;
		m_bCacheKeyDirty=rOther.m_bCacheKeyDirty;
		m_uTotalFiles=rOther.m_uTotalFiles;
	}
	return *this;

}

/*
void imageSortDirs::setRawFileExts(const QStringList& exts)
{
	m_RawImageExts=exts;
	assert(!m_RawImageExts.contains(""));
	m_NameFilter=m_RawImageExts+m_ImageExts;

}
*/ 
imageSortDirs::~imageSortDirs()
{
	clear();
}

void imageSortDirs::clear()
{
	m_bCacheKeyDirty=true;
	m_CacheKey.clear();
	m_FilesPerDir.clear();
	m_CleanDirs.clear();
	m_LinkNames.clear();
	m_uTotalFiles=0;
}

bool imageSortDirs::addDir(const QString& absPath,bool bWithChildren,bool bFirstCall,bool *pAbortFlag,ISMutex* pMutex)
{
	
	// V2.03
	// check if the passed path is a symlink
	bool isSymLink=false;
	QFileInfo finfo(absPath);
	if(!finfo.isDir())
		return true;

	// note there is a bug in Qt 4.3.x on Win32, QFileInfo() delivers uppercase drive letters
	// other functions like QDir::absolutePath() deliver lowercase drive letters
	// we always use uppercase drive letters here 
#if defined(Q_OS_WIN32)
	// note also that QFileInfo()::absolutePath() delivers the parent directory in case of a directory...
	QString realAbsPath=finfo.absoluteFilePath();
#else
	QString realAbsPath=absPath;
#endif

	// Note check if there can be a chain of links on os x
	// on win (xp), links onto links are not allowed...
	if(finfo.isSymLink()&&finfo.exists()){
		isSymLink=true;
		realAbsPath=finfo.symLinkTarget();
	}

	// V2.02:
	// QDir dir(absPath);
	//if(!dir.exists())
	//	return;

	// V2.03
	if(!m_FilesPerDir.contains(realAbsPath)){
		QDir dir(realAbsPath);
		if(!dir.exists())
			return true;

		// note this *really* cost's time on win32
		// QFileInfoList l=dir.entryInfoList(nameFilter(),QDir::Files,QDir::Name);
		// QFileInfoList l=dir.entryInfoList(nameFilter(),QDir::Files);
#if defined(Q_OS_WIN32)
		// our own solution:
		QFileInfoList l;
		fileFinder finder;
		assert(!m_NameFilter.empty());
		if(!finder.find(dir,&l,m_NameFilter,FALSE,pAbortFlag,pMutex)){
			return false;
		}
#else
		// QDirIterator solution
		// also too slow...
		QFileInfoList l;
		QDirIterator it(realAbsPath,m_NameFilter,QDir::Files);
		// note the dir iterator is *before* the first dir entry..
		while(it.hasNext()){
			if(pMutex&&pAbortFlag){
				// V4.2: check if to abort
				bool bAbort=false;
				pMutex->lock();
				bAbort=*pAbortFlag;
				pMutex->unlock();
				if(bAbort)
					return false;
			}
			it.next();
			l.append(it.fileInfo());
		}
#endif
		m_uTotalFiles+=l.size();
		m_FilesPerDir.insert(realAbsPath,l);
		if(isSymLink)
			m_LinkNames.insert(realAbsPath,absPath);
		m_bCacheKeyDirty=true;
		
		// V3BETA2
		if(bWithChildren){
			// get sub dirs
#if defined(Q_OS_WIN32)
			QFileInfoList l2;
			finder.find(dir,&l2,m_NameFilter,TRUE,pAbortFlag,pMutex);
#else
			QFileInfoList l2=dir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
#endif
			if(bFirstCall)
				m_TopDirs.insert(absPath);
			else{
				assert(!m_TopDirs.contains(absPath));
				m_ParentDirs.insert(absPath);
			}
			// add childs recursivly...
			for(int i=0;i<l2.size();++i){
				if(!addDir(l2[i].absoluteFilePath(),true,false,pAbortFlag,pMutex))
					return false;
			}
			
		}
	}


	// NOTE: 
	// we can load a dir thru a symLink now in V2.03 (tested on win32)
	// anyway, the explorer widget highlights the symLinks target after loading
	// AND the program re-reads the symLink target when restarted
	// V2.03 shall be a workaround for the mac osx version which cannot
	// display mounted volumes in the explorer. thus, the mac will have problems with
	// this behaviour since the symLinks target does not exist in the explorer view.
	// therefore, we also have to store the symLink itself and have to let the explorer
	// scroll to the symLink and have to store the symlink in the settings...
	// however, we only have to store ONE symLink per realAbsPath, a user may set N symLinks
	// to the target, but because of m_FilesPerDir.contains(realAbsPath) above we can
	// load only once, thus we only have to store the first symLink we get.
	// Thus we can recode :
	//
	// 	QMap<QString,QFileInfoList> m_FilesPerDir;
	// 
	// into
	//
	//  QMap<QString,QPair<QFileInfoList,QString> > m_FilesPerDir;
	//
	// the second QPair parameter will be the symLink (absPath) if absPath IS a symLink,
	// empty otherwise...


	// V2.02
	//if(!m_FilesPerDir.contains(absPath)){
	//	QFileInfoList l=dir.entryInfoList(nameFilter(),QDir::Files,QDir::Name);
	//	m_uTotalFiles+=l.size();
	//	m_FilesPerDir.insert(absPath,l);
	//	m_bCacheKeyDirty=true;
	//}
	return true;
}

void imageSortDirs::updateDir(const QString& absPath)
{
	// V2.03
	QFileInfo finfo(absPath);
	if(!finfo.isDir())
		return;

	// V2.02
	//QDir dir(absPath);
	//if(!dir.exists())
	//	return;

	// Note check if there can be a chain of links on os x
	// on win (xp), links onto links are not allowed...
	QString realAbsPath=absPath;
	if(finfo.isSymLink()&&finfo.exists())
		realAbsPath=finfo.symLinkTarget();

	// this is like removeDir() followed by addDir(),
	// the cache key remains the same as well as the dirty() state...
	if(m_FilesPerDir.contains(realAbsPath)){
		QDir dir(realAbsPath);
		QFileInfoList l=m_FilesPerDir.take(realAbsPath);
		assert(m_uTotalFiles>=l.size());
		m_uTotalFiles-=l.size();
#if defined(Q_OS_WIN32)
		fileFinder finder;
		assert(!m_NameFilter.empty());
		finder.find(dir,&l,m_NameFilter,FALSE,FALSE,NULL);
#else
		l=dir.entryInfoList(nameFilter(),QDir::Files,QDir::Name);
#endif
		m_uTotalFiles+=l.size();
		m_FilesPerDir.insert(absPath,l);
	}
	// V2.02
	//QDir dir(absPath);
	//if(!dir.exists())
	//	return;

	//// this is like removeDir() followed by addDir(),
	//// the cache key remains the same as well as the dirty() state...
	//if(m_FilesPerDir.contains(absPath)){
	//	QFileInfoList l=m_FilesPerDir.take(absPath);
	//	assert(m_uTotalFiles>=l.size());
	//	m_uTotalFiles-=l.size();
	//	l=dir.entryInfoList(nameFilter(),QDir::Files,QDir::Name);
	//	m_uTotalFiles+=l.size();
	//	m_FilesPerDir.insert(absPath,l);
	//}
}

void imageSortDirs::removeDir(const QString& absPath)
{
	// V2.03
	QFileInfo finfo(absPath);
	bool isSymLink=false;
	if(!finfo.isDir())
		return;
	// Note check if there can be a chain of links on os x
	// on win (xp), links onto links are not allowed...
	QString realAbsPath=absPath;
	if(finfo.isSymLink()&&finfo.exists()){
		isSymLink=true;
		realAbsPath=finfo.symLinkTarget();
	}
	
	if(m_FilesPerDir.contains(realAbsPath)){
		QFileInfoList l=m_FilesPerDir.take(realAbsPath);
		assert(m_uTotalFiles>=l.size());
		m_uTotalFiles-=l.size();
		if(isSymLink)
			m_LinkNames.remove(realAbsPath);
		m_bCacheKeyDirty=true;
		// check if this was inserted with childs,
		// if so, remove childs too
		if(m_TopDirs.contains(absPath)||m_ParentDirs.contains(absPath)){
			QDir dir(realAbsPath);
			// get sub dirs
#if defined(Q_OS_WIN32)
			fileFinder finder;
			QFileInfoList l2;
			finder.find(dir,&l2,m_NameFilter,TRUE,FALSE,NULL);
#else
			QFileInfoList l2=dir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
#endif
			if(!l2.isEmpty()){
				// add childs recursivly...
				for(int i=0;i<l2.size();++i)
					removeDir(l2[i].absoluteFilePath());
			}
			// absPath is in top or parent dirs, one operation is NOP here:
			m_TopDirs.remove(absPath);
			m_ParentDirs.remove(absPath);
		}
	}
	// V2.02
	//if(m_FilesPerDir.contains(absPath)){
	//	QFileInfoList l=m_FilesPerDir.take(absPath);
	//	assert(m_uTotalFiles>=l.size());
	//	m_uTotalFiles-=l.size();
	//	m_bCacheKeyDirty=true;
	//}
}

void imageSortDirs::filterDirs(searchOptions options)
{
	QMap<QString,QFileInfoList>::iterator it = m_FilesPerDir.begin();
	QMap<QString,QFileInfoList>::const_iterator itEnd = m_FilesPerDir.constEnd();
	int counter=0;
	
	while(it!=itEnd){
		QFileInfoList filteredList;
		QFileInfoList list = it.value();

		QFileInfoList::const_iterator itList=list.constBegin();
		QFileInfoList::const_iterator itEndList = list.constEnd();

		while(itList!=itEndList){
			QFileInfo fileInfo(*itList);
			QString msg("check " + fileInfo.fileName() + " " );
			if(!options.m_Keyword.isEmpty()) {
				if(!fileInfo.fileName().contains(QRegExp(options.m_Keyword))) {
					++itList;
					continue;//filteredList.append(fileInfo);
				}
			}
			if(options.m_Format == "jpeg") {
				if(fileInfo.suffix() == "jpeg" || fileInfo.suffix() == "jpg" ) {
					filteredList.append(fileInfo);
				}
			} else if(options.m_Format == "png") {
				if(fileInfo.suffix() == "png" ) {
					filteredList.append(fileInfo);
				}
			} else if(options.m_Format == "gif") {
				if(fileInfo.suffix() == "gif" ) {
					filteredList.append(fileInfo);
				}
			} else if(options.m_Format == "bmp") {
				if(fileInfo.suffix() == "bmp" ) {
					filteredList.append(fileInfo);
				}
			} else {
				filteredList.append(fileInfo);
			}
			qDebug() << msg;

			if(itList!=itEndList)
				++itList;
		}



		/*
		while(itList!=itEndList){
			QFileInfo fileInfo(*itList);
			QString msg("check " + fileInfo.fileName() + " " );
			if(!options.m_Keyword.isEmpty()) {
				if(!fileInfo.fileName().contains(QRegExp(options.m_Keyword)))
					list.erase(itList);
			}
			if(options.m_Format == "jpeg") {
				if(fileInfo.suffix() != "jpeg" && fileInfo.suffix() != "jpg" ) {
					if(itList != itEndList)
						list.erase(itList);
					msg += " removed " + fileInfo.baseName();
					counter++;
				}
			} else if(options.m_Format == "png") {
				if(fileInfo.suffix() != "png" ) {
					list.erase(itList);
					msg += " removed " + fileInfo.baseName() + "  " + fileInfo.suffix() + "   size: " + list.size();
					counter++;
				}
			} else if(options.m_Format == "gif") {
				if(fileInfo.suffix() != "gif" ) {
					list.erase(itList);
					msg += " removed " + fileInfo.baseName() + "  " + fileInfo.suffix() + "   size: " + list.size();
					counter++;
				}
			} else if(options.m_Format == "bmp") {
				if(fileInfo.suffix() != "bmp" ) {
					list.erase(itList);
					msg += " removed " + fileInfo.baseName() + "  " + fileInfo.suffix() + "   size: " + list.size();
					counter++;
				}
			}
			qDebug() << msg;

			if(itList!=itEndList)
				++itList;
		}*/

		/*QFileInfoList list = it.value();
		int size = list.size();
		qDebug() << list.size();
		for(int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo(list.at(i));
		QString msg("check " + fileInfo.fileName() + " " );
		if(options.m_Format == "jpeg") {
		if(fileInfo.suffix() != "jpeg" || fileInfo.suffix() != "jpg" ) {
		list.removeAt(i);
		msg += " removed " + fileInfo.baseName();
		counter++;
		}
		} else if(options.m_Format == "png") {
		if(fileInfo.suffix() != "png" ) {
		list.removeAt(i);
		msg += " removed " + fileInfo.baseName() + "  " + fileInfo.suffix();
		counter++;
		}
		}
		qDebug() << msg;
		}*/

		it.value() = filteredList;

		++it;
		//bClean=_load_dir(it.key(),it.value(),pCachedSort,bOnce);
	}
	qDebug() << "removed " << counter << "files";
}


void imageSortDirs::_refresh()
{
	if(m_bCacheKeyDirty){
		m_CacheKey.clear();

		QMap<QString,QFileInfoList>::const_iterator it=m_FilesPerDir.constBegin();
		while(it!=m_FilesPerDir.constEnd()){
			// V2.03: note that the cache key does NOT contain sym link names,
			// but the name of the sym link target...
			m_CacheKey+=it.key();
			++it;
		}
		
		m_bCacheKeyDirty=false;
	}
}

const QString& imageSortDirs::cacheKey()
{
	if(m_bCacheKeyDirty)
		_refresh();

	return m_CacheKey;
}

QFileInfoList imageSortDirs::filesInDir(const QString& absPath)
{
	// V2.03
	QFileInfo finfo(absPath);
	if(!finfo.isDir()){
		assert(false);
		return m_EmptyList;
	}
	// Note check if there can be a chain of links on os x
	// on win (xp), links onto links are not allowed...
	QString realAbsPath=absPath;
	if(finfo.isSymLink()&&finfo.exists())
		realAbsPath=finfo.symLinkTarget();

	if(!m_FilesPerDir.contains(realAbsPath))
		return m_EmptyList;

	return m_FilesPerDir.value(realAbsPath);

}

unsigned imageSortDirs::numFiles() const
{
	return(m_uTotalFiles);
}

bool imageSortDirs::dirty() const
{
	// check if all dirs in the map are in the clean set
	if(m_FilesPerDir.size()!=m_CleanDirs.size())
		return true;

	QSet<QString>::const_iterator it=m_CleanDirs.constBegin();
	QSet<QString>::const_iterator itEnd=m_CleanDirs.constEnd();
	while(it!=itEnd){
		if(!m_FilesPerDir.contains(*it))
			return true;
		++it;
	}
	return false;
}

QString imageSortDirs::firstDir() const
{
	if(empty())
		return QString("");

	QMap<QString,QFileInfoList>::const_iterator it=m_FilesPerDir.constBegin();
	assert(it!=m_FilesPerDir.constEnd());
	return it.key();

}

QString imageSortDirs::symLinkName(const QString& dir) const
{
	if(m_LinkNames.contains(dir))
		return m_LinkNames.value(dir);
	else
		return dir;
}

bool imageSortDirs::hasCleanSymLinks() const
{
	if(dirty())
		return false;

	QSet<QString>::const_iterator it=m_CleanDirs.constBegin();
	while(it!=m_CleanDirs.constEnd()){
		if(m_LinkNames.contains(*it))
			return true;
		++it;
	}
	return false;
}

void imageSortDirs::setClean()
{
	m_CleanDirs.clear();
	QList<QString> dirs=m_FilesPerDir.keys();
	QList<QString>::iterator it=dirs.begin();
	QList<QString>::const_iterator itEnd=dirs.constEnd();
	while(it!=itEnd){
		m_CleanDirs.insert(*it);
		++it;
	}
}
