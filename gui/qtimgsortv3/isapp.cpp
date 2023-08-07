#include "isapp.h"
#include <QMessageBox>

ISApplication::ISApplication(int &argc, char **argv) : QApplication(argc,argv)
{
	m_pMyMem=new char[1024*1024];
}

ISApplication::~ISApplication()
{
	if(m_pMyMem)
		delete[] m_pMyMem;
}

bool ISApplication::notify(QObject* receiver, QEvent* even)
{
    try {
        return QApplication::notify(receiver,even);
    } catch (std::bad_alloc &e) {
		if(m_pMyMem){
			delete[] m_pMyMem;
			m_pMyMem=NULL;
		}
		QMessageBox box(QMessageBox::Critical,tr("Critical error"),tr("ImageSorter V4.3 is low on memory and will be closed.\nThis may happen if too many images are loaded."));
		box.exec();
		// note settings are *not* stored. This is good, otherwise ImageSorter would try to read the folder with too many images again
		// on the next start...
		exit(-1);
    } catch (...) {
		if(m_pMyMem){
			delete[] m_pMyMem;
			m_pMyMem=NULL;
		}
		QMessageBox box(QMessageBox::Critical,tr("Critical error"),tr("ImageSorter V4.3 has an unknown error and will be closed."));
		box.exec();
		exit(-1);    
    }        

    // qFatal aborts, so this isn't really necessary
    // but you might continue if you use a different logging lib
    return false;
}
