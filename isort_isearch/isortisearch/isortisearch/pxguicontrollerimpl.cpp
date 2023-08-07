#include "pxguicontrollerimpl.h"
#include "pximage.h"
#include "pxmutex.h"
#include <math.h>
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif

// TODO: take out in release
//#include <iostream>

namespace isortisearch{

template <typename TIMG> const unsigned __int32 guiControllerImpl<TIMG>::m_uMajorVersion=2;
template <typename TIMG> const unsigned __int32 guiControllerImpl<TIMG>::m_uMinorVersion=0;
// see refPtr<T> standard ctor:
template <typename TIMG> refPtr<TIMG> guiControllerImpl<TIMG>::m_RefPtrNull;

template <typename TIMG> guiControllerImpl<TIMG>::guiControllerImpl() : 
	m_CanvasXSize(0),
	m_CanvasYSize(0),
	m_ThumbSpacing(0.1),
	m_MouseXPos(0),
	m_MouseYPos(0),
	m_MouseLastXPos(0),
	m_MouseLastYPos(0),
	m_MouseMoveX(0),
	m_MouseMoveY(0),
	m_MousePressXPos(0),
	m_MousePressYPos(0),
	m_dZoomFactor(1.0),
	m_dMaxZoomFactor(200.),
	m_dLastZoomFactor(1.0),
	m_dZoomStep(1.1),
	m_bMouseDown(false),
	m_bFirstTimeColor(true),
	m_pPaintRequestCallBack(NULL),
	m_pPaintRequestUserData(NULL),
	m_uClickSensitivity(5),
	m_ControllerMode(SORTED),
	m_bResized(false),
	m_bResizedAndResorted(false),
	m_dPyramidFactor(0.85),
	m_pThumbBoxSizesX(NULL),
	m_pThumbBoxSizesY(NULL),
	m_pImgsPerRow(NULL),
	m_pSorterImages(NULL),
	m_pSorterMutex(NULL),
	m_MapPlacesX(0),
	m_MapPlacesY(0),
	m_pSizeChangedCallback(NULL),
	m_pSizeChangedUserData(NULL),
	m_pSearcherImages(NULL),
	m_pSearcherResults(NULL),
	m_pSearcherMutex(NULL),
	m_bAttachedToSearcher(false),
	m_bAttachedToSorter(false),
	m_bAttachedToDynamicSearcher(false),
	m_pDynamicSearcherResults(NULL),
	m_pDynamicSearcherMutex(NULL)

