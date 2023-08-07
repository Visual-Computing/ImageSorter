#pragma once

#if defined(_WIN32)
// since QDir::entryInfoList() is soo slow on win32, we try to use the win32 api directly...
#include <QDir>
#include <QList>
#include <QString>
#include <QFileInfo>
class ISMutex;
class fileFinder
{
public:
	fileFinder(){;}
	~fileFinder(){;}
	/*!
	find all files in dir an fill the list passed
	if there are no files or the dir does not exist, the list is empty afterwards...
	if filter is not empty and bFindDirs==FALSE, the filters are applied
	if bFindDirs==TRUE, the filter is ignored and sub dirs are found, if any...

	pass pAbortFlag AND pMutex to enable abortion

	returns false if aborted
	*/
	bool find(const QDir& dir,QList<QFileInfo>* pInfoList,const QStringList& filter,bool bFindDirs=FALSE,bool *pAbortFlag=NULL,ISMutex* pMutex=NULL);
};
#endif