#include "isfilterwidgets.h"
#include "som.h"
#include "thumbnail.h"
#include <assert.h>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
#include "thumbnailcalculator.h"

/*! Constructs the container widget.
 */ 
ISFilterImages::ISFilterImages( QWidget * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	setAcceptDrops(true);
	m_uNonColorMapPlacesX = 0;
	m_uNonColorMapPlacesY = 0;
	m_dNonColorThumbSizeX = 0;
	m_dNonColorThumbSizeY = 0;
	setMouseTracking(true);
}

/*! Destroys the container widget.
 */
ISFilterImages::~ISFilterImages() {
}


/*! Is called when a drag is in progress and the mouse enters this widget.
 *
 *  If the event contains an image, the widget accepts this drag event.
 */
void ISFilterImages::dragEnterEvent(QDragEnterEvent *event) {
	// V4.2: this is a bug, since we can drag objects from *outside* ImageSorterV4 onto the the ISFilterImages widget (i.e. from the desktop)
	// when casting statically we crash here...
	//const ISMimeData *data = static_cast<const ISMimeData *>(event->mimeData());
	const ISMimeData* data=qobject_cast<const ISMimeData *>(event->mimeData());
	if(data) {
		if(!m_thumbnames.contains(data->getThumbnail().nameToSort(),Qt::CaseInsensitive)){
			event->acceptProposedAction();
		}
	}
}


/*!
 */
void ISFilterImages::mouseMoveEvent(QMouseEvent *event) {
	if(m_thumbnails.count()) {
		thumbnail *pItem = getPictureAt(event->pos());
		if(pItem==0)
			unsetCursor();
		if(pItem!=m_thumb) {
			m_thumb = pItem;
			setCursor(Qt::PointingHandCursor);
			update();
		}
	}
}


void ISFilterImages::mousePressEvent(QMouseEvent *event) {
	if(event->button()==Qt::LeftButton){
		if( removePictureAt(event->pos()) ) {
			setThumbSize();
			unsetCursor();
			update();
		}
	}
}


void ISFilterImages::leaveEvent(QEvent * event) {
	m_thumb=0;
	unsetCursor();
}


/*! Is called when the drag is dropped on this widget.
 *
 *  If the accepted drag event is dropped on this widget, the
 *  image is read from the event and converted to an thumbnail object.
 */
void ISFilterImages::dropEvent(QDropEvent *event) {
	// Read the MimeData
	const ISMimeData *data = static_cast<const ISMimeData *>(event->mimeData());
	
	// Create a thumbnail object and save the image
	if(data) {
		thumbnail thumb = data->getThumbnail();
		m_thumbnames.append(thumb.nameToSort());
		// Add thumbnail to the list
		m_thumbnails.append(thumb);

		setThumbSize();
		// Repaint the widget with the new thumbnail
		update();
		event->acceptProposedAction();
	}
}


/*! Receive paint events.
 *  This function draws all thumbnail images.
 */
void ISFilterImages::paintEvent(QPaintEvent *e) {
	drawThumbnails();
	//drawGrid();
}


/*!
 */
void ISFilterImages::drawThumbnails() {
	QLinkedList<thumbnail>::iterator it = m_thumbnails.begin();
	QLinkedList<thumbnail>::const_iterator itEnd = m_thumbnails.constEnd();
	while(it!=itEnd) {
		QPainter painter(this);
		thumbnail* pItem = &(*it);
		++it;
		
		if(!pItem)
			continue;

		QImage imgToDraw = pItem->m_Img;
		QRect rect = pItem->drawnRect();

		const int xs = rect.x();
		const int xLen = rect.width();
		const int ys = rect.y();
		const int yLen = rect.height();

		int w = imgToDraw.width();
		int h = imgToDraw.height();

		QRectF target(xs, ys, xLen, yLen);
		QRectF source(0.0,0.0,w,h);
		
		if(m_thumb && m_thumb==pItem) {
			painter.setOpacity(0.5);
			painter.drawImage(target,imgToDraw,source);
			painter.setOpacity(1.0);
			// QPixmap pixmap("Resources/cross.png");
			QPixmap pixmap(":/cross");
			painter.drawPixmap(xs+xLen-15,ys, pixmap);
		} else {
			painter.drawImage(target,imgToDraw,source);
		}
	}
}