	//m_bSearchResultsVisibleOnly(true)
{
	;
}

template <typename TIMG> bool guiControllerImpl<TIMG>::setCanvasSize(__int32 x,__int32 y,bool bRedraw) 
{
	if(x<=0||y<=0)
		return false;

	m_CanvasXSize=x;
	m_CanvasYSize=y;

	// inform the sorter that the size has changed
	if(m_pSizeChangedCallback)
		m_pSizeChangedCallback(m_pSizeChangedUserData);

	m_bResized=true;

	if(bRedraw)
		updateImagePositions();

	return true;
}


template <typename TIMG> void guiControllerImpl<TIMG>::setImageSpacing(double val) 
{
	val=__max(0.0,val);
	val=__min(0.5,val);
	m_ThumbSpacing=val;
}

template <typename TIMG> void guiControllerImpl<TIMG>::onMouseDown(__int32 xPos,__int32 yPos)
{
	m_bMouseDown=true;
	m_MousePressXPos=__max(0,xPos);
	m_MousePressYPos=__max(0,yPos);

	// init mouse dragging 
	m_MouseLastXPos=m_MousePressXPos;
	m_MouseLastYPos=m_MousePressYPos;

}

template <typename TIMG> void guiControllerImpl<TIMG>::onMouseMove(__int32 xPos,__int32 yPos)
{
	// Kai (always store the mouse position! Otherwise centered zoom will no work properly)
	// store current mouse position
	m_MouseXPos=__max(0,xPos);
	m_MouseYPos=__max(0,yPos);

	if(!m_bMouseDown)
		return;

	m_MouseMoveX = m_MouseXPos - m_MouseLastXPos; 
	m_MouseMoveY = m_MouseYPos - m_MouseLastYPos;
	m_MouseLastXPos = m_MouseXPos;
	m_MouseLastYPos = m_MouseYPos;
	updateImagePositions();
	m_MouseMoveX = 0; 
	m_MouseMoveY = 0;
	return;

}

template <typename TIMG> refPtr<TIMG> guiControllerImpl<TIMG>::onMouseUp(__int32 xPos,__int32 yPos,bool bRedraw,bool bReturnImage)
{
	m_bMouseDown=false;

	if(bRedraw)
		updateImagePositions();

	xPos=__max(0,xPos);
	yPos=__max(0,yPos);

	if(bReturnImage){
		// if moved only slightly, return the adress of the clicked image
		__int32 xdist=(__int32)xPos-m_MousePressXPos;
		if(xdist<0)
			xdist*=-1;
		__int32 ydist=yPos-m_MousePressYPos;
		if(ydist<0)
			ydist*=-1;

		if((unsigned)xdist+(unsigned)ydist<m_uClickSensitivity){
			return getImageAt(xPos,yPos);
		}
	}

	return m_RefPtrNull;
}

template <typename TIMG> void guiControllerImpl<TIMG>::onZoomIn()
{
	const double current=m_dZoomFactor;
	m_dZoomFactor*=m_dZoomStep;
	m_dZoomFactor=std::min(m_dZoomFactor,m_dMaxZoomFactor);
	
	// something to do?
	if(current==m_dZoomFactor)
		return;
	
	updateImagePositions();
	return;
}
template <typename TIMG> void guiControllerImpl<TIMG>::onZoomOut()
{
	const double current=m_dZoomFactor;

	m_dZoomFactor/=m_dZoomStep;
	m_dZoomFactor=std::max(m_dZoomFactor,1.0);

	// something to do?
	if(current==m_dZoomFactor)
		return;

	updateImagePositions();

	return;
}
template <typename TIMG> bool guiControllerImpl<TIMG>::setZoomStep(double step)
{
	if(step<=1.0)
		return false;
	m_dZoomStep=step;
	return true;
}
template <typename TIMG> bool guiControllerImpl<TIMG>::setMaxZoomFactor(double f)
{
	if(f<1.0)
		return false;
	m_dMaxZoomFactor=f;
	return true;
}

template <typename TIMG> void guiControllerImpl<TIMG>::onZoom(double factor)
{
	if(factor==m_dZoomFactor)
		return;

	factor=std::max(1.0,factor);
	factor=std::min(m_dMaxZoomFactor,factor);
	m_dZoomFactor=factor;
	updateImagePositions();
	return;
}

template <typename TIMG> void guiControllerImpl<TIMG>::updateImagePositions(bool bCallBack)
{
	if(m_ControllerMode==SORTER_INPUT||m_ControllerMode==SEARCHER_INPUT||m_ControllerMode==LIST_SEARCH_RESULTS||m_ControllerMode==LIST_DYNAMIC_SEARCH_RESULTS){
		updateImagePositionsList(bCallBack);
		return;
	}
	if(m_ControllerMode==PYRAMID_SEARCH_RESULTS||m_ControllerMode==PYRAMID_DYNAMIC_SEARCH_RESULTS){
		updateImagePositionsPyramid(bCallBack);
		return;
	}

	//std::cout << "MouseMove x: " << m_MouseMoveX << " y: " << m_MouseMoveY << "\n";
	// if we are not attached to a sorter, this does nothing (we need the map places...)
	//if(!m_pSorter||m_pSorter->mapPlacesX()==0||m_pSorter->mapPlacesY()==0||!m_pSorter->m_pSortImages||m_pSorter->m_pSortImages->empty()){
	if(m_MapPlacesX==0||m_MapPlacesY==0||!m_pSorterImages||!(*m_pSorterImages)||!(*m_pSorterImages)->size()){
		// even redraw if we have no images, clear() may has been called...
		// TODOV5: check if there still is clear()...
		if(bCallBack){
			assert(m_pPaintRequestCallBack);
			m_pPaintRequestCallBack(m_pPaintRequestUserData);
		}
		return;
	}
	assert(m_pSorterMutex);
	assert(*m_pSorterMutex);
	(*m_pSorterMutex)->lock();
	
	const unsigned mapPlacesX=m_MapPlacesX;
	const unsigned mapPlacesY=m_MapPlacesY;

	//static double zoomFactorLast=1.;
	static double xm,ym;
	const double borderFactor = 1.-m_ThumbSpacing; 

	// reset everthing if this mode is called for the first time
	if(m_bFirstTimeColor) {
		xm = ym = 0; 
		//zoomFactorLast = m_dZoomFactor;
		m_bFirstTimeColor = false;
	}
	else if(m_bResizedAndResorted){
		xm=ym=0;
		m_bResized=m_bResizedAndResorted=false;
	}
	
	const double scaledCanvasXSize = m_CanvasXSize*m_dZoomFactor;
	const double scaledCanvasYSize = m_CanvasYSize*m_dZoomFactor;

	// size of a scaled thumbnail area
	const double scaledThumbBoxSizeX=scaledCanvasXSize/(double)mapPlacesX;
	const double scaledThumbBoxSizeY=scaledCanvasYSize/(double)mapPlacesY; 

	
	// we do all calculations in __int32 here, because 
	// we also need negative values. Note that the canvas sizes
	// are restricted to 0x7FFFFFFF, therefore we 
	// can cast the unsigneds to int here safely...
	// TODOV5: make sure this is true...
	
	//const double zoomChange = m_dZoomFactor/zoomFactorLast-1;
	const double zoomChange = m_dZoomFactor/m_dLastZoomFactor-1;
	
	const double xDelta=-m_MouseXPos*zoomChange;
	const double yDelta=-m_MouseYPos*zoomChange;
	//zoomFactorLast = m_dZoomFactor;
	m_dLastZoomFactor = m_dZoomFactor;

	xm -= (m_MouseMoveX+xDelta)/scaledThumbBoxSizeX;
	ym -= (m_MouseMoveY+yDelta)/scaledThumbBoxSizeY;

	//xm = (xm + mapPlacesX)%mapPlacesX;
	//ym = (ym + mapPlacesY)%mapPlacesY;
	xm = fmod(xm + mapPlacesX, mapPlacesX);
	ym = fmod(ym + mapPlacesY, mapPlacesY);

	//std::cout << "xm: " << xm << " ym: " << ym << "\n";

	const double thumbnailSizeX = scaledThumbBoxSizeX*borderFactor;
	const double thumbnailSizeY = scaledThumbBoxSizeY*borderFactor;

	// Zeichenposition errechnen
	int startPosX = (int) (-xm*scaledThumbBoxSizeX);
	int startPosY = (int) (-ym*scaledThumbBoxSizeY);

	//std::vector<TIMG*>::iterator it=(*m_pSorterImages)->begin();
	//std::vector<TIMG*>::iterator itEnd=(*m_pSorterImages)->end();
#ifdef __GNUC__
	typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSorterImages)->begin();
	typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSorterImages)->end();
