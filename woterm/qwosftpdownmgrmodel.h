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
#include <QPointer>

#define ROLE_INDEX   (Qt::UserRole+1)
#define ROLE_FILEINFO (Qt::UserRole+2)
#define ROLE_REFILTER (Qt::UserRole+3)
#define ROLE_FRIENDLY_NAME (Qt::UserRole+4)

class QWoSftpDownMgrModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoSftpDownMgrModel(QObject *parent = nullptr);
    virtual ~QWoSftpDownMgrModel();
private slots:
    void onDirOpen(const QString& path, const QList<QVariant>& data);
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
    Q_DISABLE_COPY(QWoSftpDownMgrModel)
    QList<FileInfo> m_fileInfos;
    QString m_path;
};
