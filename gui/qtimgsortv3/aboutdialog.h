#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui>
#include <QDialog>



/**
 *
 *
 */
class AboutDialog : public QDialog {
	Q_OBJECT
	public:
		AboutDialog(QWidget *parent);
		~AboutDialog();
	private:
		// other methods
		void createDialogArea();
    
};


#endif
