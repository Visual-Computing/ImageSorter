#ifndef THUMBSCACHE_H
#define THUMBSCACHE_H
#include "thumbnail.h"
#include <QString>
#include <QDir>

class thumbscache
{
public:
	thumbscache(); 
	~thumbscache(){;}
	bool initialized() const {return m_bInitialized;}
	/*!
	loads a thumbnail from the cache

	finfo is the file info for the *original* image

	returns false if !initialized(), the original file does not exist, the cache folder or file does not exist,
	the file size or last recently used time stamp of finfo differ from the data stored in thumb (in this case the
	cache file is deleted).

	if false is returned, the content of thumb may be undefined afterwards.
	*/
	bool load(const QString& str,thumbnail& thumb);
	/*!
	saves a thumbnail to the cache

	returns false if !initialized(), the thumb was loaded from the net, the original file does not exist, the cache folder or
	file do not exist and cannot be created or the saving operation failed.
	*/
	bool save(const thumbnail& thumb);
	/*!
	validates the cache

	deletes all cache files for which the original file does not exist any longer

	note: not yet implemented
	*/
	bool validateCache() {return true;}
protected:
	void createCachePath(QString& path);
	QDir m_CacheRootDir;
	bool m_bInitialized;
	QString m_CacheFileExt;
};
#endif