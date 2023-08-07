
#include "isextensionwidget.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif


/*
ISProgressBar::ISProgressBar( QWidget * parent, Qt::WindowFlags f )
: QFrame( parent, f ) {

	setCursor(Qt::ArrowCursor);
	m_Label = new QLabel("Loading images...");
	m_progressBar = new QProgressBar();
	m_progressBar->setObjectName("progress");
	m_progressBar->setTextVisible(false);
	m_progressBar->setMinimumHeight(35);
	QPushButton *b = new QPushButton("Stop");
	b->setMaximumWidth(75);
	b->setMaximumHeight(25);
	b->setObjectName("loadCancel");
	QVBoxLayout *vLayout = new QVBoxLayout;
	
	QHBoxLayout *hLayout1 = new QHBoxLayout;
	QHBoxLayout *hLayout2 = new QHBoxLayout;
	QHBoxLayout *hLayout3 = new QHBoxLayout;

	m_Label->setObjectName("progressBarLabel");
	m_Label->setAlignment(Qt::AlignCenter);

	hLayout1->addWidget(m_Label);
	hLayout2->addWidget(m_progressBar);
	hLayout3->addWidget(b);

	vLayout->addLayout(hLayout1);
	vLayout->addLayout(hLayout2);
	vLayout->addLayout(hLayout3);
    setLayout(vLayout);
	
	QObject::connect(b, SIGNAL(clicked()),this, SIGNAL(clickedCancel())); 
	reset();
}


void ISProgressBar::reset() {
	m_progressBar->setMaximum(0);
	m_progressBar->setMinimum(0);
	m_progressBar->setValue(0);
	m_Label->setText("Loading images...");
}


void ISProgressBar::setValue(int value) {
	//m_value = value;
	m_progressBar->setValue(value);
}
*/


ISMsgWidget ::ISMsgWidget( QWidget * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	
	setCursor(Qt::ArrowCursor);
	m_Label = new QLabel();
	m_Label->setObjectName("msgWidgetLabel");
	m_Label->setAlignment(Qt::AlignCenter);
	m_Label->setWordWrap(true);

	vLayout = new QVBoxLayout;
	vLayout->setMargin(10);
	hLayout1 = new QHBoxLayout;
	hLayout2 = new QHBoxLayout;

	hLayout1->addWidget(m_Label);
	vLayout->addLayout(hLayout1);
	vLayout->addLayout(hLayout2);
    setLayout(vLayout);
}

ISMsgWidget::~ISMsgWidget() {
}

void ISMsgWidget::setText(const QString& text) {
	m_Label->setText(text);
}

QPushButton* ISMsgWidget::addButton(const QString& text) {
	QPushButton *pb = new QPushButton(text);
	m_pushButtons.append(pb);
#if defined(Q_WS_WIN)	
	pb->setMaximumHeight(25);
	pb->setMaximumWidth(75);
#endif	
	pb->setObjectName("startButton");
	hLayout2->addWidget(pb);
	//vLayout->addWidget(startButton);
	
	//QObject::connect(startButton, SIGNAL(clicked()),this, SIGNAL(clicked())); 
	//startButton->setAlignment(Qt::AlignHCenter);
	return pb;
}

void ISMsgWidget::reset() {
	m_Label->clear();
	if(m_pushButtons.size()) {
		QList<QPushButton *>::iterator it = m_pushButtons.begin();
		QList<QPushButton *>::const_iterator itEnd = m_pushButtons.constEnd();
		while(it!=itEnd) {
			delete (*it);
			//(*it)->deleteLater();
			it = m_pushButtons.erase(it);
		}
	}
}

