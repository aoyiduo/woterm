﻿/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#pragma once

#include "qwoglobal.h"

#include <QAbstractListModel>
#include <QFont>
#include <QIcon>


class QWoHostListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoHostListModel(QObject *parent = nullptr);
    virtual ~QWoHostListModel();

    static QWoHostListModel *instance();

    int widthColumn(const QFont& ft, int i);

    Q_INVOKABLE void refreshList();
    bool exists(const QString &name);
    void resetAllProperty(QString v);
    void modifyOrAppend(const HostInfo& hi);

    Q_INVOKABLE bool qmlRemove(const QString& name);
    Q_INVOKABLE QVariantMap qmlGet(int row);

public:
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

private slots:
    void onDataReset();
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

    QHash<int, QByteArray> roleNames() const override;

private:
    Q_DISABLE_COPY(QWoHostListModel)
    QList<HostInfo> m_hosts;
    QIcon m_sshIcon;
    QIcon m_sftpIcon;
    QIcon m_telnetIcon;
    QIcon m_rloginIcon;
    QIcon m_mstscIcon;
    QIcon m_vncIcon;
    QIcon m_serialIcon;
};
