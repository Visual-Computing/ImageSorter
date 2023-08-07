#include "pxguicontroller.h"
#include "pxguicontrollerimpl.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
namespace isortisearch{

template <typename TIMG> guiController<TIMG>::guiController() : 
	m_pImpl(NULL)
{
	m_pImpl=new guiControllerImpl<TIMG>;
}

template <typename TIMG> guiController<TIMG>::~guiController()
{
	if(m_pImpl)
		delete m_pImpl;
}

template <typename TIMG> unsigned __int32 guiController<TIMG>::majorVersion() 
{
	return guiControllerImpl<TIMG>::majorVersion();
}

template <typename TIMG> unsigned __int32 guiController<TIMG>::minorVersion() 
{
	return guiControllerImpl<TIMG>::minorVersion();
}


template <typename TIMG> bool guiController<TIMG>::setCanvasSize(__int32 x,__int32 y,bool bRedraw) 
{
	assert(m_pImpl);
	return m_pImpl->setCanvasSize(x,y,bRedraw);
}

template <typename TIMG> void guiController<TIMG>::setImageSpacing(double val) 
{
	assert(m_pImpl);
	m_pImpl->setImageSpacing(val);
}

template <typename TIMG> void guiController<TIMG>::onMouseDown(__int32 xPos,__int32 yPos)
{
	assert(m_pImpl);
	m_pImpl->onMouseDown(xPos,yPos);
}

template <typename TIMG> void guiController<TIMG>::onMouseMove(__int32 xPos,__int32 yPos)
{
	assert(m_pImpl);
	m_pImpl->onMouseMove(xPos,yPos);
}

template <typename TIMG> refPtr<TIMG> guiController<TIMG>::onMouseUp(__int32 xPos,__int32 yPos,bool bRedraw,bool bReturnImage)
{
	assert(m_pImpl);
	return m_pImpl->onMouseUp(xPos,yPos,bRedraw,bReturnImage);
}

template <typename TIMG> void guiController<TIMG>::setClickSensitivity(unsigned __int32 num)
{
	assert(m_pImpl);
	return m_pImpl->setClickSensitivity(num);
}

template <typename TIMG> unsigned __int32 guiController<TIMG>::clickSensitivity() const
{
	assert(m_pImpl);
	return m_pImpl->clickSensitivity();
}

template <typename TIMG> void guiController<TIMG>::onZoomIn()
{
	assert(m_pImpl);
	m_pImpl->onZoomIn();
}
template <typename TIMG> void guiController<TIMG>::onZoomOut()
{
	assert(m_pImpl);
	m_pImpl->onZoomOut();
}
template <typename TIMG> double guiController<TIMG>::zoomStep() const
{
	assert(m_pImpl);
	return m_pImpl->zoomStep();
}
template <typename TIMG> bool guiController<TIMG>::setZoomStep(double step)
{
	assert(m_pImpl);
	return m_pImpl->setZoomStep(step);
}

template <typename TIMG> double guiController<TIMG>::maxZoomFactor() const
{
	assert(m_pImpl);
	return m_pImpl->maxZoomFactor();
}
template <typename TIMG> bool guiController<TIMG>::setMaxZoomFactor(double f)
{
	assert(m_pImpl);
	return m_pImpl->setMaxZoomFactor(f);
}

template <typename TIMG> void guiController<TIMG>::onZoom(double factor)
{
	assert(m_pImpl);
	m_pImpl->onZoom(factor);
}

template <typename TIMG> void guiController<TIMG>::reset(bool bUpdatePositions)
{
	assert(m_pImpl);
	m_pImpl->reset(bUpdatePositions);
}

template <typename TIMG> void guiController<TIMG>::resetZoom(bool bUpdatePositions)
{
	assert(m_pImpl);
	m_pImpl->resetZoom(bUpdatePositions);
}

template <typename TIMG> bool guiController<TIMG>::registerPaintRequestCallback(pxCallback pCallBack,void *pUserData)
{
	assert(m_pImpl);
	return m_pImpl->registerPaintRequestCallback(pCallBack,pUserData);
}

template <typename TIMG> void guiController<TIMG>::onMouseLeave()
{
	assert(m_pImpl);
	m_pImpl->onMouseLeave();
}


template <typename TIMG> refPtr<TIMG> guiController<TIMG>::getImageAt(__int32 x,__int32 y)
{
	assert(m_pImpl);
	return m_pImpl->getImageAt(x,y);
}

template <typename TIMG> bool guiController<TIMG>::setMode(controller_mode mode,bool bRepaint) 
{
	assert(m_pImpl);
	return m_pImpl->setMode(mode,bRepaint);
}

template <typename TIMG> controller_mode guiController<TIMG>::mode() const
{
	assert(m_pImpl);
	return m_pImpl->mode();
}
template <typename TIMG> void guiController<TIMG>::calculate(bool bRepaint)
{
	assert(m_pImpl);
	return m_pImpl->updateImagePositions(bRepaint);
}
template class guiController<isortisearch::controllerImage>;

}