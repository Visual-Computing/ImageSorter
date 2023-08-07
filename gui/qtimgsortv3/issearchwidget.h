#ifndef ISSEARCHWIDGET_H
#define ISSEARCHWIDGET_H

#include <QtGui>
#include <QList>
#include <QColor>
#include "thumbnail.h"

class ISFilterImages;
class ISFilterSketch;
class ISFilterColor;

// TODO V4: take this into a separate #include file, we need it elsewhere

/*!
 * Struktur für die Suchoptionen.
 * 
 * \author David Piegza, Claudius Brämer
 */
typedef struct _searchOptions {
	int m_Source;
	QString m_Keyword;
	QString m_Format;
	unsigned int m_Size;
	unsigned int m_Orientation;
	unsigned int m_Coloration;
	// int m_VisualFilter;
	QLinkedList<thumbnail> m_Thumbnails;
	QVector<QRgb> m_Colors;
} searchOptions;



/*!
 * Die ISSearchWidget Klasse implementiert die komplette Suche im DockWidget.
 * 
 * \author David Piegza, Claudius Brämer
 */
class ISSearchWidget : public QFrame {
	Q_OBJECT
public:
	ISSearchWidget( QFrame * parent = 0, Qt::WindowFlags f = 0 );
	ISSearchWidget( const QString & title, QFrame * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISSearchWidget();

	void setExImgBackgroundColor(const QColor& color);

public slots:
	void openExampleImages() { m_tbVisualFilter->setCurrentIndex(0); }

private slots:
	void searchSourceChanged(int);
	void searchClicked();
	void clearFilter();
	void clearVFilter();
 
signals:
	void search(searchOptions &options);

protected:
	virtual void create();

private:
	QString m_title;
	QComboBox *m_cbSource;
	QLineEdit *m_leKeyword;
	QLabel *m_lblKeyword;
	QComboBox *m_cbFormat;
	QComboBox *m_cbColoration;
	QComboBox *m_cbSize;
	QComboBox *m_cbOrientation;
	QTabWidget *m_tbVisualFilter;

	ISFilterImages *m_filterImages;
	ISFilterSketch *m_filterSketch;
	ISFilterColor *m_filterColor;
};

#endif
