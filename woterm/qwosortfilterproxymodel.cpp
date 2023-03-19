/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwosortfilterproxymodel.h"
#include "qwohostlistmodel.h"

QWoSortFilterProxyModel::QWoSortFilterProxyModel(int maxColumn, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_maxColumnCount(maxColumn)
{

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
