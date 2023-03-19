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

#include "qmosftpqueuemodel.h"

#include <QDateTime>
#include <QDebug>

QMoSftpQueueModel::QMoSftpQueueModel(QObject *parent)
    : QAbstractListModel(parent)
{
    //QMetaObject::invokeMethod(this, "test", Qt::QueuedConnection);
}


bool QMoSftpQueueModel::append(const TaskInfo &ti)
{
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TaskInfo& t){
        return ti.local == t.local || ti.remote == t.remote;
    });
    if(it != m_tasks.end()) {
        return false;
    }
    beginResetModel();
    m_tasks.append(ti);

    TaskInfo& tmp = m_tasks.last();
    endResetModel();
    emit lengthChanged();
    return true;
}

bool QMoSftpQueueModel::isEmpty() const
{
    return m_tasks.isEmpty();
}

TaskInfo QMoSftpQueueModel::takeFirst()
{
    beginResetModel();
    const TaskInfo& ti = m_tasks.takeFirst();
    endResetModel();
    emit lengthChanged();
    return ti;
}

void QMoSftpQueueModel::removeTask(int row)
{
    if(row < 0 || row >= m_tasks.length()) {
        return;
    }
    beginResetModel();
    m_tasks.removeAt(row);
    endResetModel();
    emit lengthChanged();
}

void QMoSftpQueueModel::removeAll()
{
    beginResetModel();
    m_tasks.clear();
    endResetModel();
    emit lengthChanged();
}

int QMoSftpQueueModel::length() const
{
    return m_tasks.length();
}

int QMoSftpQueueModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_tasks.count();
}

QVariant QMoSftpQueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QVariant QMoSftpQueueModel::data(const QModelIndex &idx, int role) const
{
    //qDebug() << "HostListModel" << role;
    int row = idx.row();
    int col = idx.column();
    if (row < 0 || row >= m_tasks.size()) {
        return QVariant();
    }
    if (!idx.isValid()){
        return QVariant();
    }
    const TaskInfo& ti = m_tasks.at(row);

    if(role == ROLE_TASKINFO) {
        return convertToVariantMap(ti);
    }

    if(role == ROLE_ICON_URL) {
        return ti.isDir ? "qrc:/woterm/resource/skin/dirs.png" : "qrc:/woterm/resource/skin/file.png";
    }
    if(role == ROLE_SHORT_STRING) {
        return QString("%1-%2").arg(ti.local).arg(ti.remote);
    }
    if(role == ROLE_PATH_LOCAL) {
        return ti.local;
    }
    if(role == ROLE_PATH_REMOTE) {
        return ti.remote;
    }
    return QVariant();
}

int QMoSftpQueueModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QHash<int, QByteArray> QMoSftpQueueModel::roleNames() const
{
    QHash<int,QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(Qt::DisplayRole, "name");
    roles.insert(ROLE_ICON_URL, "iconUrl");
    roles.insert(ROLE_SHORT_STRING, "shortString");
    roles.insert(ROLE_TASKINFO, "taskInfo");
    roles.insert(ROLE_PATH_LOCAL, "local");
    roles.insert(ROLE_PATH_REMOTE, "remote");
    return roles;
}

QVariantMap QMoSftpQueueModel::convertToVariantMap(const TaskInfo &ti) const
{
    QVariantMap dm;
    dm.insert("taskId", ti.taskId);
    dm.insert("local", ti.local);
    dm.insert("remote", ti.remote);
    dm.insert("isDir", ti.isDir);
    dm.insert("state", int(ti.state));
    dm.insert("fileCount", ti.fileCount);
    return dm;
}
