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

#pragma once

#include "qwoglobal.h"

#include <QAbstractListModel>
#include <QFont>
#include <QIcon>

#define ROLE_INDEX   (Qt::UserRole+1)
#define ROLE_HOSTINFO (Qt::UserRole+2)
#define ROLE_REFILTER (Qt::UserRole+3)
#define ROLE_FRIENDLY_NAME (Qt::UserRole+4)

class QWoHostListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoHostListModel(QObject *parent = nullptr);
    virtual ~QWoHostListModel();

    static QWoHostListModel *instance();

    void setMaxColumnCount(int cnt);
    int widthColumn(const QFont& ft, int i);

    void refreshList();
    bool exists(const QString &name);
    void resetAllProperty(QString v);
    void modifyOrAppend(const HostInfo& hi);
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

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
    Q_DISABLE_COPY(QWoHostListModel)
    QList<HostInfo> m_hosts;
    int m_maxColumn;
    QIcon m_sshIcon;
    QIcon m_sftpIcon;
    QIcon m_telnetIcon;
    QIcon m_rloginIcon;
    QIcon m_mstscIcon;
    QIcon m_vncIcon;
    QIcon m_serialIcon;
};
