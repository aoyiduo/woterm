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

#ifndef QWOSORTFILTERPROXYMODEL_H
#define QWOSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class QWoSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit QWoSortFilterProxyModel(int maxColumnCount, QObject *parent = nullptr);
public:
    Q_INVOKABLE void search(const QString& key);
    Q_INVOKABLE void qmlSearch(const QString& key);
    Q_INVOKABLE void qmlRemove(const QString& name);
private:
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
private:
    int m_maxColumnCount;
};

#endif // QWOSORTFILTERPROXYMODEL_H
