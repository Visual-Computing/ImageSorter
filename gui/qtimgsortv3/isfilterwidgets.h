#ifndef ISFILTERWIDGETS_H
#define ISFILTERWIDGETS_H

#include <QtGui>
#include "thumbnail.h"

/* This file contains several classes which implements the visual filters
 * for the image search.
 */



class ISMimeData : public QMimeData 
{
	// V4.2 needed to call cobject_cast...
	Q_OBJECT
public:
	ISMimeData() {}
	virtual ~ISMimeData() {}

public:
	void setThumbnail(const thumbnail &tn) {m_thumbnail = tn;}
	thumbnail getThumbnail() const {return m_thumbnail;}

private:
	thumbnail m_thumbnail;
};



/*!
 * The ISFilterImages class provides a container widget for thumbnail images.
 * 
 * The class can take QImage objects by Drag & Drop and paint those in a widget.
 * The thumbnails() function returns a QLinkedList object with all images converted
 * to a thumbnail object.
 *
 * \author David Piegza, Claudius Brämer
 */
class ISFilterImages : public QFrame {
public:
	ISFilterImages( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISFilterImages();

	QLinkedList<thumbnail> thumbnails() { return m_thumbnails; }

public:
	void clearImages();

private:
	virtual void dragEnterEvent(QDragEnterEvent *);
	virtual void dropEvent(QDropEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent( QMouseEvent * event );
	virtual void leaveEvent(QEvent *);
	virtual void paintEvent(QPaintEvent *);

	void drawThumbnails();
	void setThumbSize();
	void drawGrid();
	thumbnail* getPictureAt(const QPoint &pos);
	bool removePictureAt( const QPoint &pos );

private:
	QLinkedList<thumbnail> m_thumbnails;
	thumbnail *m_thumb;
	QStringList m_thumbnames;

	int m_uNonColorMapPlacesX;
	int m_uNonColorMapPlacesY;
	double m_dNonColorThumbSizeX;
	double m_dNonColorThumbSizeY;
};



class ISPenWidthButton : public QPushButton {
	Q_OBJECT
public:
	ISPenWidthButton( int penWidth, QWidget *parent = 0 );
	virtual ~ISPenWidthButton() {}

	QRgb color() { return m_color; }

	int penWidth() { return m_penWidth; }

public slots:
	void setColor(QRgb color) { m_color = color; update(); }

private:
	virtual void paintEvent(QPaintEvent *);

private:
	QRgb m_color;
	int m_penWidth;
};



/*!
 * The ISFilterSketch class provides a paint widget.
 * 
 * The class provides a paint widget for painting simple sketches. 
 * The sketchImage() function returns the sketch as thumbnail object.
 *
 * \author David Piegza, Claudius Brämer
 */
class ISFilterSketch : public QWidget {
	Q_OBJECT

public:
	ISFilterSketch( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISFilterSketch();

	thumbnail sketchImage();
	void clearSketch();

private:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent * e);
	virtual void resizeEvent(QResizeEvent *e);

signals:
	void colorChanged(QRgb);

private slots:
	void setPenWidth();
	void setPenWidth(int);
	void showColorDialog();

private:
	int m_penWidth;
	bool m_bPainted;
	QFrame m_paintArea;
	QImage *m_image;
	QLabel *m_lblColor;
	QLabel *m_lblPalette;
	QLabel *m_lblColorValue;
	QRgb m_selectedColor;
};



class ISColorLabel : public QLabel {
	Q_OBJECT
public:
	ISColorLabel( QWidget *parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISColorLabel() {}

	void setColor(QRgb color);
	QRgb color() const { return m_color; }

signals:
	void clicked(QWidget *);

protected:
	virtual void paintEvent(QPaintEvent *);

private:
	virtual void mousePressEvent(QMouseEvent *);

private:
	QRgb m_color;
};


/*!
 * The ISFilterColor class provides a color picker.
 * 
 * The class draws a color palette, the selected color is displayed
 * below the palette. The colorImage() function returns an thumbnail
 * object which contains an image filled with the selected color. 
 *
 * \author David Piegza, Claudius Brämer
 */
class ISFilterColor : public QWidget {
	Q_OBJECT
public:
	ISFilterColor( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	ISFilterColor(const ISFilterColor & fc);
	virtual ~ISFilterColor();

	unsigned numColors() const {return m_colorList.size();}
	bool getColors(QVector<QRgb>& colors) const;
	void clearColors();

private:
	QImage m_image;
	virtual void leaveEvent(QEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);

private slots:
	void removeWidget(QWidget*);

private:
	QPixmap palette;
	QLabel *m_lblPalette;
	QRgb selectedColor;
	QVBoxLayout m_vbLayout;
	QGridLayout m_gridLayout;
	int m_numColors;
	QLabel *m_lblText;
	QList<ISColorLabel *> m_colorList;
	ISColorLabel *m_tmpLabel;
	bool m_showRemoveColor;
};

#endif
