#include "pxmutexhandler.h"
namespace isortisearch {

mutex mutexHandler::m_DummyMutex;

mutexHandler::mutexHandler() : m_pMutex(&m_DummyMutex),m_pClientMutex(&m_DummyMutex),m_bAutoMutex(true)
{
	;
}

mutexHandler::~mutexHandler()
{
	;
}

bool mutexHandler::setMutex(mutex* pMutex)
{
	if(!pMutex){
		m_pClientMutex=&m_DummyMutex;
		return false;
	}
	m_pClientMutex=pMutex;
	return true;
}
bool mutexHandler::enableMutex()
{
	m_pMutex=m_pClientMutex;
	return !(m_pMutex==&m_DummyMutex);
}
void mutexHandler::disableMutex()
{
	m_pMutex=&m_DummyMutex;
}

void mutexHandler::lock()
{
	m_pMutex->lock();
}

void mutexHandler::unlock()
{
	m_pMutex->unlock();
}

void mutexHandler::enableAutoMutex(bool bEnable)
{
	m_bAutoMutex=bEnable;
}

}
