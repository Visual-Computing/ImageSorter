#ifndef ETKSYNCHTTP_H
#define ETKSYNCHTTP_H
#include <QHttp>
#include <QEventLoop>
#include <QBuffer>

/**
 * Provide a synchronous api over QHttp
 * Uses a QEventLoop to block until the request is completed
 * @author Iulian M <eti@erata.net>
*/
class SyncHTTP: public QHttp
{
	Q_OBJECT
	public:
		/// structors
		SyncHTTP( QObject * parent = 0 )
		:QHttp(parent),requestID(-1),status(false){}

		// NOTE QT 4.3.x supports Https
		SyncHTTP( const QString & hostName, quint16 port = 80, QObject * parent = 0 )
		:QHttp(hostName,QHttp::ConnectionModeHttp,port,parent),requestID(-1),status(false){}

		virtual ~SyncHTTP(){}

		/// send GET request and wait until finished
		bool syncGet ( const QString & path, QIODevice * to = 0 );


		/// send POST request and wait until finished
		bool syncPost ( const QString & path, QIODevice * data, QIODevice * to = 0 );


		bool syncPost ( const QString & path, const QByteArray& data, QIODevice * to = 0 );

	protected slots:
		virtual void finished(int idx, bool err);

	private:
		/// id of current request
		int requestID;
		/// error status of current request
		bool status;
		/// event loop used to block until request finished
		QEventLoop loop;
};

#endif

