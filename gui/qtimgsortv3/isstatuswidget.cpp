
#include "isstatuswidget.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
ISStatusWidget::ISStatusWidget( QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	create();
}


ISStatusWidget::ISStatusWidget( const QString & title, QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	create();
}

void ISStatusWidget::create() 
{
	// Erstellt die GUI
	QFormLayout *formLayout = new QFormLayout;
	numDirs = new QLabel("-");
	numFiles = new QLabel("-");
	numSelFiles = new QLabel("-");

	formLayout->addRow(tr("Folders:"), numDirs);
	
	formLayout->addRow(tr("Images:"), numFiles);
	formLayout->addRow(tr("Selected images: "), numSelFiles);

	setLayout(formLayout);
}

ISStatusWidget::~ISStatusWidget() {
}

/*! Setzt die Labelfelder für die Anzahl der Ordner und Dateien
 */ 
void ISStatusWidget::setStatusWidget(QString strNumDirs, QString strNumFiles) {

	numDirs->setText(strNumDirs);
	numFiles->setText(strNumFiles);
}


/*! Setzt das Labelfeld für die Anzahl versteckter Dateien
 */ 
void ISStatusWidget::setNumSelected(QString str){

			numSelFiles->setText(str);
}