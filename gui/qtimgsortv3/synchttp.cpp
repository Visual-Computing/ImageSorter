#include "synchttp.h"

bool SyncHTTP::syncGet ( const QString & path, QIODevice * to )
{
			///connect the requestFinished signal to our finished slot
			connect(this,SIGNAL(requestFinished(int,bool)),SLOT(finished(int,bool)));
			/// start the request and store the requestID
			requestID = get(path, to );
			/// block until the request is finished
			loop.exec();
			/// return the request status
			return status;
}

bool SyncHTTP::syncPost ( const QString & path, QIODevice * data, QIODevice * to  )
{
			///connect the requestFinished signal to our finished slot
			connect(this,SIGNAL(requestFinished(int,bool)),SLOT(finished(int,bool)));
			/// start the request and store the requestID
			requestID = post(path, data , to );
			/// block until the request is finished
			loop.exec();
			/// return the request status
			return status;
}


bool SyncHTTP::syncPost ( const QString & path, const QByteArray& data, QIODevice * to  )
{
			/// create io device from QByteArray
			QBuffer buffer;
			buffer.setData(data);
			return syncPost(path,&buffer,to);
}


void SyncHTTP::finished(int idx, bool err)
{
			/// check to see if it's the request we made
			if(idx!=requestID)
				return;
			/// set status of the request
			status = !err;
			/// end the loop
			loop.exit();
}

