#include <QtGui>
#include "isfilterwidgets.h"
#include "issearchwidget.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
ISSearchWidget::ISSearchWidget( QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	create();
}


ISSearchWidget::ISSearchWidget( const QString & title, QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f), m_title(title) {
	create();
}

void ISSearchWidget::create() {
	// Erstellen der GUI für die Bildsuche
	setFixedWidth(230);
	QVBoxLayout *vLayout = new QVBoxLayout;

	if(!m_title.isEmpty()) {
		QLabel *lbl = new QLabel( m_title, this);
		lbl->setMinimumWidth(width());
		lbl->setMinimumHeight(5);
		lbl->setFixedHeight(20);
		lbl->setAlignment(Qt::AlignCenter);
		lbl->setAutoFillBackground(true);
		vLayout->addWidget(lbl);
	}

	QLabel *lblSource = new QLabel(tr("Source:"),this);
	vLayout->addWidget(lblSource);

	m_cbSource = new QComboBox(this);
	m_cbSource->addItem(tr("Hard disk"));
	// V4.3: Yahoo image search service turned off :(
	// - flickr api allows just 30 images to be displayed
	// m_cbSource->addItem("Yahoo");
	// m_cbSource->addItem("flickr");

	m_cbSource->setIconSize(QSize(16,16));
	//m_cbSource->setItemIcon(0, QIcon("Resources/harddisk.png"));
	//m_cbSource->setItemIcon(1, QIcon("Resources/yahooSearchIcon.png"));
	//m_cbSource->setItemIcon(2, QIcon("Resources/flickrSearchIcon.png"));
	m_cbSource->setItemIcon(0, QIcon(":/hardDiskIcon"));
	// m_cbSource->setItemIcon(1, QIcon(":/yahooSearchIcon"));
	m_cbSource->setItemIcon(2, QIcon(":/flickrSearchIcon"));
	m_cbSource->setMinimumHeight(20);
	m_cbSource->setMaximumHeight(30);
	m_cbSource->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	vLayout->addWidget(m_cbSource);

	QHBoxLayout *layoutKeyword = new QHBoxLayout();
	m_lblKeyword = new QLabel(tr("Filename:"),this);
	layoutKeyword->addWidget(m_lblKeyword);
	layoutKeyword->addStretch();
	
	QPushButton *resetKeyword = new QPushButton();
	//resetKeyword->setStyleSheet("background-image:url(Resources/cross.png); border: 0px;");
#if defined(Q_WS_WIN)
	resetKeyword->setStyleSheet("background-image:url(:/cross); border: 0px;");
#else // MAC
	QIcon icn(":/cross");
	resetKeyword->setIcon(icn);
	resetKeyword->setFlat(true);
	resetKeyword->setMaximumHeight(16);
#endif
	resetKeyword->setMaximumWidth(16);

	layoutKeyword->addWidget(resetKeyword);
	vLayout->addLayout(layoutKeyword);

	m_leKeyword = new QLineEdit(this);
#if defined(Q_WS_WIN)	
    m_leKeyword->setStyleSheet("background-color: white");
#endif    

	connect(m_leKeyword, SIGNAL(returnPressed()), SLOT(searchClicked()));
	vLayout->addWidget(m_leKeyword);

	connect(resetKeyword,SIGNAL(clicked()), m_leKeyword, SLOT(clear()));
	
	vLayout->addSpacerItem(new QSpacerItem( 10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum ));

	QHBoxLayout *layoutFilter = new QHBoxLayout();
	QLabel *lblFilter = new QLabel(tr("File Filters:"),this);
	layoutFilter->addWidget(lblFilter);
	layoutFilter->addStretch();
	QPushButton *resetFilter = new QPushButton();
	//resetFilter->setStyleSheet("background-image:url(Resources/cross.png); border: 0px;");
#if defined(Q_WS_WIN)
	resetFilter->setStyleSheet("background-image:url(:/cross); border: 0px;");
#else // MAC
	resetFilter->setIcon(icn);
	resetFilter->setFlat(true);
	resetFilter->setMaximumHeight(16);
#endif
	resetFilter->setMaximumWidth(16);
	layoutFilter->addWidget(resetFilter);

	vLayout->addLayout(layoutFilter);

	m_cbFormat = new QComboBox(this);
	m_cbFormat->addItem(tr("any filetype"));
	m_cbFormat->addItem("jpeg");
	m_cbFormat->addItem("png");
	m_cbFormat->addItem("gif");
	m_cbFormat->addItem("bmp");
	m_cbFormat->addItem("tif");
	m_cbFormat->setMinimumHeight(20);
	m_cbFormat->setMaximumHeight(25);
	m_cbFormat->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	vLayout->addWidget(m_cbFormat);

	m_cbSize = new QComboBox(this);
	m_cbSize->addItem(tr("any size"));
	m_cbSize->addItem(tr("small"));
	m_cbSize->addItem(tr("medium"));
	m_cbSize->addItem(tr("large"));
	m_cbSize->setMinimumHeight(20);
	m_cbSize->setMaximumHeight(25);
	m_cbSize->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	vLayout->addWidget(m_cbSize);

	m_cbColoration = new QComboBox(this);
	m_cbColoration->addItem(tr("any colors"));
	m_cbColoration->addItem(tr("color"));
	m_cbColoration->addItem(tr("black and white"));
	m_cbColoration->setMinimumHeight(20);
	m_cbColoration->setMaximumHeight(25);
	m_cbColoration->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	vLayout->addWidget(m_cbColoration);

	m_cbOrientation = new QComboBox(this);
	m_cbOrientation->addItem(tr("any orientation"));
	m_cbOrientation->addItem(tr("portrait"));
	m_cbOrientation->addItem(tr("landscape"));
	m_cbOrientation->setMinimumHeight(20);
	m_cbOrientation->setMaximumHeight(25);
	m_cbOrientation->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	vLayout->addWidget(m_cbOrientation);

	vLayout->addSpacerItem(new QSpacerItem( 10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum ));

	QHBoxLayout *layoutVFilter = new QHBoxLayout();
	QLabel *lblVisualFilter = new QLabel(tr("Visual Filters:"),this);
	layoutVFilter->addWidget(lblVisualFilter);
	layoutVFilter->addStretch();
	QPushButton *resetVFilter = new QPushButton();
	// resetVFilter->setStyleSheet("background-image:url(Resources/cross.png); border: 0px;");
#if defined(Q_WS_WIN)
	resetVFilter->setStyleSheet("background-image:url(:/cross); border: 0px;");
#else // MAC
	resetVFilter->setIcon(icn);
	resetVFilter->setFlat(true);
	resetVFilter->setMaximumHeight(16);
#endif
	resetVFilter->setMaximumWidth(16);
	layoutVFilter->addWidget(resetVFilter);

	vLayout->addLayout(layoutVFilter);

	m_tbVisualFilter = new QTabWidget(this);
	m_filterImages = new ISFilterImages();
	m_filterSketch = new ISFilterSketch();
	m_filterColor = new ISFilterColor();

#if defined(Q_WS_MAC)
	m_tbVisualFilter->addTab(m_filterImages, tr("Samples"));
#else //Windows
	m_tbVisualFilter->addTab(m_filterImages, tr("Example Images"));
#endif
	m_tbVisualFilter->addTab(m_filterSketch, tr("Sketch"));	
	m_tbVisualFilter->addTab(m_filterColor, tr("Color"));

	vLayout->addWidget(m_tbVisualFilter);

	connect(resetFilter,SIGNAL(clicked()), SLOT(clearFilter()));
	connect(resetVFilter,SIGNAL(clicked()), SLOT(clearVFilter()));

	//vLayout->addSpacerItem(new QSpacerItem( 10, 0, QSizePolicy::Minimum, QSizePolicy::Minimum ));

	QPushButton *btnSearch = new QPushButton(tr("Search Images"));
	btnSearch->setDefault(true);
	btnSearch->setFixedWidth(150);
	btnSearch->setMinimumHeight(20);
	btnSearch->setMaximumHeight(35);
	btnSearch->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	btnSearch->setObjectName("searchBtn");

	vLayout->addWidget(btnSearch, 0, Qt::AlignHCenter);

	vLayout->addStretch();
	setLayout(vLayout);

	connect(m_cbSource,SIGNAL(currentIndexChanged(int)),this,SLOT(searchSourceChanged(int)));
	connect(btnSearch,SIGNAL(clicked()),this,SLOT(searchClicked()));
}


