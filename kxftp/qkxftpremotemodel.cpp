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

#include "qkxftpremotemodel.h"
#include "qkxftprequest.h"

#include <QDebug>
#include <QDateTime>

QKxFtpRemoteModel::QKxFtpRemoteModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

void QKxFtpRemoteModel::setFtpRequest(QKxFtpRequest *ftpRequest)
{
    m_ftpRequest = ftpRequest;
}

QKxFtpRemoteModel::FileInfo QKxFtpRemoteModel::fileInfo(const QModelIndex &idx) const
{
    int row = idx.row();
    if(row < 0 || row >= m_fileInfos.length()) {
        return FileInfo();
    }
    return m_fileInfos.at(row);
}

void QKxFtpRemoteModel::setEntryInfoList(const QString &path, const QVariantList &files)
{
    m_path = path;
    beginResetModel();
    m_fileInfos.clear();
    for(int i = 0; i < files.length(); i++) {
        QVariant v = files.at(i);
        FileInfo fi;
        QVariantMap dm = v.toMap();
        fi.name = dm.value("name").toString();
        fi.type = dm.value("type").toInt();
        fi.fsize = dm.value("fsize").toLongLong();
        fi.ct = dm.value("ct").toInt();
        fi.path = path;
        m_fileInfos.append(fi);
    }
    endResetModel();
}

QString QKxFtpRemoteModel::path() const
{
    return m_path;
}

int QKxFtpRemoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_fileInfos.length();
}

QModelIndex QKxFtpRemoteModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractListModel::sibling(row, column, idx);
}

QVariant QKxFtpRemoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant();
    }
    if(section == 0) {
        return tr("FileName");
    }else if(section == 1) {
        return tr("Size");
    }else if(section == 2) {
        return tr("Created Date");
    }
    return QVariant();
}

QVariant QKxFtpRemoteModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row < 0 || row > m_fileInfos.length()) {
        return QVariant();
    }
    FileInfo fi = m_fileInfos.at(row);
    if(role == Qt::DecorationRole) {
        if(col == 0) {
            if(fi.isRoot()) {
                return m_iconProvider.icon(QFileIconProvider::Drive);
            }else if(fi.isDir()) {
                return m_iconProvider.icon(QFileIconProvider::Folder);
            }
            return m_iconProvider.icon(QFileIconProvider::File);
        }
        return QVariant();
    }else if(role == Qt::DisplayRole) {
        if(col == 0) {
            return fi.fileName();
        }else if(col == 1) {
            if(fi.isFile()) {
                return fi.size();
            }
            return QVariant();
        }else if(col == 2) {
            QDateTime dt;
            dt.setSecsSinceEpoch(fi.created());
            return dt;
        }
        return QVariant();
    }else if(role == Qt::SizeHintRole) {
        return QSize(-1, 25);
    }
    return QVariant();
}

bool QKxFtpRemoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QKxFtpRemoteModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
}

bool QKxFtpRemoteModel::insertRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "insertRow";
    return false;
}

bool QKxFtpRemoteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);


    endRemoveRows();

    return true;
}

void QKxFtpRemoteModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QKxFtpRemoteModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Qt::DropActions QKxFtpRemoteModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}
