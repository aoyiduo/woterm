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

#include "qwodbservermergemodel.h"

#include <QDebug>

QWoDBServerMergeModel::QWoDBServerMergeModel(QObject *parent)
    : QWoDBMergeModel(parent)
{

}

QString QWoDBServerMergeModel::toString(const QVariantMap &dm) const
{
    QStringList names = {tr("groupName"),tr("host"),tr("name"),tr("port"),tr("baudRate"),tr("dataBits"),tr("memo"),
                         tr("parity"),tr("stopBits"),tr("flowControl"),tr("proxyJump"),tr("loginName"),tr("identityFile"),tr("type")};
    QStringList lines;
    for(auto it = dm.begin(); it != dm.end(); it++) {
        QString key = it.key();
        QString txt = it.value().toString();
        if(key == "ct"
                || key == "delFlag"
                || key == "dt"
                || key == "id"
                || key == "version"
                || key == "syncFlag"
                || key == "property"
                || key == "loginPassword"
                || key == "scriptFile"
                || key == "script") {
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