void ISFilterImages::drawGrid() {
	QPainter painter(this);
	painter.setPen(Qt::red);
	for(unsigned int y=0; y < m_uNonColorMapPlacesY; ++y ) {
		int startY = y*m_dNonColorThumbSizeY;
		for(unsigned int x=0; x < m_uNonColorMapPlacesX; ++x ) {
			int startX = x*m_dNonColorThumbSizeX;
			painter.drawLine(startX, startY, startX+m_dNonColorThumbSizeX, startY);
			painter.drawLine(startX, startY, startX, startY + m_dNonColorThumbSizeY);
		}
	}
}


thumbnail* ISFilterImages::getPictureAt( const QPoint &pos ) {
	QLinkedList<thumbnail>::iterator it = m_thumbnails.begin();
	QLinkedList<thumbnail>::const_iterator itEnd = m_thumbnails.constEnd();
	while(it!=itEnd) {
		if((pos.x() > it->drawnRect().x()) && 
			(pos.x() < it->drawnRect().x() + it->drawnRect().width()) &&
			(pos.y() > it->drawnRect().y()) && 
			(pos.y() < it->drawnRect().y() + it->drawnRect().height())) {
				return &(*it);
		}
		++it;
	}
	return 0;
}

bool ISFilterImages::removePictureAt( const QPoint &pos ) {
	QLinkedList<thumbnail>::iterator it = m_thumbnails.begin();
	QLinkedList<thumbnail>::const_iterator itEnd = m_thumbnails.constEnd();
	while(it!=itEnd) {
		if((pos.x() > it->drawnRect().x()) && 
			(pos.x() < it->drawnRect().x() + it->drawnRect().width()) &&
			(pos.y() > it->drawnRect().y()) && 
			(pos.y() < it->drawnRect().y() + it->drawnRect().height())) {
				int index = m_thumbnames.indexOf(it->nameToSort());
				if(index!=-1)
					m_thumbnames.removeAt(index);
				m_thumbnails.erase(it);
				return true;
		}
		++it;
	}
	return false;
}


void ISFilterImages::setThumbSize() {
	const int wCanvas = width();
	const int hCanvas = height();
	int nThumbs = m_thumbnails.count();

	if (nThumbs < 2)
		nThumbs = 2;

	unsigned ts = sqrt((double)wCanvas*hCanvas/(nThumbs));
	while(ts>0&&(wCanvas/ts)*(hCanvas/ts)<nThumbs)
		--ts;


	if(ts > 0) {
		m_uNonColorMapPlacesX = wCanvas / ts;
		m_uNonColorMapPlacesY = hCanvas / ts;
		m_dNonColorThumbSizeX=(double)wCanvas/m_uNonColorMapPlacesX;
		m_dNonColorThumbSizeY=(double)hCanvas/m_uNonColorMapPlacesY;

		// avoid empty lines at the bottom 
		while (m_uNonColorMapPlacesX*(m_uNonColorMapPlacesY-1) >= nThumbs) {
			--m_uNonColorMapPlacesY;
			m_dNonColorThumbSizeY = (double)hCanvas / m_uNonColorMapPlacesY;
		}
	}

	QLinkedList<thumbnail>::iterator it = m_thumbnails.begin();
	QLinkedList<thumbnail>::const_iterator itEnd = m_thumbnails.constEnd();
	int x = 0;
	int y = 0;
	while(it!=itEnd) {
		thumbnail* pItem = &(*it);
		++it;	

		//double w = m_dNonColorThumbSizeX-5;
		//double h = m_dNonColorThumbSizeY-5;
		double w = m_dNonColorThumbSizeX*0.9;
		double h = m_dNonColorThumbSizeY*0.9;

		if(pItem->m_Img.width() < w )
			w = pItem->m_Img.width();
		double tmpw = pItem->m_Img.width()/w;
		double tmph = pItem->m_Img.height()/h;
		if(tmpw>tmph) {
			h = pItem->m_Img.height()/tmpw;
			w = pItem->m_Img.width()/tmpw;
		} else {
			h = pItem->m_Img.height()/tmph;
			w = pItem->m_Img.width()/tmph;
		}

		//pItem->0 = QRect(x*m_dNonColorThumbSizeX,y*m_dNonColorThumbSizeY,w, h);
		//pItem->m_DrawnRect = QRect(x*m_dNonColorThumbSizeX + (m_dNonColorThumbSizeX-w)/2,
		//							y*m_dNonColorThumbSizeY + (m_dNonColorThumbSizeY-h)/2, 
		//							w, h);
		pItem->setUpperLeftX(x*m_dNonColorThumbSizeX + (m_dNonColorThumbSizeX-w)/2);
		pItem->setUpperLeftY(y*m_dNonColorThumbSizeY + (m_dNonColorThumbSizeY-h)/2);
		pItem->setLowerRightX(x*m_dNonColorThumbSizeX + (m_dNonColorThumbSizeX-w)/2 + w);
		pItem->setLowerRightY(y*m_dNonColorThumbSizeY + (m_dNonColorThumbSizeY-h)/2 + h);
	
		if(x >= m_uNonColorMapPlacesX-1) {
			x = 0;
			++y;
		} else
			x++;
	}
}


