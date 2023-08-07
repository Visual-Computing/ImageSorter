/*
a qt gui for image sorting

author: sebastian richter
initial version: 0.1 3.2.2006

*/
#pragma once
#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QTreeView>
//#include <QDirModel>
// V4.2: we use a QFileSystemModel instaed a QDirModel now...
#include <QFileSystemModel>
#include <QtAlgorithms>
#include "internetsetdialog.h"
#include "aboutdialog.h"
#include "thumbnail.h"
#include "imagesortwidget.h"
#include "previewcanvas.h"
#include "istreeview.h"
#include "issearchwidget.h"
#include "isstatuswidget.h"
#include "isinfowidget.h"
#include "isextensionwidget.h"
class QAction;
class QMenu;

// this was just for debugging, see imageSortMainWindow::~imageSortMainWindow()

//class QCntMutex : public QMutex
//{
//public:
//	QCntMutex() : m_LockCnt(0) {;}
//	void lock(); 
//	void unlock();
//	unsigned m_LockCnt;
//};

class imageSortMainWindow : public QMainWindow
{
    Q_OBJECT

	public:
		imageSortMainWindow();
		~imageSortMainWindow();
		/*!
		delivers an empty menu to the caller
		the caller takes ownership

		this is to prohibit *any* menu

		return new QMenu works, too, and seems more correct,
		but we'll have a tiny 1*1 widgte then...
		*/
		virtual QMenu* createPopupMenu() {return NULL;}
	public slots:
		// void queryChanged(const QString& str);
		// void updateSearchButtons();
		/*!
		called when the widgets state has changed
		*/
		void sortWidgetStateChanged();
		/*!
		called when the widget sort mode changes
		*/
		void sortWidgetSortModeChanged();
		/*!
		called when the widget has deleted/moved/copied image files
		*/
		void refreshFileSystem();
		void refreshFileSystemPath(const QString& path);
		/*!
		opens image in preview
		*/
		void openInPreview(const QFileInfo&);
		/*!
		opens image in preview
		*/
		void openInPreview(const thumbnail*);
		/*!
		shows the wait cursor
		*/
		void showWaitCursor();
		/*!
		hides the wait cursor
		*/
		void hideWaitCursor();
		/*!
		displays image info in status bar
		*/
		
		
		//void showImageInfo(const imageInfo& info);
		
		/*!
		called when the user changes file extensions for raw files
		*/
		// void newRawFileExts();
		/*!
		scrolls the explorer pane to the dir passed
		*/
		void scrollExplorerTo(const QString& absPath);
		/*!
		select the dirs passed (w/o emitting a selection changed signal)
		*/
		void selectDirs(QStringList dirs);
		/*!
		clears all selected dirs
		*/
		void deSelectDirs();
		/*!
		shows number of loaded images
		*/
		// V4DEADCODE
		// void showProgress(unsigned int numLoaded);
	private slots:
	
		void setSearchOptions();
		void setSortOptions();
	    void about();
		void dockFloatStateChanged(bool b);

		void backgroundColorActionTriggered();
		void sortByActionTriggered();
#if defined(Q_WS_MAC)
		void showHiddenFiles(bool b);
#endif

		//void queryReturnPressed();

		// resets
		void resetVisualization(bool bRedraw=true);

		// selection changes slot (necessary for keyboard input)
		void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Methode für Suche mit Suchoptionen
		*/
		void search( searchOptions &);

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Methode für Vollbildmodus
		*/
		void showMainArea();


	protected:
		virtual void closeEvent(QCloseEvent *event);
		
		/*
		key handling
		
		just ESC to stop loading or sorting
		*/
		virtual void keyPressEvent(QKeyEvent *pe);

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Event-Methode zur Anpassung der Toolbarhöhe
		*/
		virtual void resizeEvent(QResizeEvent *re);
		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Event-Methode für Vollbildmodus
		*/
		virtual bool event(QEvent *event);

