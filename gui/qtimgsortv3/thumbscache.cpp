#include "thumbscache.h"
#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <assert.h>

thumbscache::thumbscache() : m_bInitialized(false)
{
	// set up cache directory
	QString cachePath=QCoreApplication::applicationDirPath();

	cachePath+="/thumbscache/";
	
	m_CacheRootDir.setPath(cachePath);

	// if the subfolder does not exist, create it...
	if(!m_CacheRootDir.exists()){
		if(!m_CacheRootDir.cd(QCoreApplication::applicationDirPath())){
			assert(false);
			return;
		}
		if(!m_CacheRootDir.mkdir("thumbscache")){
			assert(false);
			return;
		}
	}

	m_CacheRootDir.setPath(cachePath);
	m_CacheFileExt=".isd";
	
	m_bInitialized=true;

	return;

}

bool thumbscache::load(const QString& str,thumbnail& thumb)
{
	if(!initialized()||str.isEmpty())
		return false;
	
	QFileInfo finfo(str);
	
	if(!finfo.exists()){
		assert(false);
		return false;
	}

	QString cachePath=finfo.absolutePath();
	createCachePath(cachePath);

	// check if cache directory exists
	QDir cacheDir=m_CacheRootDir;
	if(!cacheDir.cd(cachePath))
		return false;

	// check if file exists
	QString fname=finfo.baseName();
	fname+=m_CacheFileExt;

	if(!cacheDir.exists(fname))
		return false;

	// load content
	QFile cacheFile(cacheDir.absolutePath()+'/'+fname);

	if(!cacheFile.open(QIODevice::ReadOnly)){
		assert(false);
		return false;
	}

	QDataStream iStr(&cacheFile);

	iStr >> thumb;

	if(iStr.status()!=QDataStream::Ok){
		assert(false);
		return false;
	}

	// now check if the *originals* files size and date
	// differ from the data stored in the thumbnail
	// or the thumbnail is out of version

	if(finfo.size()!=thumb.m_FileSize||finfo.lastModified()!=thumb.m_LastModified||thumb.isOutVersioned()){
		// delete the cache file, no longer valid
		bool b=cacheFile.remove();
		assert(b);
		return false;
	}

	return true;
}


bool thumbscache::save(const thumbnail& thumb)
{
	// do not cache thumbs from the net...
	if(!initialized()||thumb.isLoadedFromNet())
		return false;

	// just an abbrev:
	QFileInfo finfo(thumb.m_FileName);

	// check if original (still) exists
	// ... well, we may leave this test out...
	//if(!finfo.exists())
	//	return false;
	
	QString cachePath=finfo.absolutePath();
	createCachePath(cachePath);

	// check if cache directory exists
	QDir cacheDir=m_CacheRootDir;
	if(!cacheDir.cd(cachePath)){
		// create it (note these may be several subfolders)
		if(!m_CacheRootDir.mkpath(cachePath)){
			assert(false);
			return false;
		}
		if(!cacheDir.cd(cachePath)){
			assert(false);
			return false;
		}
	}

	// open file for writing
	QString fname=finfo.baseName();
	fname+=m_CacheFileExt;
	QFile cacheFile(cacheDir.absolutePath()+'/'+fname);

	if(!cacheFile.open(QIODevice::WriteOnly)){
		assert(false);
		return false;
	}

	QDataStream oStr(&cacheFile);

	oStr << thumb;

	if(oStr.status()!=QDataStream::Ok){
		// writing failed, delete the file
		bool b=cacheFile.remove();
		assert(b);
		return false;
	}

	return true;
	
}

void thumbscache::createCachePath(QString &cachePath)
{
	// note on os x, the absolute file path always starts with '/'
	// on windows, it start's either with an uppercase drive letter e.g. 'D:/' 
	// or with two trailing slashes followed by a network share, e.g. '//sharename/'

#if defined Q_WS_MAC || defined Q_OS_UNIX
	// in case of unix/os x, we just remove the '/', because the path is simply a subfolder of our root cache path
	assert(cachePath.startsWith('/'));
	cachePath.remove(0,1);
#elif defined Q_WS_WIN
	// in case of windows, we replace the driveletter by drive_driveletter, (eg. 'drive_D for 'D:')
	// ore the network share with share_networkshare (e.g. 'share_oberon' for '//oberon')
	if(cachePath.startsWith("//")){
		cachePath.remove(0,2);
		cachePath.prepend("share_");
	}
	else{
		assert(!cachePath.startsWith('/'));
		assert(cachePath[1]==':');
		cachePath.remove(1,1);
		cachePath.prepend("drive_");
	}
#endif

}