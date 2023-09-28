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

#include "qwodbgroupmergemodel.h"
#include "qwosshconf.h"

QWoDBGroupMergeModel::QWoDBGroupMergeModel(QObject *parent)
    : QWoDBMergeModel(parent)
{

}

void QWoDBGroupMergeModel::runApply()
{
    // adds
    for(auto it = m_mi.rhave.begin(); it != m_mi.rhave.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "add") {
            dm.insert("mergeAction", "done");
            QString name = remote.value("name").toString();
            int orderNum = remote.value("orderNum").toInt();
            QWoSshConf::instance()->updateGroup(name, orderNum);
        }
    }

    // remove
    for(auto it = m_mi.remove.begin(); it != m_mi.remove.end(); ) {
        QVariantMap dm = *it;
        QVariantMap local = dm.value("local").toMap();
        QString name = local.value("name").toString();
        QWoSshConf::instance()->removeGroup(name);
        it = m_mi.remove.erase(it);
    }

    // replace
    for(auto it = m_mi.replace.begin(); it != m_mi.replace.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "replace") {
            dm.insert("mergeAction", "done");
            QString name = remote.value("name").toString();
            int orderNum = remote.value("orderNum").toInt();
            QWoSshConf::instance()->updateGroup(name, orderNum);
        }
    }
}

QString QWoDBGroupMergeModel::toString(const QVariantMap &dm) const
{
    QStringList names = {tr("name"),tr("orderNum")};
    QStringList lines;
    for(auto it = dm.begin(); it != dm.end(); it++) {
        QString key = it.key();
        QString txt = it.value().toString();
        if(key == "ct"
                || key == "id"
                || key == "syncFlag") {
            continue;
        }
        if(txt.isEmpty()) {
            continue;
        }
        QString line = tr(key.toUtf8()) + ":" + txt;
        lines.append(line);
    }
    QString out = lines.join("\r\n");
    return out;
}
