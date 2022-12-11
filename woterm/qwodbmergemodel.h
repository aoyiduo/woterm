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

#ifndef QWODBMERGEMODEL_H
#define QWODBMERGEMODEL_H

#include "qwoglobal.h"

#include <QAbstractTableModel>
#include <QList>
#include <QFont>

class QWoDBMergeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QWoDBMergeModel(QObject *parent = nullptr);
    void setFont(const QFont& ft);
    void setData(const MergeInfo& mi);
    QString reset(bool isFull);
    void clear();
    void apply();
    bool runAction(int action, const QModelIndex& idx);
protected:
    virtual QString toString(const QVariantMap& data) const = 0;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
protected:
    MergeInfo m_mi;
    QList<QVariantMap> m_datas;
    QFont m_font;
    int m_maxWidth;
    bool m_isFull;
};

#endif // QWODBMERGEMODEL_H
