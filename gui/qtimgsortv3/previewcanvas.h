#ifndef PREVIEWCANVAS_H
#define PREVIEWCANVAS_H

#include <QtGui>
#include <assert.h>
#include "thumbnail.h"

typedef struct _imageInfo{
	
	QString m_PathOrURL;
	QString m_FileName;
	QString m_FileSize;
	QDateTime m_LastModified;
	
	unsigned m_XSize;
	unsigned m_YSize;
	bool m_bLoadFailure;
	bool m_bIsFromNet;
	bool m_bIsValid;

} imageInfo;

class PreviewCanvas : public QFrame {

	Q_OBJECT
	public:
		// constructor / destructor
		PreviewCanvas(QFrame *parent = 0, Qt::WFlags f = 0);
		~PreviewCanvas() {;}

		// public methods
		void setImageFileInfo(QFileInfo fileInfo);
		void setThumbnail(const thumbnail* pThumb);
		void clearCanvas();
		
		QString formatImageSize(qint64 size);

		void setBackgroundColor(const QColor& color); 

	//public slots:
	//	void enableHighResPreview(bool bEnable);

	signals:
		void showImageInfo(const imageInfo& iInfo);

	protected:
		virtual void paintEvent(QPaintEvent *event);
		QSize minimumSizeHint() const;


	private:
		QColor m_BGColor;

		void computeSize();

		QImage m_Img;

		// original img size
		int imgWidth;
		int imgHeight;

		// draw size
		int drawWidth;
		int drawHeight;

		//bool m_bHighRes;

		bool m_bLoadFailure;
		QImage m_LoadFailureImage;

};


#endif // PREVIEWCANVAS_H
