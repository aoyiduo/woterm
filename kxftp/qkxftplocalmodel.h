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

#ifndef QKXFTPLOCALMODEL_H
#define QKXFTPLOCALMODEL_H

#include "qkxftp_share.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QFileInfo>
#include <QList>
#include <QFileIconProvider>

class KXFTP_EXPORT QKxFtpLocalModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QKxFtpLocalModel(QObject *parent = nullptr);
    QFileInfo fileInfo(const QModelIndex& idx) const;
    QModelIndex mkdir(const QModelIndex &parent, const QString &name);
    void setPath(const QString& path);
    QString path() const;
    void reload();

    void setHome();
protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    Qt::DropActions supportedDropActions() const override;
    // header
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private slots:
    void onFileListActive(const QList<QFileInfo>& fis);
private:
    QString m_path;
    QList<QFileInfo> m_fileInfos;
    QFileIconProvider m_iconProvider;
};

#endif // QKXFTPLOCALMODEL_H
