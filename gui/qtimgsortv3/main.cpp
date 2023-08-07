#include <isapp.h>

#include "imagesortmainwindow.h"
#include <iostream>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
#if defined(Q_WS_MAC)
#include <QMacStyle>
#endif

int main(int argc, char *argv[])
{
#ifdef _DEBUG
#pragma message("building debug version")
	
#ifdef _WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
#endif
	
#if defined(Q_WS_MAC)
	QApplication::setStyle(new QMacStyle);
#endif
	// QApplication app(argc, argv);
	ISApplication app(argc, argv);

	// note since we call this, the QSettings used here will
	// have a meaningful name on all platforms...
	QCoreApplication::setOrganizationName("Visual Computing Group at the HTW Berlin");
	// this is used on Mac OSX:
	QCoreApplication::setOrganizationDomain("visual-computing.com");
	// do not change minor version number like v2.01
	QCoreApplication::setApplicationName("ImageSorterV4");

	// we'd like to have the plugins in the same folder/directory 
	// as our executable...
	// note, however, that the plugins have to be in an 'imageformats' subfolder
	// *not* in an 'plugins/imageformats' subfolder...
	// we don't want to use propably other qt plugins on the target system
	// thus use setLibraryPaths() to delete all standard pathes
	
	QStringList paths;
	paths.append(app.applicationDirPath());
	
	#if defined(Q_WS_MAC)
		QDir pluginDir(QApplication::applicationDirPath());
		pluginDir.cdUp();
		pluginDir.cd("Plugins");
		paths.append(pluginDir.absolutePath());	
	#endif
	
	app.setLibraryPaths(paths);

	//foreach(const QByteArray &fmt, QImageReader::supportedImageFormats()){
	//	qDebug() << QString(fmt);
	//}

	// check if there is one and only one translation in the app's dir
	// if so, use it, otherwise, language is english...
	// we need the translator in this scope...
	QTranslator translator;
	QString appDirStr=QCoreApplication::applicationDirPath();
	QDir dir(appDirStr);
#if defined(Q_WS_MAC)
	dir.cdUp();
	dir.cd("Translations");
#endif
	QStringList fileNames=dir.entryList(QStringList("*.qm"),QDir::Files);
	if(fileNames.size()==1){
		if(translator.load(fileNames[0],appDirStr))
			app.installTranslator(&translator);
	}

	/*
	*	Hinzugefügt von Claudius Brämer und David Piegza
	*
	*   Stylesheet-Anbindung
	*/
	// Bugfix: style file was only found when started from visual studio...
	// new V4: we have a 'styles' subfolder in the release/debug folder
	// note we need this at run time, therefore it has to be added to the setup file

	// Note the style sheet stuff leads to very starange results on OS X, leave out in V4.2
#if defined(Q_WS_MAC)
	/*
	QDir qssDir(appDirStr);
	qssDir.cdUp();
	qssDir.cd("styles");
	QString styleFilePath=qssDir.absoluteFilePath("osx_style.qss");
	*/
#else
	QString styleFilePath=appDirStr+"/styles/style.qss";
#endif

#if defined(Q_WS_WIN)
    QFile file(styleFilePath);
	if(file.open(QFile::ReadOnly)){
		QString styleSheet = QLatin1String(file.readAll());
		app.setStyleSheet(styleSheet);
	}
#endif

	// V4.2: we need this ctor after setting the style sheet, otherwise
	// the explorer/preview/info dock widgets state are not restored...
	imageSortMainWindow isort;

    isort.show();

    return app.exec();
}
