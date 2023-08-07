#pragma once
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QQueue>
#include <QString>
// TODOV4: don't include a widget here
#include "issearchwidget.h"

class yahooImgSearchV1QueryThread;
class imageSortWorker;
class yahooImgSearchV1Result;
class yahooImgSearchV1QueryDispatcher : public QThread
{
	Q_OBJECT
public:
	yahooImgSearchV1QueryDispatcher();
	~yahooImgSearchV1QueryDispatcher();
	void setWorker(imageSortWorker* p) {m_pWorker=p;}
	void setNumThreads(unsigned num) {m_uNumThreads=num;}
	void setNumQueries(unsigned num) {m_uNumQueries=num;}
	void setNumQueriesPerThread(unsigned num) {m_uNumQueriesPerThread=num;}
	void setResults(QQueue<yahooImgSearchV1Result>* pResults) {m_pResults=pResults;}
	void setQuery(const QString& query) {m_Query=query;}
	void setSearchOptions(const searchOptions& options) {m_searchOptions=options;}
	bool hasHttpError() const {return m_bHttpError;}
	const QString& httpErrorString() const {return m_HttpErrorString;}
signals:
	void queried();
protected slots:
	void childFinished();
protected:
	virtual void run();
	QVector<yahooImgSearchV1QueryThread*> m_QueryThreads;
	QQueue<yahooImgSearchV1Result>* m_pResults;
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
	searchOptions m_searchOptions;
};