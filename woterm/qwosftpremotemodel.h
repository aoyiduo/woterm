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

class QQuickItem;

class QWoSftpRemoteModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoSftpRemoteModel(QObject *parent = nullptr);
    virtual ~QWoSftpRemoteModel();
public:
    QString path() const;
    bool exist(const QString& fileName);
signals:
    void pathChanged(const QString& path);
private slots:
    void onDirOpen(const QString& path, const QList<QVariant>& data, const QVariantMap& userData);
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
    Q_DISABLE_COPY(QWoSftpRemoteModel)
    QList<FileInfo> m_fileInfos;
    QString m_path;
    QIcon m_dirIcon;
    QIcon m_fileIcon;
    QIcon m_linkIcon;
    QFont m_font;
};
