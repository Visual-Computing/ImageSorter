#pragma once
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QQueue>
#include <QString>
#pragma message("build in super class for yahoo and flickr query thread dispatcher...")
class flickrImgSearchQueryThread;
class imageSortWorker;
class flickrImgSearchResult;
class flickrImgSearchQueryDispatcher : public QThread
{
	Q_OBJECT
public:
	flickrImgSearchQueryDispatcher();
	~flickrImgSearchQueryDispatcher();
	void setWorker(imageSortWorker* p) {m_pWorker=p;}
	void setNumThreads(unsigned num) {m_uNumThreads=num;}
	void setNumQueries(unsigned num) {m_uNumQueries=num;}
	void setNumQueriesPerThread(unsigned num) {m_uNumQueriesPerThread=num;}
	void setResults(QQueue<flickrImgSearchResult>* pResults) {m_pResults=pResults;}
	void setQuery(const QString& query) {m_Query=query;}
	bool hasHttpError() const {return m_bHttpError;}
	const QString& httpErrorString() const {return m_HttpErrorString;}
signals:
	void queried();
protected slots:
	void childFinished();
protected:
	virtual void run();
	QVector<flickrImgSearchQueryThread*> m_QueryThreads;
	QQueue<flickrImgSearchResult>* m_pResults;
	unsigned m_uNumThreads;
	unsigned m_uNumQueries;
	unsigned m_uNumQueriesPerThread;
	QMutex m_Mutex;
	QString m_Query;
	unsigned m_uStart;
	imageSortWorker* m_pWorker;
	bool m_bAbort;
	bool m_bHttpError;
	QString m_HttpErrorString;
};