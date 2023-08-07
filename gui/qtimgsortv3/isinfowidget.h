#ifndef ISINFOWIDGET_H
#define ISINFOWIDGET_H

#include <QtGui>

#include "previewcanvas.h"

/*!
 * Die ISInfoWidget Klasse implementiert den Info-Bereich für ein Vorschaubild.
 * 
 * \author David Piegza, Claudius Brämer
 */
class ISInfoWidget : public QFrame {
	Q_OBJECT

public:
	ISInfoWidget( QFrame * parent = 0, Qt::WindowFlags f = 0 );
	ISInfoWidget( const QString & title, QFrame * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISInfoWidget();

public slots:
	void showImageInfo(const imageInfo&);

protected:
	virtual void create();
private:
	QLabel *nameLineEdit;
	QLabel *sizeLineEdit;
	QLabel *orientationLineEdit;
	// QLabel *lastModLineEdit;
};

#endif
