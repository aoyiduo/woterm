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

#ifndef QKXFTPREMOTEMODEL_H
#define QKXFTPREMOTEMODEL_H

#include "qkxftp_share.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QFileIconProvider>
#include <QDir>

class QKxFtpRequest;
class KXFTP_EXPORT QKxFtpRemoteModel : public QAbstractListModel
{
    Q_OBJECT
public:
    struct FileInfo
    {
        QString path;
        QString name;
        qint8 type;
        qint64 fsize;
        qint32 ct;

        FileInfo() {
            type = fsize = ct = 0;
        }

        bool isDir() const {
            return type > 1;
        }

        bool isRoot() const {
            return type > 2;
        }

        QString fileName() const {
            return name;
        }

        qint64 size() const {
            return fsize;
        }

        bool isFile() const {
            return type == 1;
        }

        qint32 created() const {
            return ct;
        }

        QString absoluteFilePath() const {
            if(path.isEmpty()) {
                return name;
            }
            return QDir::cleanPath(path + "/" + name);
        }
        QString absolutePath() const {
            if(path.isEmpty()) {
                return path;
            }
            return path;
        }
    };
public:
    explicit QKxFtpRemoteModel(QObject *parent = nullptr);
    void setFtpRequest(QKxFtpRequest *ftpRequest);
    FileInfo fileInfo(const QModelIndex& idx) const;
    void setEntryInfoList(const QString& path, const QVariantList& files);
    QString path() const;
protected:
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
    QPointer<QKxFtpRequest> m_ftpRequest;
    QString m_path;
    QList<FileInfo> m_fileInfos;
    QFileIconProvider m_iconProvider;
};

#endif // QKXFTPREMOTEMODEL_H
