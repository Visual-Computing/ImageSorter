#include "istreeview.h"
#include <assert.h>

ISTreeView::ISTreeView(QWidget *parent): m_bFileClicked(false),QTreeView(parent)
{
	setContextMenuPolicy(Qt::PreventContextMenu);
}

void ISTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) 
{
	QTreeView::selectionChanged(selected, deselected);
	emit selectionChangedSignal(selected, deselected);
}

QItemSelectionModel::SelectionFlags ISTreeView::selectionCommand(const QModelIndex& index,const QEvent *event) const
{
	// note this is essential, otherwise we get a crash if the user clicks
	// 'white space'
	if(!index.isValid())
		return QAbstractItemView::selectionCommand(index,event);
	
	// V4.2: we have a QFilesSystemModel instaed of a QDirModel now..
	//QDirModel* pModel=static_cast<QDirModel*>(model());
	QFileSystemModel* pModel=static_cast<QFileSystemModel*>(model());
	assert(pModel);
#pragma message("check if QDirModel::isDir() works in the unpatched qtcore.dll")
	//if(pModel->isDir(index))
	//	return QAbstractItemView::selectionCommand(index,event);
	//else
	//	return QItemSelectionModel::NoUpdate;
	QFileInfo info=pModel->fileInfo(index);
	if(info.isDir()){
		emit fileClicked(QFileInfo());
		m_bFileClicked=false;
		return QAbstractItemView::selectionCommand(index,event);
	}
	else{
		if(info.isSymLink()){
			QString target=info.symLinkTarget();
			QFileInfo targetInfo(target);
			emit fileClicked(targetInfo);
		}
		else
		emit fileClicked(info);
		m_bFileClicked=true;
		return QItemSelectionModel::NoUpdate;
	}
}


void ISTreeView::leaveEvent(QEvent *event)
{
	if(m_bFileClicked)
		emit fileClicked(QFileInfo());

	m_bFileClicked=false;
}

void ISTreeView::enterEvent(QEvent *event)
{
	m_bFileClicked=false;
}

void ISTreeView::indexExpanded(const QModelIndex & index)
{
	resizeColumnToContents(0);
}

void ISTreeView::indexCollapsed(const QModelIndex & index)
{
	resizeColumnToContents(0);
}