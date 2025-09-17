/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwodbservermergemodel.h"

#include "qkxcrc.h"
#include "qwosshconf.h"

#include <QDebug>

/*
std::string sql = "update servers set type=@type,host=@host,port=@port,loginName=@loginName,";
sql += "loginPassword=@loginPassword,identityFile=@identityFile,scriptFile=@scriptFile,script=@script,";
sql += "proxyJump=@proxyJump,memo=@memo,property=@property,groupName=@groupName,baudRate=@baudRate,";
sql += "dataBits=@dataBits,parity=@parity,stopBits=@stopBits,flowControl=@flowControl ";
sql += "where name=@name and delFlag=0";
*/

static void qVariantMapMergeToHostInfo(HostInfo& hi, QVariantMap dm) {
    hi.name = dm.value("name").toString();
    hi.type = (EHostType)dm.value("type").toInt();
    hi.host = dm.value("host").toString();
    hi.port = dm.value("port").toInt();
    hi.user = dm.value("loginName").toString();
    hi.password = dm.value("loginPassword").toString();
    hi.identityFile = dm.value("identityFile").toString();
    hi.scriptFile = dm.value("scriptFile").toString();
    hi.script = dm.value("script").toString();
    hi.proxyJump = dm.value("proxyJump").toString();
    hi.memo = dm.value("meno").toString();
    QMap<QString, QVariantMap> props = HostInfo::base64ToConfigure(dm.value("property").toString());
    hi.props = hi.merge(hi.props, props);
    hi.group = dm.value("groupName").toString();
    hi.baudRate = dm.value("baudRate").toString();
    hi.dataBits = dm.value("dataBits").toString();
    hi.parity = dm.value("parity").toString();
    hi.stopBits = dm.value("stopBits").toString();
    hi.flowControl = dm.value("flowControl").toString();
}

static HostInfo qVariantMapToHostInfo(QVariantMap dm) {
    HostInfo hi;
    hi.name = dm.value("name").toString();
    hi.type = (EHostType)dm.value("type").toInt();
    hi.host = dm.value("host").toString();
    hi.port = dm.value("port").toInt();
    hi.user = dm.value("loginName").toString();
    hi.password = dm.value("loginPassword").toString();
    hi.identityFile = dm.value("identityFile").toString();
    hi.scriptFile = dm.value("scriptFile").toString();
    hi.script = dm.value("script").toString();
    hi.proxyJump = dm.value("proxyJump").toString();
    hi.memo = dm.value("meno").toString();
    QMap<QString, QVariantMap> props = HostInfo::base64ToConfigure(dm.value("property").toString());
    hi.props = props;
    hi.group = dm.value("groupName").toString();
    hi.baudRate = dm.value("baudRate").toString();
    hi.dataBits = dm.value("dataBits").toString();
    hi.parity = dm.value("parity").toString();
    hi.stopBits = dm.value("stopBits").toString();
    hi.flowControl = dm.value("flowControl").toString();
    return hi;
}


QWoDBServerMergeModel::QWoDBServerMergeModel(QObject *parent)
    : QWoDBMergeModel(parent)
{

}

void QWoDBServerMergeModel::runApply()
{
    // adds
    for(auto it = m_mi.rhave.begin(); it != m_mi.rhave.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "add") {
            dm.insert("mergeAction", "done");
            HostInfo hi = qVariantMapToHostInfo(remote);
            QWoSshConf::instance()->append(hi);
        }
    }

    // remove
    for(auto it = m_mi.remove.begin(); it != m_mi.remove.end(); ) {
        QVariantMap dm = *it;
        QVariantMap local = dm.value("local").toMap();
        QString name = local.value("name").toString();
        QWoSshConf::instance()->removeServer(name);
        it = m_mi.remove.erase(it);
    }

    // replace
    for(auto it = m_mi.replace.begin(); it != m_mi.replace.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "replace") {
            dm.insert("mergeAction", "done");
            HostInfo hi;
            qVariantMapMergeToHostInfo(hi, remote);
            QWoSshConf::instance()->modify(hi);
        }
    }
}

QString QWoDBServerMergeModel::toString(const QVariantMap &dm) const
{
    QStringList names = {tr("groupName"),tr("host"),tr("name"),tr("port"),tr("property"),tr("memo"),tr("proxyJump"),tr("loginName"),tr("identityFile"),tr("type")};
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
                || key == "loginPassword"
                || key == "scriptFile"
                || key == "script") {
            continue;
        }
        if(txt.isEmpty()) {
            continue;
        }
        if(key == "property") {
            txt = "HASH("+QString::number(QKxCRC::crc32(txt.toUtf8()), 16)+")";
        }
        QString line = tr(key.toUtf8()) + ":" + txt;
        lines.append(line);
    }
    QString out = lines.join("\r\n");
    return out;
}
