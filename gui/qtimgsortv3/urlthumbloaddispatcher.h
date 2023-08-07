#pragma once
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QString>
#include <QMultiMap>
#include <QSet>
class urlThumbLoader;
class imageSortWorker;
class thumbnail;
class urlThumbLoadDispatcher : public QThread
{
	Q_OBJECT
public:
	urlThumbLoadDispatcher();
	~urlThumbLoadDispatcher();
	void setNumThreads(unsigned num) {m_uNumThreads=num;}
	void setRepaintInterval(unsigned num) {m_uNumRepaints=num;}
	void enableSimilaritySearch(bool b) {m_bSearchSimilar=b;}
	void setWorker(imageSortWorker* pWorker);
	bool hasHttpError() const {return !m_HttpErrorStrings.empty();}
	const QMultiMap<QString,QString>& httpErrorStrings() const {return m_HttpErrorStrings;}
signals:
	void loaded();
	void updateStatusBar();
	void updateStatusBar2(unsigned dirs,unsigned files);
protected slots:
	void childFinished();
protected:
	virtual void run();
	QVector<urlThumbLoader*> m_Loaders;
	QVector<thumbnail*> m_Thumbs;
	imageSortWorker* m_pWorker;
	unsigned m_uNumThreads;
	unsigned m_uNumRepaints;
	QMutex m_Mutex;
	bool m_bAbort;
	QMultiMap<QString,QString> m_HttpErrorStrings;
	bool m_bSearchSimilar;
	QSet<unsigned> m_ExitIDs;
	unsigned m_uFilesLoaded;
};