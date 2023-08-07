#ifndef ISSTATUSWIDGET_H
#define ISSTATUSWIDGET_H

#include <QtGui>

#include "imagesortwidget.h"



/*!
 * Die ISStatusWidget Klasse implementiert den Status-Bereich.
 * 
 * \author David Piegza, Claudius Brämer
 */
class ISStatusWidget : public QFrame {
	Q_OBJECT

public:
	ISStatusWidget( QFrame * parent = 0, Qt::WindowFlags f = 0 );
	ISStatusWidget( const QString & title, QFrame * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISStatusWidget();
	
public slots:
	void setStatusWidget(QString strNumDirs, QString strNumFiles);
	void setNumSelected(QString str);

protected:
	virtual void create();

private:
	QLabel *numDirs;
	QLabel *numFiles;
	QLabel *numSelFiles;

};

#endif