#else
	std::vector<refPtr<TIMG>>::iterator it=(*m_pSorterImages)->begin();
	std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSorterImages)->end();
#endif

	for(;it<itEnd;++it){

		controllerImage& img=**it;
		
		// lock here
		// TODOV5: check if really required...
		//m_pMutex->lock();		
		int xPos=img.m_uMapXPos;
		int yPos=img.m_uMapYPos;
		//m_pMutex->unlock();

		double xBoxStart = startPosX + xPos*scaledThumbBoxSizeX;
		// verschiebe Bild, wenn es rechts bzw. links aus dem Canvas laeuft
		if( xBoxStart - scaledCanvasXSize + scaledThumbBoxSizeX > m_CanvasXSize - xBoxStart ) 
			xBoxStart -= scaledCanvasXSize;  
		else if (m_CanvasXSize - (xBoxStart + scaledCanvasXSize) > xBoxStart + scaledThumbBoxSizeX) 
			xBoxStart += scaledCanvasXSize;

		double yBoxStart = startPosY + yPos*scaledThumbBoxSizeY;
		// verschiebe Bild, wenn es oben bzw. unten aus dem Canvas lŠuft
		if( yBoxStart - scaledCanvasYSize + scaledThumbBoxSizeY > m_CanvasYSize - yBoxStart ) 
			yBoxStart -= scaledCanvasYSize; 
		else if (m_CanvasYSize - (yBoxStart + scaledCanvasYSize) > yBoxStart + scaledThumbBoxSizeY) 
			yBoxStart += scaledCanvasYSize; 
		
		int imgOrigWidth = img.width();
		int imgOrigHeight = img.height();

		// scale, keep aspect ratio
		const double scale = __min(thumbnailSizeX/imgOrigWidth,thumbnailSizeY/imgOrigHeight);  
		int xLen = (int)(scale*imgOrigWidth + 0.5);
		// note if the original image has a very high or low aspect ration, we might end in a 
		// scaled dim of 0. Set it to 1 in this case...
		xLen=std::max(xLen,1);
		
		int yLen = (int)(scale*imgOrigHeight + 0.5);
		yLen=std::max(yLen,1);

		int xs = (int) (xBoxStart + (scaledThumbBoxSizeX - xLen)/2); // xStart mit Rand
		int xe=xs+xLen;		
		int ys = (int) (yBoxStart + (scaledThumbBoxSizeY - yLen)/2); // yStart mit Rand	
		int ye=ys+yLen;

		// TODOV5: do we need to lock this, too?
		if (!(xs > (int)m_CanvasXSize || xe <= 0 || ys > (int)m_CanvasYSize || ye <= 0) ) {
			// make visible
			img.m_uFlags|=isortisearch::controllerImage::m_VisibleFlag;
			// set drawing positions
			img.m_UpperLeftX = xs;
			img.m_LowerRightX =xe;
			img.m_UpperLeftY  =ys;
			img.m_LowerRightY =ye;
		}
		else
			img.m_uFlags&=isortisearch::controllerImage::m_VisibleMask;
	}
	(*m_pSorterMutex)->unlock();
		
	// tell the client to redraw...
	if(bCallBack){
		assert(m_pPaintRequestCallBack);
		m_pPaintRequestCallBack(m_pPaintRequestUserData);
	}
}


