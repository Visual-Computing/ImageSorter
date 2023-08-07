#ifndef ISAPP_H
#define	ISAPP_H
#include <QApplication>

class ISApplication : public QApplication
{
public: 
	ISApplication(int &argc, char **argv);
	~ISApplication();
	virtual bool notify( QObject *receiver, QEvent *e);
protected:
	// allocate  some mem to free in case of bad allocations
	char* m_pMyMem;
};
#endif
