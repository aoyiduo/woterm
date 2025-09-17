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

#ifndef QWOSORTFILTERPROXYMODEL_H
#define QWOSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class QWoSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit QWoSortFilterProxyModel(int maxColumnCount, QObject *parent = nullptr);
    bool treeWalk(const std::function<bool(const QModelIndex& idx)>& fn) const;
public:
    Q_INVOKABLE void search(const QString& key);
    Q_INVOKABLE void qmlSearch(const QString& key);
    Q_INVOKABLE void qmlRemove(const QString& name);
private:
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool treeWalk(const QModelIndex& idx, const std::function<bool(const QModelIndex& idx)>& fn) const;
private:
    int m_maxColumnCount;
};

#endif // QWOSORTFILTERPROXYMODEL_H
