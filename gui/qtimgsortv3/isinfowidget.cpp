
#include "isinfowidget.h"

#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
ISInfoWidget::ISInfoWidget( QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	create();
}


ISInfoWidget::ISInfoWidget( const QString & title, QFrame * parent, Qt::WindowFlags f )
: QFrame(parent, f) {
	create();
}



void ISInfoWidget::create() 
{
	// Erstellt die GUI
	QFormLayout *formLayout = new QFormLayout;
	nameLineEdit = new QLabel();
	sizeLineEdit = new QLabel();
	orientationLineEdit = new QLabel();
	// lastModLineEdit = new QLabel();
	formLayout->addRow(tr("File name:"), nameLineEdit);
	formLayout->addRow(tr("File size:"), sizeLineEdit);
	formLayout->addRow(tr("Image size:"), orientationLineEdit);

	setLayout(formLayout);
}


ISInfoWidget::~ISInfoWidget() {
}


/*! Slot-Methode, setzt die Labelfelder für die Bildinformation
 */
void ISInfoWidget::showImageInfo(const imageInfo&info)
{
	QString s1 = info.m_PathOrURL;
	QString splitter="/";
	int tmp = 0;
	QString s2 = "";
	QString result;
	

	if(info.m_bIsValid)
	{
		// Wenn das Bild aus dem Internet geladen wurde,
		// wird nur der Bildname angezeigt
		if(info.m_bIsFromNet){			
			tmp = s1.lastIndexOf(splitter)+1;
			s2 = s1.right(s1.size()-tmp);
			
			int size = s2.size();
			int anzZeichen = 25;
			int anzahl = size/anzZeichen;
			int rest = size%anzZeichen;
			
			if(size>10)
			{
				for(int i = 0; i<size;i++)
				{
					if(i%anzZeichen==0 && i!=0)
						result.append("<br></br>");
						
					result.append(s2.at(i));
				}
			nameLineEdit->setText(result);
			}
			else
			{
				nameLineEdit->setText(s2);
			}
			sizeLineEdit->setText(tr("unknown"));
		}
		else
		{
			nameLineEdit->setText(info.m_FileName);
			//nameLineEdit->setToolTip(info.m_PathOrURL);
		
			sizeLineEdit->setText(info.m_FileSize);
		}
		nameLineEdit->setToolTip(info.m_PathOrURL);
		
		if(info.m_XSize==0)
		{
			orientationLineEdit->setText(tr("unknown"));
		}
		else
		{
			orientationLineEdit->setText(QString::number(info.m_XSize)+ " x "+(QString::number(info.m_YSize))+ " Pixel ");
		}
	}
	else
	{
		nameLineEdit->setText("");
		sizeLineEdit->setText("");
		orientationLineEdit->setText("");
	}
}

