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

#include "qwosortfilterproxymodel.h"
#include "qwohostlistmodel.h"

QWoSortFilterProxyModel::QWoSortFilterProxyModel(int maxColumn, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_maxColumnCount(maxColumn)
{

}

bool QWoSortFilterProxyModel::treeWalk(const std::function<bool (const QModelIndex &)> &fn) const
{
    QModelIndex idx = index(0, 0);
    do {
        if(!treeWalk(idx, fn)) {
            return false;
        }
        idx = idx.siblingAtRow(idx.row()+1);
    }while(idx.isValid());
    return true;
}


void QWoSortFilterProxyModel::search(const QString &key)
{
    QStringList sets = key.split(' ');
    for(QStringList::iterator iter = sets.begin(); iter != sets.end(); ) {
        if(*iter == "") {
            iter = sets.erase(iter);
        }else{
            iter++;
        }
    }

    QRegExp regex(sets.join(".*"), Qt::CaseInsensitive);
    regex.setPatternSyntax(QRegExp::RegExp2);
    setFilterRegExp(regex);
}

void QWoSortFilterProxyModel::qmlSearch(const QString &key) {
    search(key);
}

void QWoSortFilterProxyModel::qmlRemove(const QString &name)
{
    QWoHostListModel *model = qobject_cast<QWoHostListModel*>(sourceModel());
    if(model) {
        model->qmlRemove(name);
    }
}

int QWoSortFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    int cnt = QSortFilterProxyModel::columnCount(parent);
    if(cnt > m_maxColumnCount) {
        return m_maxColumnCount;
    }
    return cnt;
}

bool QWoSortFilterProxyModel::treeWalk(const QModelIndex &idx, const std::function<bool (const QModelIndex &)> &fn) const
{
    if(!idx.isValid()) {
        return true;
    }
    if(!fn(idx)) {
        return false;
    }
    int count = rowCount(idx);
    for(int i = 0; i < count; i++) {
        if(!treeWalk(idx.child(i, 0), fn)) {
            return false;
        }
    }
    return true;
}
