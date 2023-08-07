#ifndef SORTOPTIONSDIALOG_H
#define SORTOPTIONSDIALOG_H

#include <QtGui>
#include <QDialog>

class sortOptionsDialog : public QDialog
{
	Q_OBJECT

	public:
		sortOptionsDialog(QWidget *parent);
		~sortOptionsDialog(void);
		
		void setDimFactor(double f);

		double dimFactor() const; 

	private:
		// other methods
		void createDialogArea();
		QLabel *dimFactorLabel;

		QSpinBox *dimFactorSpinBox;
		QSlider *dimFactorSlider;

		QPushButton *okButton;
		QPushButton *cancelButton;
		QPushButton *invisibleDefaultButton;

};

#endif