void ISFilterImages::clearImages() {
	m_thumbnails.clear();
	m_thumbnames.clear();
	m_thumb=0;
	update();
}




ISPenWidthButton::ISPenWidthButton(int penWidth, QWidget * parent)
: QPushButton( parent), m_penWidth(penWidth), m_color(0) {
	setMinimumSize(penWidth*2+4,penWidth*2+4);
}


void ISPenWidthButton::paintEvent(QPaintEvent *e) {
	QPainter painter(this);
	QPen pen(Qt::black);
	pen.setWidth(2);
	painter.setPen(pen);
	if(isChecked()) {
		painter.setBrush(QColor(m_color));
		painter.setOpacity(1);
	} else {
		painter.setBrush(Qt::black);
		painter.setOpacity(0.5);
	}
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawEllipse(QPointF(m_penWidth+2,m_penWidth+2), m_penWidth, m_penWidth);
}


/*! Constructs the paint widget.
 */
ISFilterSketch::ISFilterSketch( QWidget * parent, Qt::WindowFlags f )
: QWidget(parent, f), m_penWidth(10), m_selectedColor(0), m_image(NULL) {
	setMouseTracking(true);
	m_paintArea.setMinimumHeight(100);
	//m_paintArea.setMinimumHeight(0);
	m_paintArea.setStyleSheet("border: 1px solid black; background-color: none;");
	m_paintArea.setMouseTracking(true);
	//m_paintArea.setStyleSheet("border: 1px solid white;");
	//setMinimumHeight(0);
	//m_image = new QImage(185, 149, QImage::Format_RGB32);
	m_image = new QImage(185, 99, QImage::Format_RGB32);
	assert(m_image);
	m_image->fill(-1);
	int iconSize = 40;

	m_bPainted = false;

	QButtonGroup *buttonGroup = new QButtonGroup(this);

	ISPenWidthButton *b2 = new ISPenWidthButton(10);
	b2->setCheckable(true);
	b2->setChecked(true);
	connect(b2, SIGNAL(clicked()), SLOT(setPenWidth()));
	connect(this, SIGNAL(colorChanged(QRgb)), b2, SLOT(setColor(QRgb)));

	buttonGroup->addButton(b2);

	ISPenWidthButton *b3 = new ISPenWidthButton(15);
	b3->setCheckable(true);
	connect(b3, SIGNAL(clicked()), SLOT(setPenWidth()));
	connect(this, SIGNAL(colorChanged(QRgb)), b3, SLOT(setColor(QRgb)));
	buttonGroup->addButton(b3);

	ISPenWidthButton *b4 = new ISPenWidthButton(20);
	b4->setCheckable(true);
	connect(b4, SIGNAL(clicked()), SLOT(setPenWidth()));
	connect(this, SIGNAL(colorChanged(QRgb)), b4, SLOT(setColor(QRgb)));
	buttonGroup->addButton(b4);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addStretch();
	hbox->addWidget(b2);
	hbox->addSpacing(3);
	hbox->addWidget(b3);
	hbox->addSpacing(11);
	hbox->addWidget(b4);
	hbox->addStretch();

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(0);

	vbox->addWidget(&m_paintArea);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);
	vbox->addSpacing(10);

	m_lblPalette = new QLabel();
	m_lblPalette->setMouseTracking(true);
	//m_lblPalette->setPixmap(QPixmap("Resources/color_chooser2.png"));
	m_lblPalette->setPixmap(QPixmap(":/colorChooser2"));
	m_lblPalette->setScaledContents(true);
	m_lblPalette->setCursor(Qt::CrossCursor);
	m_lblPalette->setToolTip(tr("Click left button to select a color"));
	vbox->addWidget(m_lblPalette);

	setLayout(vbox);
}


/*! Destroys the widget.
 */
ISFilterSketch::~ISFilterSketch() {
	if(m_image)
		delete m_image;
}


