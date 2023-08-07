#pragma once
#include <QThread>
#include <QString>
#include <QQueue>
#include <QHttp>

class flickrImgSearchResult;
class QMutex;
#pragma message("build in super class for both yahoo and flickr search...")
class flickrImgSearchQueryThread : public QThread
{
	Q_OBJECT
public:
	flickrImgSearchQueryThread();
	~flickrImgSearchQueryThread();
	void setStart(unsigned start) {m_uStart=start;}
	void setQuery(const QString& query) {m_Query=query;}
	void setResults(QQueue<flickrImgSearchResult>* pResults) {m_pResults=pResults;}
	void setMutex(QMutex* pMutex) {m_pMutex=pMutex;}
	void setNumQueries(unsigned num) {m_uNumQueries=num;}
	void setTimeOut(unsigned msecs) {m_uTimeOut=msecs;}
	QHttp::Error httpError() const {return m_HttpError;}
	bool hasHttpError() const {return m_HttpError!=QHttp::NoError;}
	QString httpErrorString() const {return m_HttpErrorString;}
	void setId(unsigned id) {m_Id=id;}
protected slots:
	void isDone(bool bError);
	void timedOut(void);
protected:
	virtual void run();
	unsigned m_uStart;
	QString m_Query;
	QQueue<flickrImgSearchResult>* m_pResults;
	QMutex* m_pMutex;
	unsigned m_uNumQueries;
	bool m_bError;
	bool m_bCanRun;
	unsigned m_Id;
	unsigned m_uTimeOut;
	bool m_bTimedOut;
	QHttp::Error m_HttpError;
	QString m_HttpErrorString;
};