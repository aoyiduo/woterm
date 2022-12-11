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

#include "qwosftplocalmodel.h"
#include "qkxutils.h"

#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QGuiApplication>
#include <QFontMetrics>

QWoSftpLocalModel::QWoSftpLocalModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_font = QGuiApplication::font();
    m_font.setFamily(QKxUtils::suggestFamily());
}

QFileInfo QWoSftpLocalModel::fileInfo(const QModelIndex &idx) const
{
    int row = idx.row();
    if(row < 0 || row >= m_fileInfos.length()) {
        return QFileInfo();
    }
    return m_fileInfos.at(row);
}

QModelIndex QWoSftpLocalModel::mkdir(const QModelIndex &parent, const QString &name)
{
    return QModelIndex();
}

int QWoSftpLocalModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_fileInfos.length();
}

QModelIndex QWoSftpLocalModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractListModel::sibling(row, column, idx);
}

QVariant QWoSftpLocalModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row < 0 || row > m_fileInfos.length()) {
        return QVariant();
    }
    if(role == Qt::FontRole) {
        return m_font;
    }
    QFileInfo fi = m_fileInfos.at(row);
    if(role == ROLE_INDEX) {
        return index.row();
    }
    if(role == ROLE_FILEINFO) {
        QVariant v;
        v.setValue(fi);
        return v;
    }
    if(role == Qt::DecorationRole) {
        if(col == 0) {
            return m_iconProvider.icon(fi);
        }
        return QVariant();
    }else if(role == Qt::SizeHintRole) {
        QFontMetrics fm(m_font);
        if(col == 0) {
            QString name = fi.fileName();
            if(fi.isRoot()) {
                name = fi.filePath();
            }
            int w = fm.width(name) + 50;
            return QSize(w, 24);
        }
        if(col == 1) {
            int w = fm.width(QString::number(fi.size())) + 10;
            return QSize(w, 24);
        }
        if(col == 2) {
            int w = fm.width(fi.created().toString()) + 10;
            return QSize(w, 24);
        }
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

bool QWoSftpLocalModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoSftpLocalModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool QWoSftpLocalModel::insertRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "insertRow";
    return false;
}

bool QWoSftpLocalModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);


    endRemoveRows();

    return true;
}

void QWoSftpLocalModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QWoSftpLocalModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Qt::DropActions QWoSftpLocalModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

QVariant QWoSftpLocalModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void QWoSftpLocalModel::setPath(const QString &path)
{
    if(path.isEmpty()) {
        setHome();
    }else{
        QDir d(path);
        QFileInfoList fis = d.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::Name);
        onFileListActive(fis);
        m_path = path;
    }
    emit pathChanged(m_path);
}

QString QWoSftpLocalModel::path() const
{
    return m_path;
}

void QWoSftpLocalModel::reload()
{
    setPath(m_path);
}

void QWoSftpLocalModel::onFileListActive(const QList<QFileInfo> &fis)
{
    beginResetModel();
    m_fileInfos = fis;
    endResetModel();
}

void QWoSftpLocalModel::setHome()
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
    emit pathChanged(m_path);
}
