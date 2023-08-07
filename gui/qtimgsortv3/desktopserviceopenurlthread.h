#pragma once
#include <QThread>
#include <QUrl>
/*!
desktopServiceOpenUrlThread

this is a thread which calls QDesktopServive::openUrl() in run().
when run returns, the thread deletes itself

thus: never construct a desktopServiceOpenUrlThread on the heap, always get it be operator new
TODO: check if we can *prohibit* heap allocation of this class
IDEA: maybe an overloaded desktopServiceOpenUrlThread::operator new() which is public and protected ctors?
*/
class desktopServiceOpenUrlThread : public QThread
{
	Q_OBJECT
public:
	desktopServiceOpenUrlThread(const QUrl& toOpen);
	desktopServiceOpenUrlThread();
	~desktopServiceOpenUrlThread(){;}
	void setUrl(const QUrl& toOpen) {m_UrlToOpen=toOpen;}
protected slots:
	void hasFinished();
protected:
	virtual void run();
	QUrl m_UrlToOpen;

};