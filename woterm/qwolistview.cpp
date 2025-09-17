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

#include "qwolistview.h"
#include <QMouseEvent>
#include <QtDebug>

QWoListView::QWoListView(QWidget *parent)
    :QListView (parent)
{

}

void QWoListView::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        clearSelection();
        QListView::keyPressEvent(e);
        QItemSelectionModel *model = selectionModel();
        QModelIndex idx = currentIndex();
        if(idx.isValid()){
            model->select(currentIndex(), QItemSelectionModel::Select);
        }
    }else if(e->key() == Qt::Key_Return) {
        emit returnKeyPressed();
    }
}

void QWoListView::mousePressEvent(QMouseEvent *e)
{
    QListView::mousePressEvent(e);
    QItemSelectionModel *model = selectionModel();
    if(e->button() == Qt::LeftButton){
        clearSelection();
        QModelIndex idx = indexAt(e->pos());
        if(idx.isValid()){
            setCurrentIndex(idx);
            model->select(idx, QItemSelectionModel::Select);
        }else{
            setCurrentIndex(QModelIndex());
        }
    }else{
        QModelIndex idx = indexAt(e->pos());
        if(!idx.isValid()) {
            clearSelection();
            setCurrentIndex(QModelIndex());
            return;
        }
        QModelIndexList idxs = selectedIndexes();
        for(int i  = 0; i < idxs.length(); i++) {
            if(idx == idxs.at(i)) {
                return;
            }
        }
        clearSelection();
        setCurrentIndex(idx);
        model->select(idx, QItemSelectionModel::Select);
    }
}

void QWoListView::mouseReleaseEvent(QMouseEvent *e)
{
    QListView::mouseReleaseEvent(e);
}

void QWoListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QListView::currentChanged(current, previous);
    emit itemChanged(current);
}