/*! Receive paint events.
 *  Paints the sketch image.
 */
void ISFilterSketch::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	if(m_bPainted) {
		QRect source(0,0,m_image->width(),m_image->height());
		painter.drawImage(m_paintArea.frameGeometry(), *m_image, source);
	}/* else {
		QPen pen(Qt::red);
		painter.setPen(pen);
		painter.drawLine(m_paintArea.frameGeometry().topLeft(), m_paintArea.frameGeometry().bottomRight());
	}*/
}


/*! Receive mouse move events for the widget.
 *  This function is called when additionally the left mouse button is pressed.
 *  If the mouse pointer is inside the sketch rect, an ellipse is drawn into an
 *  image object.
 */
void ISFilterSketch::mouseMoveEvent(QMouseEvent * e) {
	if(m_paintArea.frameGeometry().contains(e->pos(),true)) {
		if(e->buttons() == Qt::LeftButton ) {
			//m_sketch.contains(e->pos(),true)) {
			m_bPainted = true;
			QPainter painter(m_image);
			const QColor & col(m_selectedColor);
			QPen pen(col);
			pen.setColor(col);
			pen.setWidth(m_penWidth);
			painter.setPen(pen);
			painter.setBrush(col);
			painter.drawEllipse(e->x()-m_paintArea.x(),e->y()-m_paintArea.y(), m_penWidth, m_penWidth);
			painter.end();
			update();
		}
	} else if( m_lblPalette->frameGeometry().contains(e->pos()) ) {
		QRgb tmpColor  = m_lblPalette->pixmap()->grabWidget(this, e->x(), e->y(), 1, 1).toImage().pixel(0,0);
		emit colorChanged(tmpColor);
		//QString bgColor = QString("background-color: rgb(%1,%2,%3);").arg(qRed(m_selectedColor)).arg(qGreen(m_selectedColor)).arg(qBlue(m_selectedColor));
		//m_lblColor->setStyleSheet("border: 1px solid black; padding: 2px;" + bgColor);
		//m_lblColorValue->setText(QString("Rgb: %1 %2 %3").arg(qRed(m_selectedColor)).arg(qGreen(m_selectedColor)).arg(qBlue(m_selectedColor)));
	} else
		emit colorChanged(m_selectedColor);
	QWidget::mouseMoveEvent(e);
}


void ISFilterSketch::mousePressEvent(QMouseEvent * e) {
	if( e->button() == Qt::LeftButton ) {
		if(m_paintArea.frameGeometry().contains(e->pos(),true)) {
			m_bPainted = true;
			QPainter painter(m_image);
			const QColor & col(m_selectedColor);
			QPen pen(col);
			pen.setColor(col);
			pen.setWidth(m_penWidth);
			painter.setPen(pen);
			painter.setBrush(col);
			painter.drawEllipse(e->x()-m_paintArea.x(),e->y()-m_paintArea.y(), m_penWidth, m_penWidth);
			painter.end();
			update();
		} else if( m_lblPalette->frameGeometry().contains(e->pos()) ) {
			m_selectedColor  = m_lblPalette->pixmap()->grabWidget(this, e->x(), e->y(), 1, 1).toImage().pixel(0,0);
			emit colorChanged(m_selectedColor);
			//QString bgColor = QString("background-color: rgb(%1,%2,%3);").arg(qRed(m_selectedColor)).arg(qGreen(m_selectedColor)).arg(qBlue(m_selectedColor));
			//m_lblColor->setStyleSheet("border: 1px solid black; padding: 2px;" + bgColor);
			//m_lblColorValue->setText(QString("Rgb: %1 %2 %3").arg(qRed(m_selectedColor)).arg(qGreen(m_selectedColor)).arg(qBlue(m_selectedColor)));
		} 
	}
	QWidget::mousePressEvent(e);
}



/*! Resizes the sketch and the corresponding image object.
 */
void ISFilterSketch::resizeEvent(QResizeEvent *e) {
	/*m_sketch.setRect( 0, 0, e->size().width()-1, 150 );
	QImage *tempImg = new QImage(m_sketch.width()-1,m_sketch.height()-1,m_image->format());
	*tempImg = m_image->copy(QRect(0,0,m_sketch.width()-1,m_sketch.height()-1));

	m_image = tempImg;*/
}

/*! Clears the sketch area and the corresponding image.
 */
void ISFilterSketch::clearSketch() {
	m_image->fill(-1);
	m_bPainted = false;
	update();
}
 

