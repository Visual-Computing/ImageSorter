#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QtGui>
#include <QMainWindow>

#include "imagecanvas.h"
// ANUJ***
#include "synchttp.h"
// ***ANUJ
//#include "preferences.h"

class QLabel;
class QAction;
class QString;
class QImage;
class QScrollBar;
class QScrollArea;
class QToolBar;
class QStatusBar;
class QMenuBar;
class QMenu;
class QFile;

class ImageCanvas;

class ImageViewer : public QMainWindow {
	
	Q_OBJECT
    public:
        ImageViewer(QWidget *parent = 0);
		~ImageViewer();

		void setBackgroundColor(QColor color);
		// ANUJ***
		void setImageFile(const QFileInfo& fio,QString url = NULL);
		// ***ANUJ

		void updateFitWindowStatus();
		void updateStatusBarZoom();
		void updateStatusBarImageInfo();


	protected:
		//virtual void closeEvent(QCloseEvent *event);		
		virtual void showEvent(QShowEvent *event);
		virtual void resizeEvent(QResizeEvent *event);
		virtual void keyPressEvent(QKeyEvent *event);


    private slots:
        void originalSize();
		void fitToWindow();
		void rotateCW();
		void rotateCCW();
        void zoomIn();
        void zoomOut();


    private:

		// function definitions
		void init();
		void createActions();
		void createMenuBar();
		void createToolBar();
		void createStatusBar();
		void createMainArea();

		void setWindowSize();
        void scaleImage(double factor);
        void adjustScrollBar(QScrollBar *scrollBar, double factor);
		void enableActions(bool enable);
		QString formatImageSize(double size);				

		//Preferences *prefs;

		// variable definitions
		QFile *imgFile;
		QImage *image;

		// gui elements
		QMenuBar *menuBar;
		QToolBar *toolBar;
		QStatusBar *statusBar;

		QMenu *menuFile;
		QMenu *menuView;	

		QLabel *statusBarPathLabel;
		QLabel *statusBarZoomLabel;
		QLabel *statusBarImgSizeLabel;
		QLabel *statusBarFileSizeLabel;

		ImageCanvas *imageCanvas;
        QScrollArea *scrollArea;

		QAction *closeAction;
        QAction *zoomInAction;
        QAction *zoomOutAction;
        QAction *zoomOrigSizeAction;
        QAction *zoomFitWindowAction;
		QAction *rotateCWAction;
		QAction *rotateCCWAction;

		bool m_bLoadFailure;
		// ANUJ***
		bool m_fromNet;
		// ***ANUJ
		QImage m_LoadFailureImage;

};

#endif
