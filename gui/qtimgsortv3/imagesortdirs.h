#pragma once
#include <QtCore>
// TODO V4: change issearchwidget.h, we don't need to include a widget (!!!) here
#include "issearchwidget.h"
/*!
a helper class holding the dirs to be sorted
*/
class ISMutex;
class imageSortDirs
{
public:
#pragma message("check if it easier to make imageSortDirs a Q_OBJECT (to have signales and slots)...")
	imageSortDirs();
	imageSortDirs(const imageSortDirs& rOther);
	imageSortDirs& operator=(const imageSortDirs& rOther);
	~imageSortDirs();
	/*!
	call this to add a dir

	nothing happens if the dir does not exist or already contained

	V2.03: dirs can be added by a sym link name now. internally, the dir is stored by
	the sym links target name. the cache key is internally composed of the sym link target names, too.
	Note you cannot addDir() a dir more than once, even by different sym link names...

	Note you can get the sym link name for a dir added by a sim link by calling symLinkName()

	Note due to a bug in Qt (QFileInfo() delivers uppercase Drivenames, QDir::absolutePath() delivers lowercase Drivenames,
	we have to build in a workaround here...

	pass pAbortFlag AND pMutex to enable abortion

	returns false if aborted
	*/
	bool addDir(const QString& absPath,bool bWithChildren=false,bool bFirstCall=true,bool *pAbortFlag=NULL,ISMutex* pMutex=NULL);
	/*!
	call this to remove a dir

	nothing happens if the dir not contained
	*/
	void removeDir(const QString& absPath);
	/*!
	call this to get a QString which describes all dirs inside

	note can be used with imageSortCache()

	note this is the concatenation of all strings, sorted by QString::operator<()
	*/
	const QString& cacheKey();
	/*!
	clears all dirs
	*/
	void clear();
	/*!
	tells if empty
	*/
	bool empty() const {return m_FilesPerDir.empty();}
	/*!
	returns the number of dirs contained
	*/
	unsigned size() const {return m_FilesPerDir.size();}
	/*!
	read access to the files per dir
	*/
	const QMap<QString,QFileInfoList>& filesPerDir() const {return m_FilesPerDir;}
	/*!
	get the name filter, i.e. the image files we can load

	note the name filter is set in the ctor
	*/
	const QStringList& nameFilter() const {return m_NameFilter;}
	// V4.x: no raw file support any longer...
	/*!
	sets the raw file extensions

	NOTE this will change nameFilter()
	*/
	//void setRawFileExts(const QStringList& exts);
	/*!
	returns the raw file extensions
	*/
	//const QStringList& rawFileExts() const {return m_RawImageExts;}
	/*!
	returns the files in the dir passed

	if the dir not contained, the list is empty
	*/
	QFileInfoList filesInDir(const QString& dir);
	/*!
	returns the total number of files contained
	*/
	unsigned numFiles() const;
	/*!
	tells if dirty

	the dirs are getting dirty on addDir() and removeDir()

	calling setClean() on all dirs added makes the dirs clean

	calling setDirty() makes the dirs dirty

	calling setDirty(const QString&) makes the dirs dirty

	*/
	bool dirty() const;
	/*!
	adds all dirs to the clean list

	afterwards, this is !dirty()
	*/
	void setClean();
	/*!
	adds dir to the clean list

	if all added dirs are in the clean list, this is !dirty()
	*/
	void setClean(const QString& dir) {m_CleanDirs.insert(dir);}
	/*!
	if dir is contained in the clean list, this will be dirty()
	afterwards
	*/
	void setDirty(const QString& dir) {m_CleanDirs.remove(dir);}
	/*!
	this will be dirty() afterwards
	*/
	void setDirty() {m_CleanDirs.clear();}
	/*!
	returns the name of the first dir, if any, otherwise an empty string
	*/
	QString firstDir() const;
	/*!
	return true if contained
	*/
	bool contains(const QString& dir) const {return m_FilesPerDir.contains(dir);}
	/*!
	returns the clean dirs
	*/
	QList<QString> cleanDirs() const {return m_CleanDirs.values();}
	/*!
	returns the top dirs
	*/
	QList<QString> topDirs() const {return m_TopDirs.toList();}
	/*!
	returns true if one or more of the clean dirs where added by a sym link name
	*/
	bool hasCleanSymLinks() const;
	/*!
	updates the dir, if included

	note the dirs will not get dirty by this,
	but the internal file info lists may change...
	*/
	void updateDir(const QString& dir);

	void filterDirs(searchOptions options);
	/*!
	returns the sym link name of the dir if the dir was added by a sym link name
	*/
	QString symLinkName(const QString &dir) const;
	/*!
	returns true if the extension passed is a raw file extesion

	note do not pass a leading '.'
	*/
	//bool isRawFileExt(const QString& ext) const {return m_RawImageExts.contains("*."+ext);}
protected:
	// the image files we can load
	// QStringList m_NameFilter;
	QStringList m_ImageExts;
	//QStringList m_RawImageExts;
	QStringList m_NameFilter;
	// tells if we have to recreate m_CacheKey;
	bool m_bCacheKeyDirty;
	QString m_CacheKey;
	QMap<QString,QFileInfoList> m_FilesPerDir;
	// note if we want to load recursivly, we also need to unload recursivly, thus store top dirs:
	QSet<QString> m_TopDirs;
	QSet<QString> m_ParentDirs;
	// V2.03
	// this holds the name of the link, if a dir was added by a link...
	QMap<QString,QString> m_LinkNames;
	QSet<QString> m_CleanDirs;
	void _refresh();
	unsigned m_uTotalFiles;
	// never touched:
	const QFileInfoList m_EmptyList;
};