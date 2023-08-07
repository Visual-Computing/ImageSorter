#include "imageerror.h"

const string imageError::m_strUnKnown="unknown cause";
const string imageError::m_strAlreadyCreated="instance already created";
const string imageError::m_strBadParameter="bad parameter";
const string imageError::m_strBadAlloc="operator new failed";
const string imageError::m_strImageNotCreated="image not created";
const string imageError::m_strImageNotSameDim="image not same dimension";
const string imageError::m_strNotImplemented="not implemented";
const string imageError::m_strIllegalBaseCall="illegal base call";
const string imageError::m_strSelfReference="self reference";
const string imageError::m_strImageNotSameLayout="images not same layout";
const string imageError::m_strBadCall="bad call";
const string imageError::m_strBadTemplateType="bad template type";
const string imageError::m_strNotAllocated="not allocated";
const string imageError::m_strImageNotAsRequested="image not as requested";
const string imageError::m_strBadChannelLayout="image bad channel layout";
const string imageError::m_strTiffOpen="tiff open failed";
const string imageError::m_strTiffBadTags="bad tiff tags";
const string imageError::m_strTiffRead="tiff read failed";
const string imageError::m_strTiffWrite="tiff write failed";
const string imageError::m_strTiffBadColorSpace="bad tiff colorspace";
const string imageError::m_strBadColorSpace="bad colorspace";
const string imageError::m_strBadRoi="bad roi";
const string imageError::m_strRoiNotImplemented="not implemented for roi";
const string imageError::m_strMaxCountExceeded="maximum count exceeded";
const string imageError::m_strImageTooBig="image too big";
const string imageError::m_strImageStatsNotCreated="image stats not created";
const string imageError::m_strImageIdExceed="too many images (id exceed)";
const string imageError::m_strImageBadSize="bad image size";
const string imageError::m_strBadRoiStack="roi stack underflow";
const string imageError::m_strInternalError="internal error";
const string imageError::m_strNoErrorStr="no error description available";
const string imageError::m_strLastCause="internal error: last cause";
const string imageError::m_strBadKernelSize="bad kernel size";
const string imageError::m_strImageBorderTooBig="image border too big";
const string imageError::m_strBadTemplateParameter="bad template parameter";
const string imageError::m_strInstanceNotInitialized="instance not initialized";
const string imageError::m_strBadChannelLayoutStack="channel layout stack underflow";

imageError::imageError(eCause cause,const char* pSrcFile,int srcLine,const char* addInfo)
{
	m_eCause=cause;
	m_sSrcFile=pSrcFile;
	m_nSrcLine=srcLine;
	lineStr();
	if(addInfo)
		m_sAddInfo=addInfo;
	else
		m_sAddInfo.empty();
}

void imageError::lineStr()
{
	m_sSrcLine.empty();
	if(m_nSrcLine<0)
		m_sSrcLine="unknown";
	else{
		// src line is a positive int, thus can be up to 10 digits...
		// ...which is quite a long src file...
		char s[11];
#if defined(_MSC_VER)
			_snprintf(s,10,"%d",m_nSrcLine);
#elif defined(__GNUC__)
			snprintf(s,10,"%d",m_nSrcLine);
#else
#error neither GNU nor MSC compiler
#endif
		m_sSrcLine=s;
	}
}


const string& imageError::causeStr() const
{
	switch(m_eCause){
		case unKnown:
			return m_strUnKnown;
			break;
		case alreadyCreated:
			return m_strAlreadyCreated;
			break;
		case badParameter:
			return m_strBadParameter;
			break;
		case badAlloc:
			return m_strBadAlloc;
			break;
		case imageNotCreated:
			return m_strImageNotCreated;
			break;
		case imageNotSameDim:
			return m_strImageNotSameDim;
			break;
		case notImplemented:
			return m_strNotImplemented;
			break;
		case illegalBaseCall:
			return m_strIllegalBaseCall;
			break;
		case selfReference:
			return m_strSelfReference;
			break;
		case imageNotSameLayout:
			return m_strImageNotSameLayout;
			break;
		case badCall:
			return m_strBadCall;
			break;
		case badTemplateType:
			return m_strBadTemplateType;
			break;
		case notAllocated:
			return m_strNotAllocated;
			break;
		case imageNotAsRequested:
			return m_strImageNotAsRequested;
			break;
		case badChannelLayout:
			return m_strBadChannelLayout;
			break;
		case tiffOpen:
			return m_strTiffOpen;
			break;
		case tiffBadTags:
			return m_strTiffBadTags;
			break;
		case tiffRead:
			return m_strTiffRead;
			break;
		case tiffWrite:
			return m_strTiffWrite;
			break;
		case tiffBadColorSpace:
			return m_strTiffBadColorSpace;
			break;
		case badColorSpace:
			return m_strBadColorSpace;
			break;
		case badRoi:
			return m_strBadRoi;
			break;
		case roiNotImplemented:
			return m_strRoiNotImplemented;
			break;
		case maxCountExceeded:
			return m_strMaxCountExceeded;
			break;
		case imageTooBig:
			return m_strImageTooBig;
			break;
		case imageStatsNotCreated:
			return m_strImageStatsNotCreated;
			break;
		case imageIdExceed:
			return m_strImageIdExceed;
			break;
		case imageBadSize:
			return m_strImageBadSize;
			break;
		case internalError:
			return m_strInternalError;
			break;
		case badRoiStack:
			return m_strBadRoiStack;
			break;
		case badKernelSize:
			return m_strBadKernelSize;
			break;
		case lastErrorCause:
			return m_strLastCause;
			break;
		case instanceNotInitialized:
			return m_strInstanceNotInitialized;
			break;
		case badChannelLayoutStack:
			return m_strBadChannelLayoutStack;
			break;
		default:
			return m_strNoErrorStr;
			break;
	}
	// hopefully never reached:
	return m_strNoErrorStr;
}