void ISFilterSketch::setPenWidth() {
	//QObject *sender = sender();
	ISPenWidthButton *b = static_cast<ISPenWidthButton*>(sender());
	if(b)
		m_penWidth = b->penWidth();
}


/*! Set the pen width to the selected width.
 */
void ISFilterSketch::setPenWidth(int id) {
	m_penWidth = id;
}

/*! Shows a Qt color dialog and sets the selected color.
 */
void ISFilterSketch::showColorDialog() {
///	m_col = QColorDialog::getColor(m_col);
///	m_lblColor->setStyleSheet(QString("background-color: rgb(%1,%2,%3);").arg(m_col.red()).arg(m_col.green()).arg(m_col.blue()));
}

/*! Returns the sketch image converted to a thumbnail object.
 */
thumbnail ISFilterSketch::sketchImage() {
	thumbnail thumb;
	if(m_bPainted) {
		thumb.m_Img = *m_image;
		bool b=thumb.setBaseMembers();
		assert(b);
		// note this is alpha ignore mode
		thumbnailCalculator::calculateFeatureData(&thumb);
		assert(thumb.featureDataValid());
		thumb.validate();
	}
	return thumb;
}





ISColorLabel::ISColorLabel( QWidget *parent, Qt::WindowFlags f )
: QLabel( parent, f ) {
	setCursor(Qt::PointingHandCursor);
	setMinimumHeight(0);
}

void ISColorLabel::mousePressEvent(QMouseEvent *e) {
	if( e->button() == Qt::LeftButton ) {
		emit clicked(this);
	}
	QLabel::mousePressEvent(e);
}

void ISColorLabel::setColor(QRgb color) {
	m_color = color;
	update();
	//QString bgColor = QString("background-color: rgb(%1,%2,%3);").arg(qRed(m_color)).arg(qGreen(m_color)).arg(qBlue(m_color));
	//setStyleSheet("border: 1px solid black; padding: 2px;" + bgColor);
}

void ISColorLabel::paintEvent(QPaintEvent *e) {
	QPainter painter(this);
	painter.setBrush(QColor(m_color));
	QRect r=rect();
	r.setWidth(r.width()-20);
	r.setHeight(r.height()-1);
	painter.drawRect(r);
	painter.drawPixmap(width()-pixmap()->width(),0,pixmap()->width(), pixmap()->height(), *pixmap());
}



ISFilterColor::ISFilterColor( QWidget * parent, Qt::WindowFlags f )
: QWidget(parent, f) {
	setMouseTracking(true);
	//setMinimumHeight(0);
	m_numColors = 0;
	m_showRemoveColor = false;
	m_tmpLabel = 0;

	//palette.load(QPixmap"Resources/palette.png");
	//palette.load("Resources/color_chooser1.png");
	palette.load(":/colorChooser1");

	selectedColor = qRgb(255,255,255);

	m_lblPalette = new QLabel(this);
	m_lblPalette->setPixmap(palette.scaled(palette.width(), 105));
	m_lblPalette->setCursor(Qt::CrossCursor);
	m_lblPalette->setMouseTracking(true);
	//m_lblPalette->setMinimumHeight(0);
	//m_lblPalette->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	//setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

#if defined(Q_WS_MAC)
	m_lblText = new QLabel(tr("Select up to 4 colors"), this);
#else
	m_lblText = new QLabel(tr("<b>Please select up to 4 colors</b>"), this);
#endif
	//m_vbLayout.addWidget(m_lblPalette);
	//m_vbLayout.addWidget(m_lblText);
	m_vbLayout.insertWidget(0, m_lblText);
	m_vbLayout.addStretch(150);
	m_vbLayout.insertWidget(5, m_lblPalette);
	//m_vbLayout.addStretch();

	setLayout(&m_vbLayout);
}

ISFilterColor::~ISFilterColor() {
}


void ISFilterColor::leaveEvent(QEvent *) {
	if(m_tmpLabel) {
		m_vbLayout.removeWidget(m_tmpLabel);
		delete m_tmpLabel;
		m_tmpLabel = 0;
	}
}


