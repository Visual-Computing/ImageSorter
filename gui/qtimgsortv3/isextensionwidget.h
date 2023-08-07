#ifndef ISLayer_H
#define ISLayer_H

#include <QtGui>
#include "imagesortworker.h"
/*
class ISProgressBar : public QFrame {
	Q_OBJECT

public:
	ISProgressBar( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISProgressBar() {}

public:
	void setText(const QString& text) { m_Label->setText(text); }
	void setMinimum(int minimum) { m_progressBar->setMinimum(minimum); }
	void setMaximum(int maximum) { m_progressBar->setMaximum(maximum); }
	void incValue() { m_progressBar->setValue(m_progressBar->value()+1); }

	int minimum() { return m_progressBar->minimum(); }
	int maximum() { return m_progressBar->maximum(); }
	int value() { return m_progressBar->value(); }

	void reset();

public slots:
	void setValue(int value);

signals: 
	void clickedCancel();

private:
	QProgressBar *m_progressBar;
	QLabel *m_Label;
};
*/


class ISMsgWidget : public QFrame {
	Q_OBJECT

public:
	ISMsgWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~ISMsgWidget();

public:
	void setText(const QString& text);
	QPushButton* addButton(const QString& text);

	void reset();
	
protected:

private:
	QList<QPushButton*> m_pushButtons;
	QLabel *m_Label;
	QVBoxLayout *vLayout;
	QHBoxLayout *hLayout1;
	QHBoxLayout *hLayout2;
	
signals: 
	void clicked();
};


#endif
