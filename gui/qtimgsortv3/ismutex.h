#pragma once

#include "pxmutex.h"
#include <QMutex>

/*!
the clients implementation of isortisearch::mutex

this simply wraps a QMutex
*/
class ISMutex : public isortisearch::mutex
{
public:
	ISMutex() {;}
	~ISMutex() {;}
	/*!
	The isortisearch::mutex::lock() implementation
	*/
	virtual void lock() {m_Mutex.lock();}
	/*!
	The isortisearch::mutex::unlock() implementation
	*/
	virtual void unlock(){m_Mutex.unlock();}
	/*!
	direct acces to the mutex
	*/
	QMutex& rmutex() {return m_Mutex;}
protected:
	/*!
	The mutex itself
	*/
	QMutex m_Mutex;
};

