#include "sortoptionsdialog.h"

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
sortOptionsDialog::sortOptionsDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("Sort Options"));
	setWindowModality(Qt::WindowModal);
	setFixedWidth(360);
	setFixedHeight(80);
	createDialogArea();

}

sortOptionsDialog::~sortOptionsDialog()
{
}

void sortOptionsDialog::createDialogArea()
{
	dimFactorLabel = new QLabel(this);
	dimFactorLabel->setText(tr("Image Spacing: "));

	okButton = new QPushButton(this);
	okButton->setText("Ok");

	cancelButton = new QPushButton(this);
	cancelButton->setText(tr("Cancel"));

	invisibleDefaultButton=new QPushButton(this);
	invisibleDefaultButton->setVisible(false);
	invisibleDefaultButton->setDefault(true);
	
	dimFactorSpinBox = new QSpinBox(this);
	dimFactorSpinBox->setKeyboardTracking(false);
	dimFactorSlider = new QSlider(Qt::Horizontal,this);
	dimFactorSpinBox->setRange(0,10);
	dimFactorSlider->setRange(0,10);

	connect(cancelButton, SIGNAL(clicked()),this, SLOT(reject()));
	connect(okButton, SIGNAL(clicked()),this, SLOT(accept()));

	connect(dimFactorSpinBox, SIGNAL(valueChanged(int)),dimFactorSlider, SLOT(setValue(int)));
    connect(dimFactorSlider, SIGNAL(valueChanged(int)),dimFactorSpinBox, SLOT(setValue(int)));

	dimFactorLabel->setGeometry(3,3,190,20);
	dimFactorSpinBox->setGeometry(300,3,45,20);
	dimFactorSlider->setGeometry(195,3,100,20);
	
	okButton->setGeometry(60,40,100,25);
	cancelButton->setGeometry(360-160,40,100,25);
}

 void sortOptionsDialog::setDimFactor(double f)
{
	// the dim factor is from [1.0,2.0]
	// subtract one and multiplicate with10
	f-=1.0;
	f*=10.0;
	int i=(int)(f+.5);
	dimFactorSlider->setValue(i);
}
double sortOptionsDialog::dimFactor() const
{
	double f=(double)dimFactorSlider->value();
	f/=10.;
	f+=1.;
	f=qMax(1.0,f);
	f=qMin(2.0,f);
	return f;
}