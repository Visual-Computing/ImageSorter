#pragma once
#include <QThread>
#include <QString>
#include <QQueue>
#include <QHttp>
//TODOV4: don't include a widget here
#include "issearchwidget.h"

class yahooImgSearchV1Result;
class QMutex;

class yahooImgSearchV1QueryThread : public QThread
{
	Q_OBJECT
public:
	yahooImgSearchV1QueryThread();
	~yahooImgSearchV1QueryThread();
	// call this with a thumbnail containing valid yahoo image search urls
	// the timeOut is given in milliseconds (0 means no timer at all)
	void setStart(unsigned start) {m_uStart=start;}
	void setQuery(const QString& query) {m_Query=query;}

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Setzen der Suchoptionen für diesen Threads
	*/
	void setSearchOptions(const searchOptions& options) {m_searchOptions=options;}

	void setResults(QQueue<yahooImgSearchV1Result>* pResults) {m_pResults=pResults;}
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
	QQueue<yahooImgSearchV1Result>* m_pResults;
	QMutex* m_pMutex;
	unsigned m_uNumQueries;
	bool m_bError;
	bool m_bCanRun;
	unsigned m_Id;
	unsigned m_uTimeOut;
	bool m_bTimedOut;
	QHttp::Error m_HttpError;
	QString m_HttpErrorString;


	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Variable für die Suchoptionen
	*/
	searchOptions m_searchOptions;
};


