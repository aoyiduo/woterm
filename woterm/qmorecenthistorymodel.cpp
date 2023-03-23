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

#include "qmorecenthistorymodel.h"

#include "qwoglobal.h"
#include "qwosshconf.h"
#include "qwosetting.h"

#include <QDateTime>
#include <QDataStream>

#define MAX_COUNT   (10)

QMoRecentHistoryModel::QMoRecentHistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void QMoRecentHistoryModel::update(const QString &name, int type)
{
    beginResetModel();
    auto it = std::find_if(m_recents.begin(), m_recents.end(), [=](const RecentHistoryData& v){
        const RecentHistoryData& ra = v;
        return name == ra.name && type == ra.type;
    });
    if(it != m_recents.end()) {
        m_recents.erase(it);
    }
    RecentHistoryData ra;
    ra.name = name;
    ra.type = type;
    ra.timeLast = QDateTime::currentSecsSinceEpoch();
    m_recents.prepend(ra);
    if(m_recents.length() > MAX_COUNT) {
        m_recents.removeLast();
    }
    endResetModel();
    save();

    emit lengthChanged();
}

void QMoRecentHistoryModel::clear()
{
    beginResetModel();
    m_recents.clear();
    endResetModel();
    emit lengthChanged();
}

int QMoRecentHistoryModel::length() const
{
    return m_recents.length();
}

int QMoRecentHistoryModel::rowCount(const QModelIndex &parent) const
{
    return m_recents.length();
}

int QMoRecentHistoryModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QMoRecentHistoryModel::data(const QModelIndex &idx, int role) const
{
    int row = idx.row();
    if(row >= m_recents.length() || row < 0) {
        return QVariant();
    }

    QWoSshConf *gSshConf = QWoSshConf::instance();
    if(gSshConf == nullptr) {
        return QVariant();
    }
    const RecentHistoryData& ra = m_recents.at(row);
    const HostInfo& hi = gSshConf->find(ra.name);

    if(role == Qt::DisplayRole) {
        return ra.name;
    }
    if(role == ROLE_ICON_URL) {
        switch (ra.type) {
        case EOT_SSH:
            return "qrc:/woterm/resource/skin/ssh2.png";
        case EOT_SFTP:
            return "qrc:/woterm/resource/skin/sftp.png";
        case EOT_TELNET:
            return "qrc:/woterm/resource/skin/telnet.png";
        case EOT_RLOGIN:
            return "qrc:/woterm/resource/skin/rlogin.png";
        case EOT_MSTSC:
            return "qrc:/woterm/resource/skin/mstsc2.png";
        case EOT_VNC:
            return "qrc:/woterm/resource/skin/vnc2.png";
        }
        return QVariant();
    }
    if(role == ROLE_SHORT_STRING) {
        QDateTime dt = QDateTime::fromSecsSinceEpoch(ra.timeLast);
        return QString("%1:%2 %3:%4").arg(hi.host).arg(hi.port).arg(tr("Last time:")).arg(dt.toString("yyyy-MM-dd hh:mm:ss"));
    }
    if(role == ROLE_GROUP_NAME) {
        return hi.group;
    }
    if(role == ROLE_TYPE) {
        return ra.type;
    }
    return QVariant();
}

QVariant QMoRecentHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QHash<int, QByteArray> QMoRecentHistoryModel::roleNames() const
{
    QHash<int,QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(Qt::DisplayRole, "name");
    roles.insert(ROLE_ICON_URL, "iconUrl");
    roles.insert(ROLE_SHORT_STRING, "shortString");
    roles.insert(ROLE_GROUP_NAME, "groupName");
    roles.insert(ROLE_TYPE, "type");
    return roles;
}

void QMoRecentHistoryModel::save()
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << int(m_recents.size());
    for(auto it = m_recents.begin(); it != m_recents.end(); it++) {
        RecentHistoryData rhd = *it;
        ds << rhd.name << rhd.timeLast << int(rhd.type);
    }
    QWoSetting::setValue("recentHistory", buf);
}

void QMoRecentHistoryModel::init()
{
    QByteArray buf = QWoSetting::value("recentHistory").toByteArray();
    QDataStream ds(buf);
    int cnt;
    ds >> cnt;
    beginResetModel();
    for(int i = 0; i < cnt; i++) {
        RecentHistoryData rhd;
        int type;
        ds >> rhd.name >> rhd.timeLast >> type;
        rhd.type = type;
        m_recents.append(rhd);
    }
    endResetModel();
    emit lengthChanged();
}
