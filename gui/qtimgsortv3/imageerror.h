#pragma once
#include <string>
using namespace std;
#include <cstdio>

/*!
an error class for images and helper classes to be thrown as exceptions

this error class can be thrown unqualified, in this case causeStr() and causeCStr() will be "unknown cause", 
srcFileStr(), srcFileCStr(), srcLineStr() and srcLineCStr() will be "unknown", 
srcLine() will be -1. use the standard ctor imageError() for such an exception.

it can be thrown partly qualified, in this case causeStr() and causeCStr() will describe the error, 
srcFileStr(), srcFileCStr(), srcLineStr() and srcLineCStr() will be "unknown" and srcLine() will be -1. 
use ctor imageError(eCause) for such an exception.

it can be thrown fully qualified, in this case causeStr() and causeCStr() will describe the error, 
srcFileStr() and srcFileCStr() will tell the file name, srcLineStr(), srcLineCStr() and srcLine() will be 
the line number where the exception was thrown. 
best use the makro fqImageError(cause) which expands to ctor imageError(eCause,const char*,int) by using 
the ansi makros __FILE__ and __LINE__ for such an exception.

you may also use the non-parameter makro qImageError(), this sets srcFileStr(), srcFileCStr(), srcLineStr(), 
srcLineCStr() and srcLine() by __FILE__ and __LINE__ and leaves causeStr() and causeCStr() "unknown", which
is propably better than imageError()...
*/
class imageError
{
public:
	/*!
	the cause descriptor
	*/
	enum eCause{
		unKnown=0,
		alreadyCreated,
		badParameter,
		badAlloc,
		imageNotCreated,
		imageNotSameDim,
		notImplemented,
		illegalBaseCall,
		selfReference,
		imageNotSameLayout,
		badCall,
		badTemplateType,
		notAllocated,
		imageNotAsRequested,
		badChannelLayout,
		tiffOpen,
		tiffBadTags,
		tiffRead,
		tiffWrite,
		tiffBadColorSpace,
		badColorSpace,
		badRoi,
		roiNotImplemented,
		maxCountExceeded,
		imageTooBig,
		imageStatsNotCreated,
		imageIdExceed,
		imageBadSize,
		internalError,
		badRoiStack,
		badKernelSize,
		imageBorderTooBig,
		badTemplateParameter,
		instanceNotInitialized,
		badChannelLayoutStack,
		jpegError,
		lastErrorCause,			// note has to be *always* the last enum value...
	};
	/*!
	ctor for a fully unqualified imageError
	*/
	imageError() {m_eCause=unKnown;m_sSrcFile="unknown";m_nSrcLine=-1;lineStr();m_sAddInfo.empty();}
	/*!
	ctor for an imageError with known cause(), but unknown srcFile() and srcLine()
	*/
	imageError(eCause cause) {m_eCause=cause;m_sSrcFile="unknown";m_nSrcLine=-1;lineStr();m_sAddInfo.empty();}
	/*!
	ctor for a fully qualified imageError

	note use the __FILE__ and __LINE__ ansi makros for pSrcFile and srcLine, 
	or use the fqImageError(cause) makro which expands __FILE_ and __LINE__ automatically
	*/
	imageError(eCause cause,const char* pSrcFile,int srcLine,const char*addInfo=NULL);
	virtual ~imageError() {m_sSrcFile.clear();m_sSrcLine.clear();m_sAddInfo.clear();}
	/*!
	returns the cause of the exception
	*/
	eCause cause() const {return m_eCause;}
	/*!
	returns the cause of the exception as a <string>
	*/
	const string& causeStr() const;
	/*!
	returns the cause of the exception as a c-string
	*/
	const char* causeCStr() const {return causeStr().c_str();}
	/*!
	returns the source file name where the exception was thrown, if known, otherwise "unknown",
	as a <string>
	*/
	const string& srcFileStr() const {return m_sSrcFile;}
	/*!
	returns the source file name where the exception was thrown, if known, otherwise "unknown",
	as a c-string
	*/
	const char* srcFileCStr() const {return m_sSrcFile.c_str();}
	/*!
	returns the source line number in srcFile() at which the exception was thrown, if known, otherwise -1
	*/
	int srcLine() const {return m_nSrcLine;}
	/*!
	returns the source line number in srcFile() at which the exception was thrown as a string, 
	if known, otherwise "-1"
	*/
	const string& srcLineStr() const {return m_sSrcLine;}
	/*!
	returns the source line number in srcFile() at which the exception was thrown as a c-string, 
	if known, otherwise "-1"
	*/
	const char* srcLineCStr() const {return m_sSrcLine.c_str();}
	/*!
	returns the additional info string
	*/
	const string& addInfoStr() const {return m_sAddInfo;}
	/*!
	returns the additional info string as a c-string
	*/
	const char* addInfoCStr() const {return m_sAddInfo.c_str();}

protected:
	eCause m_eCause;
	string m_sSrcFile;
	int m_nSrcLine;
	string m_sSrcLine;
	string m_sAddInfo;
private:
	//cretaes m_sSrcLine from m_nSrcLine, used in ctors...
	void lineStr();
	static const string m_strUnKnown;
	static const string m_strAlreadyCreated;
	static const string m_strBadParameter;
	static const string m_strBadAlloc;
	static const string m_strImageNotCreated;
	static const string m_strImageNotSameDim;
	static const string m_strNotImplemented;
	static const string m_strIllegalBaseCall;
	static const string m_strSelfReference;
	static const string m_strImageNotSameLayout;
	static const string m_strBadCall;
	static const string m_strBadTemplateType;
	static const string m_strNotAllocated;
	static const string m_strImageNotAsRequested;
	static const string m_strBadChannelLayout;
	static const string m_strTiffOpen;
	static const string m_strTiffBadTags;
	static const string m_strTiffRead;
	static const string m_strTiffWrite;
	static const string m_strTiffBadColorSpace;
	static const string m_strBadColorSpace;
	static const string m_strBadRoi;
	static const string m_strRoiNotImplemented;
	static const string m_strMaxCountExceeded;
	static const string m_strImageTooBig;
	static const string m_strImageStatsNotCreated;
	static const string m_strImageIdExceed;
	static const string m_strImageBadSize;
	static const string m_strInternalError;
	static const string m_strBadRoiStack;
	static const string m_strNoErrorStr;
	static const string m_strBadKernelSize;
	static const string m_strImageBorderTooBig;
	static const string m_strBadTemplateParameter;
	static const string m_strInstanceNotInitialized;
	static const string m_strBadChannelLayoutStack;
	static const string m_strLastCause;
};

 
/*!
a makro which expands to the fully qualified ctor imageError(eCause,const char*,unsigned)
by using __FILE__ and __LINE__ ansi makros
*/

#define fqImageError(cause) imageError(cause,__FILE__,__LINE__,NULL)

//#if defined(_MSC_VER)
//#define fqImageError(cause) imageError(##cause,__FILE__,__LINE__,NULL)
//#elif defined(__GNUC__)
//	#define fqImageError(cause) imageError(cause,__FILE__,__LINE__,NULL)
//#else
//#error neither GNU nor MSC compiler
//#endif




/*!
a makro which expands to the fully qualified ctor imageError(eCause,const char*,unsigned)
by using __FILE__ and __LINE__ ansi makros, but eCause will be imageError::unKnown 
this is for really lazy ones...
*/
#define qImageError() fqImageError(imageError::unKnown)