template <typename TIMG> void guiControllerImpl<TIMG>::updateImagePositionsPyramid(bool bCallBack)
{
	isortisearch::mutex* pMutex;
	// note the pyramid mode is only for searched images
	if(m_ControllerMode==PYRAMID_SEARCH_RESULTS){
		if(!m_pSearcherResults||!m_pSearcherResults->size()){
			// even redraw if we have no images, clear() may have been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
	assert(m_pSearcherMutex);
	assert(*m_pSearcherMutex);
	pMutex=*m_pSearcherMutex;
	}
	else if(m_ControllerMode==PYRAMID_DYNAMIC_SEARCH_RESULTS){
		if(!m_pDynamicSearcherResults||!m_pDynamicSearcherResults->size()){
			// even redraw if we have no images, clear() may have been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
	assert(m_pDynamicSearcherMutex);
	assert(*m_pDynamicSearcherMutex);
	pMutex=*m_pDynamicSearcherMutex;
	}
	else{
		assert(false);
		return;
	}


	pMutex->lock();

	//static double zoomFactorLast=1.;
	static double xm=0,ym=0;
	const double borderFactor = 1.-m_ThumbSpacing; 

	// reset everthing if this mode is called for the first time
	if(m_bFirstTimeColor) {
		xm = ym = 0; 
		//zoomFactorLast = m_dZoomFactor;
		m_bFirstTimeColor = false;
	}
	else if(m_bResizedAndResorted){
		xm=ym=0;
		m_bResized=m_bResizedAndResorted=false;
	}

	// here as an example we assume we want to show half of the images in pyramid mode
	int desiredResultNumber=(m_ControllerMode==PYRAMID_SEARCH_RESULTS)?m_pSearcherResults->size():m_pDynamicSearcherResults->size();
	// next we calculate the best fitting number of images (close to the desired one)
#pragma message("results bigger than desiredResultNumber ???")
	desiredResultNumber = getIdealPyramidResultNumber(desiredResultNumber);

	calculatePyramidLayout(desiredResultNumber);

	calculateDrawingLimits();

	if(m_ControllerMode==PYRAMID_SEARCH_RESULTS){
		// note we operate on the searchers result images only here
		// however, these are searcher::size() images only, we may need to hide all others
		// because the clients draw routine may draw all...
		// we hide all here, calculateDrawingPositionPyramid() will make the results visible
		
		//if(m_bSearchResultsVisibleOnly){
			hideAllSearchImages();
		//}

		//std::vector<TIMGPTR>::iterator it=m_pSearcherResults->begin();
		//std::vector<TIMGPTR>::iterator itEnd=m_pSearcherResults->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=m_pSearcherResults->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=m_pSearcherResults->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=m_pSearcherResults->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=m_pSearcherResults->end();
#endif

		int n=0;
		for(;it<itEnd;++it){
			calculateDrawingPositionPyramid((*it).operator->(), n++ /*img.id */);	
		}
	}
	else /*if((m_ControllerMode==PYRAMID_DYNAMIC_SEARCH_RESULTS)*/{

#ifdef __GNUC__
		typename std::multimap<float,refPtr<TIMG> >::iterator it=m_pDynamicSearcherResults->begin();
		typename std::multimap<float,refPtr<TIMG> >::iterator itEnd=m_pDynamicSearcherResults->end();
#else
		std::multimap<float,refPtr<TIMG> >::iterator it=m_pDynamicSearcherResults->begin();
		std::multimap<float,refPtr<TIMG> >::iterator itEnd=m_pDynamicSearcherResults->end();
#endif

		int n=0;
		while(it!=itEnd){
			calculateDrawingPositionPyramid(&(*((*it).second)), n++ /*img.id */);
			++it;
		}
	}

	pMutex->unlock();

	// tell the client to redraw...
	if(bCallBack){
		assert(m_pPaintRequestCallBack);
		m_pPaintRequestCallBack(m_pPaintRequestUserData);
	}

	// these are only !NULL, if calculatePyramidLayout() was called
	if(m_pThumbBoxSizesX){
		delete[] m_pThumbBoxSizesX;
		m_pThumbBoxSizesX=NULL;
	}
	if(m_pThumbBoxSizesY){
		delete[] m_pThumbBoxSizesY;
		m_pThumbBoxSizesY=NULL;
	}
	if(m_pImgsPerRow){
		delete[] m_pImgsPerRow;
		m_pImgsPerRow=NULL;
	}
}

template <typename TIMG> void guiControllerImpl<TIMG>::calculateDrawingPositionPyramid(controllerImage* pImg, int imgNumber) 
{
	int row=0;
	int col=0;
	int numberPics = 0;
	double thumbBoxSizeX = 0;
	double thumbBoxSizeY = 0;
	const double borderFactor = 1.-m_ThumbSpacing; 

	double yStart = m_MinPosY;
	
	for (int r = 0; r < m_NumRows; r++){
		if (imgNumber < numberPics + m_pImgsPerRow[r]) {
			row = r;
			col = (imgNumber - numberPics) % m_pImgsPerRow[row];
			thumbBoxSizeX = m_pThumbBoxSizesX[row];
			thumbBoxSizeY = m_pThumbBoxSizesY[row];
			break;
		}
		yStart += m_pThumbBoxSizesY[r]*m_dZoomFactor;
		numberPics += m_pImgsPerRow[r];	
	}

	double thumbSizeX = thumbBoxSizeX*m_dZoomFactor*borderFactor;
	double thumbSizeY = thumbBoxSizeY*m_dZoomFactor*borderFactor;

	// skalierung, keep aspect ratio
	double scale = std::min(thumbSizeX/pImg->width(), thumbSizeY/pImg->height());  
	// Thumbnailgroesse
	int xLen = std::max(1,(int)(scale*pImg->width()));
	int yLen = std::max(1,(int)(scale*pImg->height()));

	// Thumbnailposition
	int xs = (int)(m_MinPosX + (col+0.5)*thumbBoxSizeX*m_dZoomFactor - xLen*0.5 + 0.5); 
	int ys = (int)(yStart  +      0.5*(thumbBoxSizeY*m_dZoomFactor - yLen)    + 0.5);
	int xe = xs+xLen;
	int ye = ys+yLen;

	if (!(xs > (int)m_CanvasXSize || xe <= 0 || ys > (int)m_CanvasYSize || ye <= 0) ){
		// make visible
		pImg->m_uFlags|=isortisearch::controllerImage::m_VisibleFlag;
		// set drawing positions
		pImg->m_UpperLeftX  = xs;
		pImg->m_LowerRightX = xe;
		pImg->m_UpperLeftY  = ys;
		pImg->m_LowerRightY = ye;
	}
	else
		pImg->m_uFlags&=isortisearch::controllerImage::m_VisibleMask;
}


template <typename TIMG> void guiControllerImpl<TIMG>::calculatePyramidLayout(unsigned nThumbs) 
{
	// Groesse des obersten thumbnail-Bereichs
	int thumbBoxSize = std::min(m_CanvasXSize,m_CanvasYSize); 	// erste Annahne

	int remainingHeight = 0; 

	bool doesFit = false;
	
	while (!doesFit) {
		m_NumRows = 1;
		int size = --thumbBoxSize;
		remainingHeight = m_CanvasYSize - thumbBoxSize; 
		unsigned numPics = m_CanvasXSize / size;
		while (remainingHeight >= 0 && !doesFit) {
			m_NumRows++;
			size = (int) std::max((int)(size*m_dPyramidFactor), 1);
			numPics += m_CanvasXSize / size;
			remainingHeight -= size;
			if (remainingHeight >= 0 && numPics >= nThumbs)
				doesFit = true;
		}
	}

	try{
		m_pThumbBoxSizesX = new double[m_NumRows];
		m_pThumbBoxSizesY = new double[m_NumRows];
		m_pImgsPerRow = new int[m_NumRows];
	}
	catch(std::bad_alloc){
		if(m_pThumbBoxSizesX){
			delete[] m_pThumbBoxSizesX;
			m_pThumbBoxSizesX=NULL;
		}
		if(m_pThumbBoxSizesY){
			delete[] m_pThumbBoxSizesY;
			m_pThumbBoxSizesY=NULL;
		}
		if(m_pImgsPerRow){
			delete[] m_pImgsPerRow;
			m_pImgsPerRow=NULL;
		}
		throw;
	}

	m_pThumbBoxSizesY[0] = thumbBoxSize;
	m_pImgsPerRow[0] = (int) (m_CanvasXSize / thumbBoxSize);
	m_pThumbBoxSizesX[0] = (double) m_CanvasXSize / m_pImgsPerRow[0];
	for (int r = 1; r < m_NumRows; r++) {
		m_pThumbBoxSizesY[r] = (int) std::max((int)(m_dPyramidFactor*m_pThumbBoxSizesY[r-1]),1);	
		m_pImgsPerRow[r] = (int) (m_CanvasXSize / m_pThumbBoxSizesY[r]);
		m_pThumbBoxSizesX[r] = (double) m_CanvasXSize / m_pImgsPerRow[r];
	}

	// verbleibende Hoehe verteilen
	while (remainingHeight >= m_NumRows) {
		for (int r = 0; r < m_NumRows; r++) 
			m_pThumbBoxSizesY[r]++;
		remainingHeight -= m_NumRows;
	}
}

template <typename TIMG> int guiControllerImpl<TIMG>::getIdealPyramidResultNumber(int desiredResultNumber) 
{
	int thumbBoxSize = std::min(m_CanvasXSize,m_CanvasYSize); 	// erste Annahne

	int nRows = 0;

	int remainingHeight = 0; 
	int numPics=0;

	bool doesFit = false;
	while (!doesFit) {
		nRows = 1;
		int size = --thumbBoxSize;
		remainingHeight = m_CanvasYSize - thumbBoxSize; 
		numPics = m_CanvasXSize / size;
		while (remainingHeight >= 0 && !doesFit) {
			nRows++;
			size = (int)(size*m_dPyramidFactor);
			size = std::max(size, 1);
			numPics += m_CanvasXSize / size;
			remainingHeight -= size;
			if (remainingHeight >= 0) {
				if (numPics >= desiredResultNumber)
					doesFit = true;
			}
		}
	}
	return numPics;
}



template <typename TIMG> void guiControllerImpl<TIMG>::updateImagePositionsList(bool bCallBack)
{
	isortisearch::mutex* pMutex;
	if(m_ControllerMode==SORTER_INPUT){
		//if(!m_pSorter||!m_pSorter->m_pSortImages||m_pSorter->m_pSortImages->empty()){
		if(!m_pSorterImages||!(*m_pSorterImages)||!(*m_pSorterImages)->size()){
			// even redraw if we have no images, clear() may has been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
		assert(m_pSorterMutex);
		assert(*m_pSorterMutex);
		pMutex=(*m_pSorterMutex);
	}
	else if(m_ControllerMode==LIST_SEARCH_RESULTS){
		// if(!m_pSearcher||m_pSearcher->m_ResultImages.empty()){
		if(!m_pSearcherResults||!m_pSearcherResults->size()){
			// even redraw if we have no images, clear() may have been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
		assert(m_pSearcherMutex);
		assert(*m_pSearcherMutex);
		pMutex=*m_pSearcherMutex;
	}
	else if(m_ControllerMode==SEARCHER_INPUT){
		// if(!m_pSearcher||m_pSearcher->m_ResultImages.empty()){
		if(!m_pSearcherImages||!(*m_pSearcherImages)||!((*m_pSearcherImages)->size())){
			// even redraw if we have no images, clear() may have been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
		assert(m_pSearcherMutex);
		assert(*m_pSearcherMutex);
		pMutex=*m_pSearcherMutex;
	}
	else if(m_ControllerMode==LIST_DYNAMIC_SEARCH_RESULTS){
		if(!m_pDynamicSearcherResults||!(m_pDynamicSearcherResults->size())){
			// even redraw if we have no images, clear() may have been called...
			// TODOV5: check if there still is clear()...
			if(bCallBack){
				assert(m_pPaintRequestCallBack);
				m_pPaintRequestCallBack(m_pPaintRequestUserData);
			}
			return;
		}
		assert(m_pDynamicSearcherMutex);
		assert(*m_pDynamicSearcherMutex);
		pMutex=*m_pDynamicSearcherMutex;
	}
	else{
		assert(false);
		return;
	}
	pMutex->lock();

	//static double zoomFactorLast=1.;
	static double xm,ym;
	const double borderFactor = 1.-m_ThumbSpacing; 

	// reset everthing if this mode is called for the first time
	if(m_bFirstTimeColor) {
		xm = ym = 0; 
		//zoomFactorLast = m_dZoomFactor;
		m_bFirstTimeColor = false;
	}
	else if(m_bResizedAndResorted){
		xm=ym=0;
		m_bResized=m_bResizedAndResorted=false;
	}
	
	if(m_ControllerMode==SORTER_INPUT)
		calculateListLayout((*m_pSorterImages)->size());
	else if(m_ControllerMode==LIST_SEARCH_RESULTS)
		calculateListLayout(m_pSearcherResults->size());
	else if(m_ControllerMode==SEARCHER_INPUT)
		calculateListLayout((*m_pSearcherImages)->size());
	else if(m_ControllerMode==LIST_DYNAMIC_SEARCH_RESULTS)
		calculateListLayout(m_pDynamicSearcherResults->size());
	else
		return;

	calculateDrawingLimits();

	if(m_ControllerMode==SORTER_INPUT){
		//std::vector<TIMG*>::iterator it=(*m_pSorterImages)->begin();
		//std::vector<TIMG*>::iterator itEnd=(*m_pSorterImages)->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSorterImages)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSorterImages)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(*m_pSorterImages)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSorterImages)->end();
#endif

		int n=0;
		for(;it<itEnd;++it){
			controllerImage& img=**it;
			calculateDrawingPositionList(img, n++ /*img.id */);	
		}
	}
	else if(m_ControllerMode==LIST_SEARCH_RESULTS){
		// note we operate on the searchers result images only here
		// however, these are searcher::size() images only, we may need to hide all others
		// because the clients draw routine may draw all...
		// we hide all here, calculateDrawingPositionList() will make the results visible
		
		//if(m_bSearchResultsVisibleOnly){
			hideAllSearchImages();
		//}

		//std::vector<TIMGPTR>::iterator it=m_pSearcherResults->begin();
		//std::vector<TIMGPTR>::iterator itEnd=m_pSearcherResults->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(m_pSearcherResults)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(m_pSearcherResults)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(m_pSearcherResults)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(m_pSearcherResults)->end();
#endif
		int n=0;
		for(;it<itEnd;++it){
			controllerImage& img=*(*it);
			calculateDrawingPositionList(img, n++ /*img.id */);	
		}
	}
	else if(m_ControllerMode==SEARCHER_INPUT){
		//std::vector<TIMG*>::iterator it=(*m_pSearcherImages)->begin();
		//std::vector<TIMG*>::iterator itEnd=(*m_pSearcherImages)->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSearcherImages)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSearcherImages)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(*m_pSearcherImages)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSearcherImages)->end();
#endif

		int n=0;
		for(;it<itEnd;++it){
			controllerImage& img=**it;
			calculateDrawingPositionList(img, n++ /*img.id */);	
		}
	}
	else if(m_ControllerMode==LIST_DYNAMIC_SEARCH_RESULTS){
		// note we operate on the searchers result images only here
		// however, these are searcher::size() images only, we may need to hide all others
		// because the clients draw routine may draw all...
		// we hide all here, calculateDrawingPositionList() will make the results visible
		
		//if(m_bSearchResultsVisibleOnly){
			hideAllSearchImages();
		//}

#ifdef __GNUC__
		typename std::multimap<float,refPtr<TIMG> >::iterator it=m_pDynamicSearcherResults->begin();
		typename std::multimap<float,refPtr<TIMG> >::iterator itEnd=m_pDynamicSearcherResults->end();
#else
		std::multimap<float,refPtr<TIMG>>::iterator it=m_pDynamicSearcherResults->begin();
		std::multimap<float,refPtr<TIMG>>::iterator itEnd=m_pDynamicSearcherResults->end();
#endif
		int n=0;
		while(it!=itEnd){
			controllerImage& img=*((*it).second);
			calculateDrawingPositionList(img, n++ /*img.id */);	
			++it;
		}
	}
	pMutex->unlock();

	// tell the client to redraw...
	if(bCallBack){
		assert(m_pPaintRequestCallBack);
		m_pPaintRequestCallBack(m_pPaintRequestUserData);
	}
}

template <typename TIMG> void guiControllerImpl<TIMG>::calculateDrawingPositionList(controllerImage& img, int pos) 
{
	const double borderFactor = 1.-m_ThumbSpacing; 
	
	int w = img.width();
	int h = img.height();

	assert(w);
	assert(h);

	// skalierung, keep aspect ratio
	double scale = borderFactor*std::min(m_ScaledThumbBoxSizeX/w,m_ScaledThumbBoxSizeY/h);  

	int xLen = (int)(scale*w);
	int yLen = (int)(scale*h);

	int xs = (int)(m_MinPosX + (pos%m_MapPlacesListX)*m_ScaledThumbBoxSizeX + (m_ScaledThumbBoxSizeX - xLen + 1)/2); // xStart mit Rand
	int ys = (int)(m_MinPosY + (pos/m_MapPlacesListX)*m_ScaledThumbBoxSizeY + (m_ScaledThumbBoxSizeY - yLen + 1)/2);
	int xe = xs + xLen;
	int ye = ys + yLen;

	if (!(xs > (int)m_CanvasXSize || xe <= 0 || ys > (int)m_CanvasYSize || ye <= 0) ){
		// make visible
		img.m_uFlags|=isortisearch::controllerImage::m_VisibleFlag;
		// set drawing positions
		img.m_UpperLeftX  = xs;
		img.m_LowerRightX = xe;
		img.m_UpperLeftY  = ys;
		img.m_LowerRightY = ye;
	}
	else
		img.m_uFlags &= ~isortisearch::controllerImage::m_VisibleFlag;
}


template <typename TIMG> void guiControllerImpl<TIMG>::calculateDrawingLimits() {
	static double xm=0,ym=0;
	double scaledwCanvas = m_CanvasXSize*m_dZoomFactor;
	double scaledhCanvas = m_CanvasYSize*m_dZoomFactor;

	//static double zoomFactorLast=1.;

	//double xDelta= -m_MouseXPos * (m_dZoomFactor/zoomFactorLast-1);
	//double yDelta= -m_MouseYPos * (m_dZoomFactor/zoomFactorLast-1);
	//zoomFactorLast = m_dZoomFactor;
	double xDelta= -m_MouseXPos * (m_dZoomFactor/m_dLastZoomFactor-1);
	double yDelta= -m_MouseYPos * (m_dZoomFactor/m_dLastZoomFactor-1);
	m_dLastZoomFactor = m_dZoomFactor;

	double xmLast = xm;
	double ymLast = ym;

	xm -= (m_MouseMoveX+xDelta)/scaledwCanvas;
	ym -= (m_MouseMoveY+yDelta)/scaledhCanvas;

	m_MinPosX = (int)(-xm*scaledwCanvas);
	m_MaxPosX = (int) (m_MinPosX + scaledwCanvas);

	// disallow to move out of the map by dragging
	if (m_MinPosX > 0 || m_MaxPosX < m_CanvasXSize) {
		xm = xmLast;
		m_MinPosX = (int)(-xm*scaledwCanvas);
		m_MaxPosX = (int)(m_MinPosX + scaledwCanvas);
	}
	// when zooming out (centered at the mouseposition) it might be necessary to shift the map back to the canvas
	if (m_MaxPosX < m_CanvasXSize) {
		int xMoveCorrection=m_CanvasXSize - m_MaxPosX;
		m_MinPosX += xMoveCorrection;
		xm -= xMoveCorrection/scaledwCanvas;
	}
	else if (m_MinPosX > 0 ) {
		xm += m_MinPosX/scaledwCanvas;
		m_MinPosX = 0;
	}

	//same for y
	m_MinPosY = (int)(-ym*scaledhCanvas);
	m_MaxPosY = (int)(m_MinPosY + scaledhCanvas);

	if (m_MinPosY > 0 || m_MaxPosY < m_CanvasYSize) { 
		ym = ymLast;
		m_MinPosY = (int)(-ym*scaledhCanvas);
		m_MaxPosY = (int)(m_MinPosY + scaledhCanvas);
	}
	if (m_MaxPosY < m_CanvasYSize) {
		int yMoveCorrection=m_CanvasYSize - m_MaxPosY;
		m_MinPosY += yMoveCorrection;
		ym -= yMoveCorrection/scaledhCanvas;
	}
	else if (m_MinPosY > 0 ) {
		ym += m_MinPosY/scaledhCanvas;
		m_MinPosY = 0;
	}
}



template <typename TIMG> void guiControllerImpl<TIMG>::calculateListLayout(unsigned nThumbs) 
{
	// Groesse eines thumbnail-Bereichs - erste Annahme
	unsigned thumbSize = (int) sqrt((double)m_CanvasXSize*m_CanvasYSize/nThumbs);

	while (thumbSize>0 && (m_CanvasXSize/thumbSize) * (m_CanvasYSize/thumbSize) < nThumbs)
		--thumbSize;			

	m_MapPlacesListX = m_CanvasXSize / thumbSize;
	m_MapPlacesListY = m_CanvasYSize / thumbSize;

	double thumbBoxSizeX = (double)m_CanvasXSize / m_MapPlacesListX;
	double thumbBoxSizeY = (double)m_CanvasYSize / m_MapPlacesListY;

	// avoid empty lines at the bottom
	while (m_MapPlacesListX*(m_MapPlacesListY-1) >= nThumbs) {
		m_MapPlacesListY--;
	}
	thumbBoxSizeY = (double)m_CanvasYSize / m_MapPlacesListY;

	m_ScaledThumbBoxSizeX = thumbBoxSizeX*m_dZoomFactor;
	m_ScaledThumbBoxSizeY = thumbBoxSizeY*m_dZoomFactor;
}


template <typename TIMG> void guiControllerImpl<TIMG>::reset(bool bUpdatePositions)
{
	resetZoom(false);
	m_MouseMoveX=m_MouseMoveY=0;
	m_bFirstTimeColor=true;
	if(bUpdatePositions)
		updateImagePositions(true);
}

template <typename TIMG> void guiControllerImpl<TIMG>::resetZoom(bool bUpdatePositions)
{
	m_dZoomFactor=m_dLastZoomFactor=1.0;

	if(bUpdatePositions)
		updateImagePositions(true);
}


template <typename TIMG> refPtr<TIMG> guiControllerImpl<TIMG>::getImageAt(__int32 x,__int32 y)
{
	x=__max(0,x);
	y=__max(0,y);

	if(m_ControllerMode==SORTED||m_ControllerMode==SORTER_INPUT){
		assert(m_pSorterImages);
		assert(*m_pSorterImages);

		if(!m_pSorterImages||!*m_pSorterImages)
			return m_RefPtrNull;

#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSorterImages)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSorterImages)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(*m_pSorterImages)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSorterImages)->end();
#endif


		for(;it<itEnd;++it){
			const controllerImage& img=**it;

			// not this is essential, because invisible images
			// are 'hanging around' at the canvas edges (but are not drawn)
			// therefore, w/o this test we sometimes select invisible images in zoom modes... 
			if(img.visible())
				if(x>=img.upperLeftX()&&
				   x<=img.lowerRightX()&&
				   y>=img.upperLeftY()&&
				   y<=img.lowerRightY())
				   // TODOV5: is this allowd with stl vector<> ???
				   return *it;

		}
	}
	else if(m_ControllerMode==SEARCHER_INPUT){
		assert(m_pSearcherImages);
		assert(*m_pSearcherImages);

		if(!m_pSearcherImages||!*m_pSearcherImages)
			return m_RefPtrNull;

		//std::vector<TIMG*>::iterator it=(*m_pSearcherImages)->begin();
		//std::vector<TIMG*>::iterator itEnd=(*m_pSearcherImages)->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSearcherImages)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSearcherImages)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(*m_pSearcherImages)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSearcherImages)->end();
#endif


		for(;it<itEnd;++it){
			const controllerImage& img=**it;

			// not this is essential, because invisible images
			// are 'hanging around' at the canvas edges (but are not drawn)
			// therefore, w/o this test we sometimes select invisible images in zoom modes... 
			if(img.visible())
				if(x>=img.upperLeftX()&&
				   x<=img.lowerRightX()&&
				   y>=img.upperLeftY()&&
				   y<=img.lowerRightY())
				   // TODOV5: is this allowd with stl vector<> ???
				   return *it;
		}
	}
	else if(m_ControllerMode==LIST_SEARCH_RESULTS||m_ControllerMode==PYRAMID_SEARCH_RESULTS){
		assert(m_pSearcherResults);

		if(!m_pSearcherResults)
			return m_RefPtrNull;

		//std::vector<TIMGPTR>::iterator it=(m_pSearcherResults)->begin();
		//std::vector<TIMGPTR>::iterator itEnd=(m_pSearcherResults)->end();
#ifdef __GNUC__
		typename std::vector<refPtr<TIMG> >::iterator it=(m_pSearcherResults)->begin();
		typename std::vector<refPtr<TIMG> >::iterator itEnd=(m_pSearcherResults)->end();
#else
		std::vector<refPtr<TIMG>>::iterator it=(m_pSearcherResults)->begin();
		std::vector<refPtr<TIMG>>::iterator itEnd=(m_pSearcherResults)->end();
#endif

		for(;it<itEnd;++it){
			const controllerImage& img=*(*it);

			// not this is essential, because invisible images
			// are 'hanging around' at the canvas edges (but are not drawn)
			// therefore, w/o this test we sometimes select invisible images in zoom modes... 
			if(img.visible())
				if(x>=img.upperLeftX()&&
				   x<=img.lowerRightX()&&
				   y>=img.upperLeftY()&&
				   y<=img.lowerRightY())
				   // TODOV5: is this allowd with stl vector<> ???
				   return *it;
		}

	}
	else if(m_ControllerMode==LIST_DYNAMIC_SEARCH_RESULTS||m_ControllerMode==PYRAMID_DYNAMIC_SEARCH_RESULTS){
		assert(m_pDynamicSearcherResults);

		if(!m_pDynamicSearcherResults)
			return m_RefPtrNull;

#ifdef __GNUC__
		typename std::multimap<float,refPtr<TIMG> >::iterator it=(m_pDynamicSearcherResults)->begin();
		typename std::multimap<float,refPtr<TIMG> >::iterator itEnd=(m_pDynamicSearcherResults)->end();
#else
		std::multimap<float,refPtr<TIMG>>::iterator it=(m_pDynamicSearcherResults)->begin();
		std::multimap<float,refPtr<TIMG>>::iterator itEnd=(m_pDynamicSearcherResults)->end();
#endif


		while(it!=itEnd){
			const controllerImage& img=*((*it).second);

			// not this is essential, because invisible images
			// are 'hanging around' at the canvas edges (but are not drawn)
			// therefore, w/o this test we sometimes select invisible images in zoom modes... 
			if(img.visible())
				if(x>=img.upperLeftX()&&
				   x<=img.lowerRightX()&&
				   y>=img.upperLeftY()&&
				   y<=img.lowerRightY())
				   // TODOV5: is this allowd with stl vector<> ???
				   return (*it).second;

			++it;
		}

	}
	return m_RefPtrNull;
}

template <typename TIMG> bool guiControllerImpl<TIMG>::registerPaintRequestCallback(pxCallback pCallBack,void *pUserData)
{
	if(!pCallBack)
		return false;

	m_pPaintRequestCallBack=pCallBack;
	m_pPaintRequestUserData=pUserData;

	return true;
}

template <typename TIMG> void guiControllerImpl<TIMG>::onMouseLeave()
{
	if(m_bMouseDown)
		onMouseUp(m_MouseXPos,m_MouseYPos,false,false);
}

template <typename TIMG> void guiControllerImpl<TIMG>::setClickSensitivity(unsigned __int32 num)
{
	m_uClickSensitivity=num;
}

template <typename TIMG> bool guiControllerImpl<TIMG>::setMode(controller_mode mode,bool bRepaint)
{
	switch(mode){
		case SORTED:
		case SORTER_INPUT:
			if(!m_bAttachedToSorter)
				return false;
			break;
		case LIST_SEARCH_RESULTS:
		case PYRAMID_SEARCH_RESULTS:
		case SEARCHER_INPUT:
			if(!m_bAttachedToSearcher)
				return false;
			break;
		case LIST_DYNAMIC_SEARCH_RESULTS:
		case PYRAMID_DYNAMIC_SEARCH_RESULTS:
			if(!m_bAttachedToDynamicSearcher)
				return false;
			break;
		default:
			return false;
			break;
	}
	m_ControllerMode=mode;

	updateImagePositions(bRepaint);

	return true;

}

template <typename TIMG> void guiControllerImpl<TIMG>::hideAllSearchImages()
{
	if(!m_pSearcherImages||!(*m_pSearcherImages))
		return;

	//std::vector<TIMG*>::iterator it=(*m_pSearcherImages)->begin();
	//std::vector<TIMG*>::iterator itEnd=(*m_pSearcherImages)->end();
#ifdef __GNUC__
	typename std::vector<refPtr<TIMG> >::iterator it=(*m_pSearcherImages)->begin();
	typename std::vector<refPtr<TIMG> >::iterator itEnd=(*m_pSearcherImages)->end();
#else
	std::vector<refPtr<TIMG>>::iterator it=(*m_pSearcherImages)->begin();
	std::vector<refPtr<TIMG>>::iterator itEnd=(*m_pSearcherImages)->end();
#endif

	while(it!=itEnd){
		(*it)->m_uFlags&=isortisearch::controllerImage::m_VisibleMask;
		++it;
	}

}
template class guiControllerImpl<isortisearch::controllerImage>;
}


#if defined(_LEAK_H)
	#define new new
#endif
