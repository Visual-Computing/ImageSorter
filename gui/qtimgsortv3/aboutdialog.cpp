#include "aboutdialog.h"

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
	setWindowTitle(tr("About ImageSorter"));
	setWindowModality(Qt::WindowModal);
	setFixedWidth(572);
	setFixedHeight(380);

	createDialogArea();
}


AboutDialog::~AboutDialog() {

}

void AboutDialog::createDialogArea() {
	// create img label
	//QPixmap pixmap(":/about");
	QPixmap pixmap(":/about3");
	QLabel *imgLabel = new QLabel(this);	
	imgLabel->setPixmap(pixmap);

	// create close button
	QPushButton *closeButton = new QPushButton(this);
	closeButton->setText(tr("Close"));

	// create Visual Computing website label 
	QLabel *websiteLink = new QLabel(this);
	websiteLink->setText("<a href=\"www.visual-computing.com\">www.visual-computing.com</a>");
	websiteLink->setOpenExternalLinks(true);

	// connect button events to slots
	connect(closeButton, SIGNAL(clicked()),this, SLOT(close()));

	// set widget positions
	imgLabel->setGeometry(0, 0, 616, 345);
	closeButton->setGeometry(488, 350, 80, 25);
	websiteLink->setGeometry(20, 350, 120, 25);
}
