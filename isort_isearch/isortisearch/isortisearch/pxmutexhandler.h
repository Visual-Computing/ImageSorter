#if !defined(PXMUTEXHANDLER_H)
#define PXMUTEXHANDLER_H

#include "pxmutex.h"

namespace isortisearch{

class mutexHandler
{
public:
	mutexHandler();
	~mutexHandler();
	bool setMutex(mutex* pMutex);
	bool enableMutex();
	void disableMutex();
	void enableAutoMutex(bool bEnable);
	void lock();
	void unlock();
protected:
	static mutex m_DummyMutex;
	mutex* m_pMutex;
	mutex* m_pClientMutex;
	bool m_bAutoMutex;
private:
	mutexHandler(const mutexHandler& rOther){;}
	const mutexHandler& operator=(const mutexHandler& rOther){;}
};

}
#endif