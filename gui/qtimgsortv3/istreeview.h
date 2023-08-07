#ifndef ISTREEVIEW_H
#define ISTREEVIEW_H

#include <QObject>
#include <QTreeView>
#include <QtGui>

class ISTreeView : public QTreeView {

	Q_OBJECT


	public:
		ISTreeView(QWidget *parent);
		~ISTreeView(){;}

	signals:
		void selectionChangedSignal(const QItemSelection &selected, const QItemSelection &deselected);
		void fileClicked(const QFileInfo&) const;

	protected slots:
		void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
		// V4.2: recalculate column 0 to show/hide horizontal scrollbar if anything gets expanded/collapsed
		void indexExpanded(const QModelIndex & index);
		void indexCollapsed(const QModelIndex & index);
	protected:
		/*!
		overridden virtual QAbstractItemView member

		we return QItemSelectionModel::NoUpdate for all indexes which are not directories
		*/
		virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index,const QEvent *event=0) const;

		virtual void leaveEvent(QEvent *event); 
		virtual void enterEvent(QEvent *event); 

		// set in const function, thus mutable...
		mutable bool m_bFileClicked;

   
};


#endif // ISTREEVIEW_H
