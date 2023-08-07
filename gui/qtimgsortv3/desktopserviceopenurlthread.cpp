#include "desktopserviceopenurlthread.h"
#include <QDesktopServices>

desktopServiceOpenUrlThread::desktopServiceOpenUrlThread()
{
	connect(this,SLOT(hasFinished()),this,SIGNAL(finished()));
}

desktopServiceOpenUrlThread::desktopServiceOpenUrlThread(const QUrl& toOpen)
{
	connect(this,SIGNAL(finished()),this,SLOT(hasFinished()));
	setUrl(toOpen);
}

void desktopServiceOpenUrlThread::hasFinished()
{
	// destroy myself:
	delete this;
}

void desktopServiceOpenUrlThread::run()
{
	if(!m_UrlToOpen.isValid())
		return;

	QDesktopServices::openUrl(m_UrlToOpen);

}