	private:
		void init();
		void createDocks();
		void createActions();
		void createMenuBar();
		void createToolBar();
		// V4DEADCODE
		// void createStatusBar();
		void createMainArea();
		// void createSearchArea(); // obsolete!!

#pragma message("the member names are identical to the corresponding functions in QMainWindow class!")
		// main window elements
		QMenuBar *m_menuBar;
		QToolBar *toolBar;
		// V4DEADCODE
		// QStatusBar *statusBar;
		ISTreeView *m_pFolderView;

		QHBoxLayout *m_hLayout;



		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Variablen für GUI-Darstellung
		*/
	
		ISSearchWidget *m_pSearchWidget;
		ISStatusWidget *m_pStatusWidget;
		ISInfoWidget *m_pInfoWidget;

		QItemSelectionModel* m_pSelectionModel;
		// V4.2: use Qt 4.7 QFileSystemModel instead uf obsolete QDirModel
		//QDirModel m_Folders;
		QFileSystemModel m_Folders;
		QLabel *previewLabel;

		QDockWidget *explorerDock;
		QDockWidget *previewDock;

		/*
		*	Hinzugefügt von Claudius Brämer und David Piegza
		*
		*   Variablen für DockWidgets
		*/
		QDockWidget *searchDock;
		QDockWidget *statusDock;
		QDockWidget *infoDock;

		// QLineEdit *queryLine;
		// QLabel *queryLabel;
		// QPushButton *searchButton;
		// QPushButton *searchSimilarButton;

		// V2 BETA2
		// QPushButton *searchSimilarOnDiskButton;

		// V2 BETA 3
		// QComboBox *inetSearchServiceDropList;
		// QToolButton *inetSearchServiceToolBtn;

		// canvas's
		imageSortWidget *m_pImageSortWidget;
		PreviewCanvas *m_pPreviewCanvas;

		// status bar elements
		QLabel *m_pStatusBarInfoLabel;
		QLabel *m_pStatusBarNumIgnoredLabel;
		QLabel *m_pStatusBarFileSizeLabel;
		QLabel *m_pStatusBarImageSizeLabel;
		QLabel *m_pStatusBarImageNameLabel;
		QLabel *m_pStatusBarImageDateLabel;

		// actions
		QAction *exitAction;
		QAction *aboutAct;
		QAction *bgColorAction;
		QAction *zoomInAction;
		QAction *zoomOutAction;
		QAction *sortByNameAction;
		QAction *sortByColorAction;
		QAction *sortBySizeAction;
		QAction *sortByDateAction;
		QAction *sortBySimilarityAction;
		QAction *internetSetAct;
		QAction *setSortOptionsAction;

		QAction *noWarnAct;
#if defined(Q_WS_MAC)
		QAction *showHiddenAct;
#endif	
		//QAction *resetVisualizationAction;
		QAction *resetZoomAction;
		QAction *fullscreenAction;

		QAction* previewOnMouseOverAction;
		//QAction* highResPreviewAction;
		QAction* resortOnHideMoveDeleteAction;
		//V3BETA2
		QAction* loadRecursiveAction;

		// menubar menus
		QMenu *fileMenu;
		QMenu *viewMenu;
		QMenu *optionsMenu;
		QMenu *helpMenu;

		QActionGroup *sortCriteriaGroup;
		QActionGroup *visualizationGroup;

		// methods
		void updateVisualizationButtons();
		//V4DEADCODE
		// void updateStatusBarInfoLabel(QString text, bool forceRedraw = false);	
		
		void loadSettings();
		void saveSettings();

		// call this to restrict the gui
		void restrictGui();

		// this frees allocated items
		// and resets the imageSortWidget
		// called by dtor and folderChanged()
		void reset();

		//QString formatImageSize(qint64 size);

		// a bool neded to keep track of internally changed selection in the explorer widget
		bool m_bSelectionChangedInternally;

		// see loadSettings() and selectionChanged()...
		bool m_bDelayChangeDirs;

		QSet<QString> m_DirsToRemove;
		QSet<QString> m_DirsToAdd;

};

