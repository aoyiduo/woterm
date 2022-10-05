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

#include "qkxftplocalmodel.h"

#include <QDebug>
#include <QDir>
#include <QDateTime>

QKxFtpLocalModel::QKxFtpLocalModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

QFileInfo QKxFtpLocalModel::fileInfo(const QModelIndex &idx) const
{
    int row = idx.row();
    if(row < 0 || row >= m_fileInfos.length()) {
        return QFileInfo();
    }
    return m_fileInfos.at(row);
}

QModelIndex QKxFtpLocalModel::mkdir(const QModelIndex &parent, const QString &name)
{
    return QModelIndex();
}

int QKxFtpLocalModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_fileInfos.length();
}

QModelIndex QKxFtpLocalModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractListModel::sibling(row, column, idx);
}

QVariant QKxFtpLocalModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row < 0 || row > m_fileInfos.length()) {
        return QVariant();
    }
    QFileInfo fi = m_fileInfos.at(row);
    if(role == Qt::DecorationRole) {
        if(col == 0) {
            return m_iconProvider.icon(fi);
        }
        return QVariant();
    }else if(role == Qt::DisplayRole) {
        if(col == 0) {
            if(fi.isRoot()) {
                return fi.filePath();
            }
            return fi.fileName();
        }else if(col == 1) {
            if(fi.isFile()) {
                return fi.size();
            }
            return QVariant();
        }else if(col == 2) {
            return fi.created();
        }
        return QVariant();
    }else if(role == Qt::SizeHintRole) {
        return QSize(-1, 25);
    }
    return QVariant();
}

bool QKxFtpLocalModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QKxFtpLocalModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
}

bool QKxFtpLocalModel::insertRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "insertRow";
    return false;
}

bool QKxFtpLocalModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);


    endRemoveRows();

    return true;
}

void QKxFtpLocalModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QKxFtpLocalModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Qt::DropActions QKxFtpLocalModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

QVariant QKxFtpLocalModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void QKxFtpLocalModel::setPath(const QString &path)
{
    if(path.isEmpty()) {
        setHome();
    }else{
        QDir d(path);
        QFileInfoList fis = d.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::Name);
        onFileListActive(fis);
        m_path = path;
    }
}

QString QKxFtpLocalModel::path() const
{
    return m_path;
}

void QKxFtpLocalModel::reload()
{
    setPath(m_path);
}

void QKxFtpLocalModel::onFileListActive(const QList<QFileInfo> &fis)
{
    beginResetModel();
    m_fileInfos = fis;
    endResetModel();
}

void QKxFtpLocalModel::setHome()
{
    m_path.clear();
#if defined(Q_OS_WIN)
    QFileInfoList fis = QDir::drives();
    onFileListActive(fis);
#elif defined(Q_OS_UNIX)
    QDir d = QDir::home();
    QFileInfoList fis = d.entryInfoList();
    onFileListActive(fis);
#endif
}
