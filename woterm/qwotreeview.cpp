/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwotreeview.h"
#include <QMouseEvent>
#include <QtDebug>

QWoTreeView::QWoTreeView(QWidget *parent)
    :QTreeView (parent)
{

}

void QWoTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    bool open = options.state & QStyle::State_Open;
    QVariant v = model()->data(index, ROLE_GROUP);
    if(v.isValid()) {
        model()->setData(index, open, ROLE_GROUP_STATE);
    }
    QTreeView::drawRow(painter, options, index);
}

void QWoTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    //QTreeView::drawBranches(painter, rect, index);
}

void QWoTreeView::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        clearSelection();
        QTreeView::keyPressEvent(e);
        QItemSelectionModel *model = selectionModel();
        QModelIndex idx = currentIndex();
        if(idx.isValid()){            
            model->select(idx, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        }
    }else if(e->key() == Qt::Key_Return) {
        emit returnKeyPressed();
    }else{
        QTreeView::keyPressEvent(e);
    }
}

void QWoTreeView::mousePressEvent(QMouseEvent *e)
{
    QTreeView::mousePressEvent(e);
    QItemSelectionModel *model = selectionModel();
    if(e->button() == Qt::LeftButton){
        QModelIndex idx = indexAt(e->pos());
        if(e->modifiers().testFlag(Qt::ControlModifier)) {
            if(idx.isValid()) {
                model->select(idx, QItemSelectionModel::Toggle|QItemSelectionModel::Rows);
                // the follow line must add.
                QTreeView::mouseMoveEvent(e);
            }
        }else{
            model->clearSelection();
            if(idx.isValid()){
                model->select(idx, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            }
        }
    }else{
        QModelIndex idx = indexAt(e->pos());
        if(!idx.isValid()) {
            model->clearSelection();
            return;
        }
        QModelIndexList idxs = selectedIndexes();
        for(int i  = 0; i < idxs.length(); i++) {
            if(idx == idxs.at(i)) {
                return;
            }
        }

        model->clearSelection();
        model->select(idx, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    }
}

void QWoTreeView::mouseReleaseEvent(QMouseEvent *e)
{
    QTreeView::mouseReleaseEvent(e);
}

void QWoTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    emit itemChanged(current);
}
