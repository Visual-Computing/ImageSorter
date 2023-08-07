#if defined(_WIN32)

#include "filefinder.h"

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ismutex.h"

bool fileFinder::find(const QDir &dir,QList<QFileInfo> *pInfoList, const QStringList &filter,bool bFindDirs,bool *pAbortFlag,ISMutex* pMutex)
{
	pInfoList->clear();
	if(!dir.exists())
		return true;
	
	// we need to remove the leading '*' from the filters...
	QStringList filter2;
	if(!bFindDirs){
		for(int i=0;i<filter.size();++i){
			QString fstr=filter[i];
			fstr.remove("*");
			filter2.append(fstr);
		}
		// remove *any* empty strings
		int removed=filter2.removeAll("");
		assert(!removed);
	}


	QString path=dir.absolutePath();
	QString path2=path+"/";
	path+="\\*";
	path.replace("/","\\");
	QByteArray arr=path.toAscii();
	// plain win32 api:
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];  // directory specification
	DWORD dwError;

	//strncpy (DirSpec, argv[1], strlen(argv[1])+1);
	//strncat (DirSpec, "\\*", 3);
	//hFind = FindFirstFile(DirSpec, &FindFileData);
	assert(arr.size()<=MAX_PATH);
	hFind=FindFirstFile(arr.data(),&FindFileData);

	if(hFind==INVALID_HANDLE_VALUE){
		//printf ("Invalid file handle. Error is %u\n", GetLastError());
		//return (-1);
		return true;
	}
	else{
		QString str(FindFileData.cFileName);
		// ignore '.' and '..'
		if(str!="."&&str!=".."){
			QString str2=path2+str;
			if(!bFindDirs){
				if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)){
					if(filter.isEmpty()){
						QFileInfo f(str2);
						pInfoList->append(f);
					}
					else{
						for(int i=0;i<filter2.size();++i){
							if(str.endsWith(filter2[i])){
								QFileInfo f(str2);
								pInfoList->append(f);
								break;
							}
						}
					}
				}
			}
			else{
				if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
					// ignore filter
					QFileInfo f(str2);
					pInfoList->append(f);
					}
				}
			}
	}

	bool bAbort=false;
	while(FindNextFile(hFind,&FindFileData)!=0) {
		if(pAbortFlag&&pMutex){
			pMutex->lock();
			bAbort=*pAbortFlag;
			pMutex->unlock();
		}
		if(bAbort){
			FindClose(hFind);
			return false;
		}
		QString str(FindFileData.cFileName);
		// ignore '.' and '..'
		if(str!="."&&str!=".."){
			QString str2=path2+str;
			if(!bFindDirs){
				if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)){
					if(filter.isEmpty()){
						QFileInfo f(str2);
						pInfoList->append(f);
					}
					else{
						for(int i=0;i<filter2.size();++i){
							if(str.endsWith(filter2[i])){
								QFileInfo f(str2);
								pInfoList->append(f);
								break;
							}
						}
					}
				}
			}
			else{
				if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
					QFileInfo f(str2);
					pInfoList->append(f);
				}
			}
		}
	}

	dwError=GetLastError();
	FindClose(hFind);
	if(dwError!=ERROR_NO_MORE_FILES){
		//printf ("FindNextFile error. Error is %u\n", dwError);
		pInfoList->clear();
	}
	return true;

}

#endif // _WIN32