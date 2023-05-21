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

#include "qmosftpremotemodel.h"

#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>
#include <QIcon>
#include <QFont>
#include <QGuiApplication>

QMoSftpRemoteModel::QMoSftpRemoteModel(QObject *parent)
    : QAbstractListModel (parent)
{
}

QMoSftpRemoteModel::~QMoSftpRemoteModel()
{

}

QString QMoSftpRemoteModel::fileName(int row) const
{
    if(row >= m_fileInfos.length() || row < 0) {
        return "";
    }
    const FileInfo& fi = m_fileInfos.at(row);
    return fi.name;
}

QVariantMap QMoSftpRemoteModel::fileInfo(int row) const
{
    if(row >= m_fileInfos.length() || row < 0) {
        return QVariantMap();
    }
    const FileInfo& fi = m_fileInfos.at(row);
    return toMap(fi);
}

QVariantList QMoSftpRemoteModel::fileInfosFromSelection() const
{
    QVariantList lsv;
    for(auto it = m_fileInfos.begin(); it != m_fileInfos.end(); it++) {
        const FileInfo& fi = *it;
        if(fi.name == "." || fi.name == ".." || !fi.selected) {
            continue;
        }
        lsv.append(toMap(fi));
    }
    return lsv;
}

void QMoSftpRemoteModel::selectAll()
{
    for(auto it = m_fileInfos.begin(); it != m_fileInfos.end(); it++) {
        FileInfo& fi = *it;
        if(fi.name == "." || fi.name == "..") {
            continue;
        }
        fi.selected = true;
    }
    QModelIndex idx = index(0, 0);
    QModelIndex idx2 = index(m_fileInfos.length()-1, 0);
    emit dataChanged(idx, idx2);
}

void QMoSftpRemoteModel::clearSelection()
{
    for(auto it = m_fileInfos.begin(); it != m_fileInfos.end(); it++) {
        FileInfo& fi = *it;
        if(fi.name == "." || fi.name == "..") {
            continue;
        }
        fi.selected = false;
    }
    QModelIndex idx = index(0, 0);
    QModelIndex idx2 = index(m_fileInfos.length()-1, 0);
    emit dataChanged(idx, idx2);
}

QString QMoSftpRemoteModel::path() const
{
    return m_path;
}

bool QMoSftpRemoteModel::exist(const QString &fileName)
{
    auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [=](const FileInfo& fi) {
        return fi.name == fileName;
    });
    return it != m_fileInfos.end();
}

int QMoSftpRemoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_fileInfos.count();
}

QVariant QMoSftpRemoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QHash<int, QByteArray> QMoSftpRemoteModel::roleNames() const
{
    QHash<int,QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(Qt::DisplayRole, "name");
    roles.insert(ROLE_ICON_URL, "iconUrl");
    roles.insert(ROLE_SHORT_STRING, "shortString");
    roles.insert(ROLE_GROUP_NAME, "groupName");
    roles.insert(ROLE_FILEINFO, "fileInfo");
    roles.insert(ROLE_SELECTED, "selected");
    roles.insert(ROLE_TYPE, "type");
    return roles;
}

QVariantMap QMoSftpRemoteModel::toMap(const FileInfo &fi) const
{
    QVariantMap dm;
    dm.insert("longName", fi.longName);
    dm.insert("label", fi.label);
    dm.insert("name", fi.name);
    dm.insert("type", fi.type);
    dm.insert("owner", fi.owner);
    dm.insert("group", fi.group);
    dm.insert("size", fi.size);
    dm.insert("date", fi.date);
    dm.insert("permission", fi.permission);
    return dm;
}

QVariant QMoSftpRemoteModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_fileInfos.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    const FileInfo& fi = m_fileInfos.at(index.row());

    if(role == Qt::DisplayRole) {
        return fi.name;
    }
    if(role == ROLE_INDEX) {
        return index.row();
    }

    if(role == ROLE_ICON_URL) {
        if(fi.isDir()) {
            return "qrc../private/skins/black/folder2.png";
        }else if(fi.isLink()) {
            return "qrc../private/skins/black/link.png";
        }
        return "qrc../private/skins/black/file.png";
    }
    if(role == ROLE_SHORT_STRING) {
        return QString("%1 %2 %3:%4 %5").arg(fi.permission).arg(fi.size).arg(fi.owner).arg(fi.group).arg(fi.date);
    }
    if(role == ROLE_SELECTED) {
        return fi.selected;
    }
    if(role == ROLE_TYPE) {
        return fi.type;
    }
    return QVariant();
}

bool QMoSftpRemoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_fileInfos.size()) {
        return false;
    }
    if (!index.isValid()){
        return false;
    }
    FileInfo& fi = m_fileInfos[index.row()];
    if(role == ROLE_SELECTED) {
        fi.selected = value.toBool();
        return true;
    }
    return false;
}

int QMoSftpRemoteModel::columnCount(const QModelIndex &parent) const
{
    return 6;
}

static bool lessThan(const FileInfo &s1, const FileInfo &s2)
{
    static QString type = "dl-";
    int t1 = type.indexOf(s1.type);
    int t2 = type.indexOf(s2.type);
    if(t1 < t2) {
        return true;
    }
    if(t1 > t2) {
        return false;
    }
    return s1.name < s2.name;
}

void QMoSftpRemoteModel::onDirOpen(const QString &path, const QVariantList &v, const QVariantMap& userData)
{
    beginResetModel();
    m_path = path;
    m_fileInfos.clear();
    for(int i = 0; i < v.length(); i++) {
        QMap<QString, QVariant> mdata = v.at(i).toMap();
        FileInfo fi;
        fi.longName = mdata.value("longName").toString();
        fi.label = mdata.value("label").toString();
        fi.name = mdata.value("name").toString();
        fi.type = mdata.value("type").toString();
        fi.owner = mdata.value("owner").toString();
        fi.group = mdata.value("group").toString();
        fi.size = mdata.value("size").toLongLong();
        fi.date = mdata.value("date").toString();
        fi.permission = mdata.value("permission").toString();
        m_fileInfos.append(fi);
    }
    std::sort(m_fileInfos.begin(), m_fileInfos.end(), lessThan);
    endResetModel();

    emit pathChanged(m_path);
}
