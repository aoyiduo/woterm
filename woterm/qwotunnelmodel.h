/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QWOTUNNELMODEL_H
#define QWOTUNNELMODEL_H

#include "qwoglobal.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QList>
#include <QFont>
#include <QIcon>

class QWoTunnelModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoTunnelModel(QObject *parent = nullptr);
    virtual ~QWoTunnelModel();

    bool add(const TunnelInfo& ti);
    void modify(const TunnelInfo& ti);
    void remove(qint64 id);
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::DropActions supportedDropActions() const override;
private:
    Q_INVOKABLE void load();
    void save();
private:
    Q_DISABLE_COPY(QWoTunnelModel)
    QList<TunnelInfo> m_tunnelInfos;
    QFont m_font;
    QIcon m_icoRunning;
};

#endif // QWOTUNNELMODEL_H
