#pragma once
#include <QThread>
#include <QHttp>
#include <QUrl>
#include <QTimer>
class thumbnail;

class urlThumbLoader : public QThread
{
	Q_OBJECT
public:
	urlThumbLoader();
	~urlThumbLoader();
	// call this with a thumbnail containing valid thumnail urls (currently from yahoo or flickr)
	// the timeOut is given in milliseconds (0 means no timer at all)
	bool setRunArgs(thumbnail* pThumb,unsigned timeOut=5000);
	void setSize(unsigned size) {m_uSize=size;}
	QHttp::Error httpError() const {return m_HttpError;}
	bool hasHttpError() const {return m_HttpError!=QHttp::NoError;}
	bool hasImageLoadError() const {return m_bImageLoadError;}
	const QString& httpErrorString() const {return m_HttpErrorString;}
	void setId(unsigned id) {m_Id=id;}
protected slots:
	void isDone(bool bError);
	void timedOut(void);
protected:
	virtual void run();
	thumbnail* m_pThumb;
	unsigned m_uSize;
	QUrl m_Url;
	bool m_bError;
	bool m_bImageLoadError;
	bool m_bCanRun;
	unsigned m_Id;
	unsigned m_uTimeOut;
	bool m_bTimedOut;
	QHttp::Error m_HttpError;
	QString m_HttpErrorString;
};


