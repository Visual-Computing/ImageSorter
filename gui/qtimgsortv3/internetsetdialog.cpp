#include "internetsetdialog.h"

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
InternetSetDialog::InternetSetDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("Search Options"));
	setWindowModality(Qt::WindowModal);
#if defined(Q_WS_WIN)	
	setFixedWidth(400);
	setFixedHeight(180);
#else //Q_WS_MAC
	setFixedWidth(510);
	setFixedHeight(180);
#endif
	createDialogArea();

}

InternetSetDialog::~InternetSetDialog()
{
}

void InternetSetDialog::createDialogArea()
{
	numberOfImagesLabel = new QLabel(this);
	numberOfImagesLabel->setText(tr("Number of Images for Internet Search: "));
	simImgLabel = new QLabel(this);
	simImgLabel->setText(tr("Number of Images for Internet Similarity Search: "));
	resImgLabel = new QLabel(this);
	resImgLabel->setText(tr("Number of Results for Similarity Search: "));
	numThreadsLabel = new QLabel(this);
	numThreadsLabel->setText(tr("Number of threads for Internet Search: "));

	// V4.3: no internet search any longer, but maybe later again...
	numberOfImagesLabel->setEnabled(false);
	simImgLabel->setEnabled(false);
	numThreadsLabel->setEnabled(false);

	okButton = new QPushButton(this);
	okButton->setText("Ok");

	cancelButton = new QPushButton(this);
	cancelButton->setText(tr("Cancel"));

	invisibleDefaultButton=new QPushButton(this);
	invisibleDefaultButton->setVisible(false);
	invisibleDefaultButton->setDefault(true);
	
	numImgspinBox = new QSpinBox(this);
	numImgspinBox->setKeyboardTracking(false);
	numImgslider = new QSlider(Qt::Horizontal,this);
	numImgspinBox->setRange(1,1000);
	numImgslider->setRange(1,1000);

	simImgspinBox = new QSpinBox(this);
	simImgspinBox->setKeyboardTracking(false);
	simImgslider = new QSlider(Qt::Horizontal,this);
	simImgspinBox->setRange(1,1000);
	simImgslider->setRange(1,1000);

	resImgspinBox = new QSpinBox(this);
	resImgspinBox->setKeyboardTracking(false);
	resImgslider = new QSlider(Qt::Horizontal,this);
	resImgspinBox->setRange(1,1000);
	resImgslider->setRange(1,1000);

	numThreadsSpinBox = new QSpinBox(this);
	numThreadsSpinBox->setKeyboardTracking(false);
	numThreadsSlider = new QSlider(Qt::Horizontal,this);
	numThreadsSpinBox->setRange(1,50);
	numThreadsSlider->setRange(1,50);

	connect(cancelButton, SIGNAL(clicked()),this, SLOT(reject()));
	connect(okButton, SIGNAL(clicked()),this, SLOT(accept()));

	connect(numImgspinBox, SIGNAL(valueChanged(int)),numImgslider, SLOT(setValue(int)));
    connect(numImgslider, SIGNAL(valueChanged(int)),numImgspinBox, SLOT(setValue(int)));
	connect(numImgslider,SIGNAL(valueChanged(int)),this,SLOT(simSliderByImgSlider(int)));

	connect(simImgspinBox, SIGNAL(valueChanged(int)),simImgslider, SLOT(setValue(int)));
	connect(simImgslider, SIGNAL(valueChanged(int)),simImgspinBox, SLOT(setValue(int)));
	connect(simImgslider, SIGNAL(valueChanged(int)),this, SLOT(imgSliderBySimSlider(int)));
	// V4.3: no internet search any longer, but maybe later again...
	//connect(simImgslider, SIGNAL(valueChanged(int)),this, SLOT(resSliderBySimSlider(int)));
	
	connect(resImgspinBox, SIGNAL(valueChanged(int)),resImgslider, SLOT(setValue(int)));
	connect(resImgslider, SIGNAL(valueChanged(int)),resImgspinBox, SLOT(setValue(int)));
	// V4.3: no internet search any longer, but maybe later again...
	// connect(resImgslider, SIGNAL(valueChanged(int)),this, SLOT(simSliderByResSlider(int)));

	connect(numThreadsSpinBox, SIGNAL(valueChanged(int)),numThreadsSlider, SLOT(setValue(int)));
    connect(numThreadsSlider, SIGNAL(valueChanged(int)),numThreadsSpinBox, SLOT(setValue(int)));

	// V4.3: no internet search any longer, but maybe later again...
	numImgspinBox->setEnabled(false);
	simImgspinBox->setEnabled(false);
	numThreadsSpinBox->setEnabled(false);

	numImgslider->setEnabled(false);
	simImgslider->setEnabled(false);
	numThreadsSlider->setEnabled(false);


#if defined(Q_WS_WIN)
	numberOfImagesLabel->setGeometry(3,3,190,20);
	numImgslider->setGeometry(245,3,100,20);
	numImgspinBox->setGeometry(350,3,45,20);
		
	simImgLabel->setGeometry(3,28,240,20);
	simImgslider->setGeometry(245,28,100,20);
	simImgspinBox->setGeometry(350,28,45,20);

	resImgLabel->setGeometry(3,53,190,20);
	resImgslider->setGeometry(245,53,100,20);
	resImgspinBox->setGeometry(350,53,45,20);

	numThreadsLabel->setGeometry(3,78,190,20);
	numThreadsSlider->setGeometry(245,78,100,20);
	numThreadsSpinBox->setGeometry(350,78,45,20);

	okButton->setGeometry(60,125,100,25);
	cancelButton->setGeometry(400-160,125,100,25);
#else
	numberOfImagesLabel->setGeometry(8,13,310,20);
	numImgspinBox->setGeometry(435,13,65,20);
	numImgslider->setGeometry(320,13,100,20);

	simImgLabel->setGeometry(8,38,310,20);
	simImgspinBox->setGeometry(435,38,65,20);
	simImgslider->setGeometry(320,38,100,20);
	
	resImgLabel->setGeometry(8,63,310,20);
	resImgspinBox->setGeometry(435,63,65,20);
	resImgslider->setGeometry(320,63,100,20);
	
	okButton->setGeometry(60,100,100,25);
	cancelButton->setGeometry(510-160,100,100,25);
#endif
	
}

void InternetSetDialog::simSliderByImgSlider(int val)
{
	if(val>=simImgslider->value())
		simImgslider->setValue(val+1);
}
void InternetSetDialog::imgSliderBySimSlider(int val)
{
	if(val<=numImgslider->value())
		numImgslider->setValue(val-1);
}
void InternetSetDialog::resSliderBySimSlider(int val)
{
	if(val<=resImgslider->value())
		resImgslider->setValue(val-1);
}
void InternetSetDialog::simSliderByResSlider(int val)
{
	if(val>=simImgslider->value())
		simImgslider->setValue(val+1);
}

