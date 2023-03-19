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

#include "qworecenthistory.h"

#include "qwoglobal.h"
#include "qwosshconf.h"
#include "qwosetting.h"

#include <QDateTime>
#include <QDataStream>
#include <QMenu>
#include <QAction>

#define MAX_COUNT   (5)

QWoRecentHistory::QWoRecentHistory(QObject *parent)
    : QObject(parent)
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void QWoRecentHistory::update(const QString &name, int type)
{
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
    save();
}

void QWoRecentHistory::buildMenu(QMenu *menu)
{
    auto iconFile=[=](int t){
        switch (t) {
        case EOT_SSH:
            return ":/woterm/resource/skin/ssh2.png";
        case EOT_SFTP:
            return ":/woterm/resource/skin/sftp.png";
        case EOT_TELNET:
            return ":/woterm/resource/skin/telnet.png";
        case EOT_RLOGIN:
            return ":/woterm/resource/skin/rlogin.png";
        case EOT_MSTSC:
            return ":/woterm/resource/skin/mstsc2.png";
        case EOT_VNC:
            return ":/woterm/resource/skin/vnc2.png";
        }
        return "";
    };

    for(auto it = m_recents.begin(); it != m_recents.end(); it++) {
        const RecentHistoryData& rhd = *it;
        QAction *a = menu->addAction(QIcon(iconFile(rhd.type)), rhd.name, this, SLOT(onMenuTriggered()));
        QVariantMap dm;
        dm.insert("name", rhd.name);
        dm.insert("type", rhd.type);
        a->setData(dm);
    }
}

void QWoRecentHistory::onClearAll()
{
    m_recents.clear();
    save();
}

void QWoRecentHistory::onMenuTriggered()
{
    QAction *a = qobject_cast<QAction*>(sender());
    QVariantMap dm = a->data().toMap();
    QString name = dm.value("name").toString();
    int t = dm.value("type").toInt();
    emit readyToConnect(name, t);
}

void QWoRecentHistory::save()
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

void QWoRecentHistory::init()
{
    QByteArray buf = QWoSetting::value("recentHistory").toByteArray();
    QDataStream ds(buf);
    int cnt;
    ds >> cnt;
    for(int i = 0; i < cnt; i++) {
        RecentHistoryData rhd;
        int type;
        ds >> rhd.name >> rhd.timeLast >> type;
        rhd.type = type;
        m_recents.append(rhd);
    }
}