void ISFilterColor::mouseMoveEvent(QMouseEvent *e) {
	if( m_lblPalette->frameGeometry().contains(e->pos()) && m_colorList.count() < 4 ) {
		m_image = palette.grabWidget(this, e->x(), e->y(), 1, 1).toImage();
		selectedColor = m_image.pixel(0, 0);

		if(!m_tmpLabel ) {
			m_tmpLabel = new ISColorLabel(this);
			m_tmpLabel->setPixmap(QPixmap(":/cross"));
			m_tmpLabel->setMaximumHeight(18);
			//lbl->setMinimumWidth(90);
			m_tmpLabel->setToolTip(tr("Click to remove this color"));
			connect(m_tmpLabel, SIGNAL(clicked(QWidget*)), this, SLOT(removeWidget(QWidget*)));
			
			//m_vbLayout.insertWidget(m_vbLayout.count()-1, m_tmpLabel);
			m_vbLayout.insertWidget(m_colorList.count()+1, m_tmpLabel);
		}
		m_tmpLabel->setColor(selectedColor);
	} else {
		if(m_tmpLabel) {
			m_vbLayout.removeWidget(m_tmpLabel);
			delete m_tmpLabel;
			m_tmpLabel = 0;
		}
	}
}

void ISFilterColor::mouseReleaseEvent(QMouseEvent *e) {
	if( e->button() == Qt::LeftButton ) {
		if(m_tmpLabel) {
			//ISColorLabel *lbl = new ISColorLabel(*m_tmpLabel);
			//m_vbLayout.removeWidget(m_tmpLabel);
			//delete m_tmpLabel;
			m_colorList.append(m_tmpLabel);

			//m_vbLayout.insertWidget(m_vbLayout.count()-1, lbl);
			//m_vbLayout.insertWidget(m_colorList.count(), m_tmpLabel);
			m_tmpLabel = 0;
		}
		/*if( m_lblPalette->frameGeometry().contains(e->pos()) && m_colorList.count() < 4 ) {
			m_image = palette.grabWidget(this, e->x(), e->y(), 1, 1).toImage();
			selectedColor = m_image.pixel(0, 0);

			ISColorLabel *lbl = new ISColorLabel(this);
			lbl->setPixmap(QPixmap(":/cross"));
			lbl->setMaximumHeight(18);
			//lbl->setMinimumWidth(90);
			lbl->setToolTip(tr("Click to remove this color"));
			lbl->setColor(selectedColor);
			connect(lbl, SIGNAL(clicked(QWidget*)), this, SLOT(removeWidget(QWidget*)));

			m_colorList.append(lbl);
			m_vbLayout.insertWidget(m_vbLayout.count()-1, lbl);
		} */
		
		if(m_showRemoveColor)
#if defined(Q_WS_MAC)			
			m_lblText->setText(tr("Please remove a color"));
#else
			m_lblText->setText(tr("<b>Please remove a color</b>"));
#endif		
		if(m_colorList.count() < 4 )
#if defined(Q_WS_MAC)
			m_lblText->setText(tr("Select up to 4 colors"));
#else			
			m_lblText->setText(tr("<b>Please select up to 4 colors</b>"));
#endif		
		else {
			m_showRemoveColor = true;
		}
	}
	
}

void ISFilterColor::removeWidget(QWidget *widget) {
	m_vbLayout.removeWidget(widget);
	m_colorList.removeOne(static_cast<ISColorLabel*>(widget));
	widget->deleteLater();
	if(m_colorList.count() < 4 ) {
#if defined(Q_WS_MAC)
		m_lblText->setText(tr("Select up to 4 colors"));
#else
		m_lblText->setText(tr("<b>Please select up to 4 colors</b>"));
#endif
		m_showRemoveColor = false;
	}
}


void ISFilterColor::clearColors() {
	QList<ISColorLabel *>::iterator it = m_colorList.begin();
	QList<ISColorLabel *>::const_iterator itEnd = m_colorList.constEnd();
	while(it!=itEnd) {
		ISColorLabel *lbl = *it;
		m_vbLayout.removeWidget(lbl);
		it = m_colorList.erase(it);
		lbl->deleteLater();
	}
#if defined(Q_WS_MAC)
	m_lblText->setText(tr("Select up to 4 colors"));
#else
	m_lblText->setText(tr("<b>Please select up to 4 colors</b>"));
#endif
}


bool ISFilterColor::getColors(QVector<QRgb>& colors) const
{
	if(!m_colorList.size())
		return false;

	colors.clear();

	QList<ISColorLabel *>::const_iterator it = m_colorList.constBegin();
	QList<ISColorLabel *>::const_iterator itEnd = m_colorList.constEnd();
	while(it!=itEnd) {
		colors.push_back((*it)->color());
		++it;
	}

	return true;
}