ISSearchWidget::~ISSearchWidget() {
}

/*! Ändert die Überschrift über dem Suchwort, abhängig von ausgewählter Suchquelle
 */ 
void ISSearchWidget::searchSourceChanged(int index) {
	m_cbFormat->setEnabled(index!=2);
	m_cbSize->setEnabled(index!=2);
	m_cbColoration->setEnabled(index!=2);
	m_cbOrientation->setEnabled(index!=2 );
	if(index==0) {
		m_lblKeyword->setText(tr("Filename:"));
	} else if(index==1) {
		m_lblKeyword->setText(tr("Keyword:"));
		//m_cbSize->setEnabled(false);
	} else if(index==2) {
		m_lblKeyword->setText(tr("Keyword, Title, Tags:"));
	}
}


/*! Setzt die Suchoptionen in der Struktur, wenn der Suchbutton geklickt wurde
 */
void ISSearchWidget::searchClicked() {
	searchOptions options;
	options.m_Source = m_cbSource->currentIndex();
	options.m_Keyword = m_leKeyword->text();
	if(m_cbFormat->currentIndex() > 0)
		options.m_Format = m_cbFormat->currentText();
	options.m_Size = m_cbSize->currentIndex();
	options.m_Orientation = m_cbOrientation->currentIndex();
	options.m_Coloration = m_cbColoration->currentIndex();
	
	// Note in Versions prior to V4.3, we could just search for either one ore more image(s), a single sketch or up to four colors
	// we now van search for one or more images(s) and/or a single sketch and/or up to four colors
	// options.m_VisualFilter = m_tbVisualFilter->currentIndex();
	options.m_Thumbnails.clear();
	options.m_Colors.clear();
	// add image(s), if any
	options.m_Thumbnails = m_filterImages->thumbnails();
	// add sketch (note sketchImage() returns an invalid thumbnail if there is no sketch)
	thumbnail sketch=m_filterSketch->sketchImage();
	if(sketch.isValid())
		options.m_Thumbnails.append(sketch);
	// add colors (if any)
	m_filterColor->getColors(options.m_Colors);

	emit search(options);
}

/*! Setzt die Dateifilter zurück
 */
void ISSearchWidget::clearFilter() {
	m_cbFormat->setCurrentIndex(0);
	m_cbSize->setCurrentIndex(0);
	m_cbColoration->setCurrentIndex(0);
	m_cbOrientation->setCurrentIndex(0);
}


/*! Setzt die visuellen Filter zurück.
 */ 
void ISSearchWidget::clearVFilter() {
	switch(m_tbVisualFilter->currentIndex()) {
		case 0:
			m_filterImages->clearImages();
			break;
		case 1:
			m_filterSketch->clearSketch();
			break;
		case 2:
			m_filterColor->clearColors();
			break;
	}
}


void ISSearchWidget::setExImgBackgroundColor(const QColor& color) {
	m_filterImages->setStyleSheet("background-color:" + color.name());
}