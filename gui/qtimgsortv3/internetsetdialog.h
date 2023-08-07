#ifndef INTERNETSETDIALOG_H
#define INTERNETSETDIALOG_H

#include <QtGui>
#include <QDialog>

class InternetSetDialog : public QDialog
{
	Q_OBJECT

	public:
		InternetSetDialog(QWidget *parent);
		~InternetSetDialog(void);
		
		void setNumQueries(unsigned n) {numImgslider->setValue(n);}
		void setNumSimilarQueries(unsigned n) {simImgslider->setValue(n);}
		void setNumSimilarShown(unsigned n) {resImgslider->setValue(n);}
		void setNumNetSearchThreads(unsigned n) {numThreadsSlider->setValue(n);}

		unsigned numQueries() const {return (unsigned)numImgslider->value();}
		unsigned numSimilarQueries() const {return (unsigned)simImgslider->value();}
		unsigned numSimilarShown() const {return (unsigned)resImgslider->value();}
		unsigned numnetSearchThreads() const {return (unsigned)numThreadsSlider->value();}

	private:
		// other methods
		void createDialogArea();
		QLabel *numberOfImagesLabel;
		QLabel *simImgLabel;
		QLabel *resImgLabel;
		QLabel *numThreadsLabel;

		QSpinBox *numImgspinBox ;
		QSlider *numImgslider;
		QSpinBox *simImgspinBox ;
		QSlider *simImgslider;
		QSpinBox *resImgspinBox ;
		QSlider *resImgslider;
		QSpinBox *numThreadsSpinBox ;
		QSlider *numThreadsSlider;

		QPushButton *okButton;
		QPushButton *cancelButton;
		QPushButton *invisibleDefaultButton;

	private slots:
		void simSliderByImgSlider(int);
		void imgSliderBySimSlider(int);
		void resSliderBySimSlider(int);
		void simSliderByResSlider(int);
};

#endif

