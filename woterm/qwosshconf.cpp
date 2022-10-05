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

#include "qwosshconf.h"
#include "qwosetting.h"
#include "qwoutils.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QBuffer>

/*
 *  #
 *  #    <---memo--->
 *  #
 *  Host name
 *    IgnoreUnknown Password,Group,XXX
 *    HostName xxxx
 *    Port  22
 *    User  abc
 *    IdentityFile ~xxx
 *    Password xxxx
 *    ProxyJump xxx
 *    GroupName xxxx
 */

#define IGNORE_UNKNOW ("Group,Password")

bool lessThan(const HostInfo& a, const HostInfo& b) {
    return a.name < b.name;
}
bool moreThan(const HostInfo& a, const HostInfo& b) {
    return a.name > b.name;
}

void copyHostInfo(HostInfo &dst, const HostInfo &src)
{
    if(!src.host.isEmpty()) {
        dst.host = src.host;
    }
    if(!src.identityFile.isEmpty()) {
        dst.identityFile = src.identityFile;
    }
    if(!src.password.isEmpty()) {
        dst.password = src.password;
    }
    if(src.port > 0) {
        dst.port = src.port;
    }
    if(!src.proxyJump.isEmpty()) {
        dst.proxyJump = src.proxyJump;
    }
    if(!src.user.isEmpty()) {
        dst.user = src.user;
    }
}

QWoSshConf::QWoSshConf(const QString& conf, QObject *parent)
    :QObject (parent)
    ,m_conf(conf)
{
}

QWoSshConf *QWoSshConf::instance()
{
    static QWoSshConf sc(QWoSetting::sshServerListPath());
    return &sc;
}

QHash<QString, HostInfo> QWoSshConf::parse(const QByteArray& buf)
{
    QList<QByteArray> lines = buf.split('\n');
    QList<QStringList> blocks;
    QStringList block;
    for(int i = 0; i < lines.length(); i++) {
        QByteArray line = lines.at(i);
        if(line.startsWith("Host ")) {
            if(!block.isEmpty()) {
                blocks.push_back(block);
                block.clear();
            }
            for(int j = i-1; j > 0; j--) {
                QByteArray prev = lines.at(j);
                if(prev.startsWith('#')){
                    block.insert(0, prev);
                    continue;
                }
                break;
            }
            block.push_back(line);
        }else if(line.startsWith(' ') || line.startsWith('\t')) {
            if(!block.isEmpty()) {
                block.push_back(line);
            }
        }else{
            if(!block.isEmpty()) {
                blocks.push_back(block);
                block.clear();
            }
        }
    }
    if(!block.isEmpty()) {
        blocks.push_back(block);
        block.clear();
    }
    //for(QList<QStringList>::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
    //    qDebug() << *iter;
    //}
    QHash<QString, HostInfo> common;
    QHash<QString, HostInfo> wildcard;
    for(int i = 0; i < blocks.length(); i++) {
        QStringList host = blocks.at(i);
        QStringList memos;
        HostInfo hi;
        //qDebug() << host;
        hi.type = SshWithSftp;
        for(int j = 0; j < host.length(); j++) {
            QString item = host.at(j).trimmed();
            if(item.startsWith("Host ")) {
                hi.name = item.mid(5).trimmed();
            }else if(item.startsWith("Type ")) {
                QString type = item.mid(5).trimmed();
                if(type == "SshWithSftp") {
                    hi.type = SshWithSftp;
                }else if(type == "SftpOnly") {
                    hi.type = SftpOnly;
                }else if(type == "Telnet") {
                    hi.type = Telnet;
                }else if(type == "RLogin") {
                    hi.type = RLogin;
                }else if(type == "Mstsc") {
                    hi.type = Mstsc;
                }else if(type == "Vnc") {
                    hi.type = Vnc;
                }else if(type == "SerialPort") {
                    hi.type = SerialPort;
                }
            }else if(item.startsWith("HostName ")) {
                hi.host = item.mid(9).trimmed();
            }else if(item.startsWith("Port ")) {
                hi.port = item.mid(5).trimmed().toInt();
            }else if(item.startsWith("User ")) {
                hi.user = item.mid(5).trimmed();
            }else if(item.startsWith("IdentityFile ")) {
                hi.identityFile = item.mid(13).trimmed();
            }else if(item.startsWith("Password")) {
                hi.password = item.mid(9).trimmed();
            }else if(item.startsWith("ProxyJump")) {
                hi.proxyJump = item.mid(9).trimmed();
            }else if(item.startsWith("Property")) {
                hi.property = item.mid(8).trimmed();
            }else if(item.startsWith("Script")) {
                hi.script = item.mid(6).trimmed();
            }else if(item.startsWith("ScriptFile")) {
                hi.scriptFile = item.mid(10).trimmed();
            }else if(item.startsWith("#")) {
                memos.push_back(item.mid(1));
            }else if(item.startsWith("BaudRate")) {
                hi.baudRate = item.mid(8).trimmed();
            }else if(item.startsWith("DataBits")) {
                hi.dataBits = item.mid(8).trimmed();
            }else if(item.startsWith("Parity")) {
                hi.parity = item.mid(6).trimmed();
            }else if(item.startsWith("StopBits")) {
                hi.stopBits = item.mid(8).trimmed();
            }else if(item.startsWith("FlowControl")) {
                hi.flowContrl = item.mid(11).trimmed();
            }
        }
        hi.memo = memos.join("\n");
        QStringList names = hi.name.split(' ');
        for(int i = 0; i < names.length(); i++) {
            QString name = names.at(i).trimmed();
            if(name.isEmpty()) {
                continue;
            }
            hi.name = name;
            if(name.contains("*")) {
                wildcard.insert(hi.name, hi);
            }else{
                common.insert(hi.name, hi);
            }
        }
    }
    QHash<QString, HostInfo> hosts;
    for(QHash<QString,HostInfo>::iterator iter = common.begin(); iter != common.end(); iter++) {
        QString name = iter.key();
        HostInfo hi = iter.value();
        HostInfo hiTmp;
        bool hitWildcard = false;
        for(QHash<QString,HostInfo>::iterator iter = wildcard.begin(); iter != wildcard.end(); iter++) {
            QString nameHit = iter.key();
            HostInfo hiHit = iter.value();
            QRegExp rx(nameHit);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if(rx.exactMatch(name)) {
                hitWildcard = true;
                copyHostInfo(hiTmp, hiHit);
            }
        }
        if(hitWildcard) {
            copyHostInfo(hi, hiTmp);
        }

        hosts.insert(hi.name, hi);
    }

    return hosts;
}

QByteArray QWoSshConf::toStream()
{
    QByteArray buf;
    QBuffer file(&buf);
    file.open(QIODevice::WriteOnly);
    QList<QString> hosts = m_hosts.keys();
    std::sort(hosts.begin(), hosts.end());
    for(int i = 0; i < hosts.length(); i++) {
        HostInfo hi = m_hosts[hosts.at(i)];
        file.write("\n", 1);
        if(!hi.memo.isEmpty()) {
            QStringList comments = hi.memo.split('\n');
            for(int j = 0; j < comments.length(); j++) {
                QString line(QString("#%1\n").arg(comments.at(j)));
                file.write(line.toUtf8());
            }
        }
        if(!hi.name.isEmpty()) {
            QString line(QString("Host %1\n").arg(hi.name));
            file.write(line.toUtf8());
        }
        if(1) {
            QString line(QString("  IgnoreUnknown %1\n").arg(IGNORE_UNKNOW));
            file.write(line.toUtf8());
        }
        if(1) {
            QString type;
            if(hi.type == SshWithSftp) {
                type = "SshWithSftp";
            }else if(hi.type == SftpOnly) {
                type = "SftpOnly";
            }else if(hi.type == Telnet) {
                type = "Telnet";
            }else if(hi.type == RLogin) {
                type = "RLogin";
            }else if(hi.type == Mstsc) {
                type = "Mstsc";
            }else if(hi.type == Vnc) {
                type = "Vnc";
            }else if(hi.type == SerialPort) {
                type = "SerialPort";
            }
            QString line(QString("  Type %1\n").arg(type));
            file.write(line.toUtf8());
        }
        if(!hi.host.isEmpty()) {
            QString line(QString("  HostName %1\n").arg(hi.host));
            file.write(line.toUtf8());
        }
        if(hi.port > 0) {
            QString line(QString("  Port %1\n").arg(hi.port));
            file.write(line.toUtf8());
        }
        if(!hi.user.isEmpty()) {
            QString line(QString("  User %1\n").arg(hi.user));
            file.write(line.toUtf8());
        }
        if(!hi.identityFile.isEmpty()) {
            QString line(QString("  IdentityFile %1\n").arg(hi.identityFile));
            file.write(line.toUtf8());
        }
        if(!hi.password.isEmpty()) {
            QString line(QString("  Password %1\n").arg(hi.password));
            file.write(line.toUtf8());
        }
        if(!hi.proxyJump.isEmpty()) {
            QString line(QString("  ProxyJump %1\n").arg(hi.proxyJump));
            file.write(line.toUtf8());
        }
        if(!hi.script.isEmpty()) {
            QString line(QString("  Script %1\n").arg(hi.script));
            file.write(line.toUtf8());
        }
        if(!hi.scriptFile.isEmpty()) {
            QString line(QString("  ScriptFile %1\n").arg(hi.scriptFile));
            file.write(line.toUtf8());
        }
        if(!hi.property.isEmpty()) {
            QString line(QString("  Property %1\n").arg(hi.property));
            file.write(line.toUtf8());
        }
        if(!hi.baudRate.isEmpty()) {
            QString line(QString("  BaudRate %1\n").arg(hi.baudRate));
            file.write(line.toUtf8());
        }
        if(!hi.dataBits.isEmpty()) {
            QString line(QString("  DataBits %1\n").arg(hi.dataBits));
            file.write(line.toUtf8());
        }
        if(!hi.parity.isEmpty()) {
            QString line(QString("  Parity %1\n").arg(hi.parity));
            file.write(line.toUtf8());
        }
        if(!hi.stopBits.isEmpty()) {
            QString line(QString("  StopBits %1\n").arg(hi.stopBits));
            file.write(line.toUtf8());
        }
        if(!hi.flowContrl.isEmpty()) {
            QString line(QString("  FlowControl %1\n").arg(hi.flowContrl));
            file.write(line.toUtf8());
        }
    }
    return buf;
}

bool QWoSshConf::save()
{
    return exportToFile(m_conf);
}

bool QWoSshConf::refresh()
{
    QFile file(m_conf);
    if(!file.exists()) {
        return false;
    }
    if(!file.open(QFile::ReadOnly)) {
      QMessageBox::warning(nullptr, tr("LoadSshConfig"), tr("Failed to open file:")+m_conf, QMessageBox::Ok);
      return false;
    }
    QByteArray buf = file.readAll();
    buf = QWoUtils::fromWotermStream(buf);
    //qDebug() << buf;
    m_hosts = parse(buf);
    return true;
}

bool QWoSshConf::exportToFile(const QString &path)
{
    QByteArray buf = toStream();
    //qDebug() << buf;

    buf = QWoUtils::toWotermStream(buf);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)){
        return false;
    }
    file.write(buf);
    return true;
}

void QWoSshConf::remove(const QString &name)
{
    m_hosts.remove(name);
    save();
}

bool QWoSshConf::modify(const HostInfo &hi)
{
    if(!m_hosts.contains(hi.name)) {
        return false;
    }
    m_hosts.insert(hi.name, hi);
    return save();
}

bool QWoSshConf::append(const HostInfo &hi)
{
    if(m_hosts.contains(hi.name)) {
        return false;
    }
    m_hosts.insert(hi.name, hi);
    return save();
}

bool QWoSshConf::modifyOrAppend(const HostInfo &hi)
{
    m_hosts.insert(hi.name, hi);
    return save();
}

void QWoSshConf::resetAllProperty(const QString &v)
{
    for(QHash<QString, HostInfo>::iterator iter = m_hosts.begin(); iter != m_hosts.end(); iter++) {
        HostInfo& hi = iter.value();
        hi.property = v;
    }
    save();
}

bool QWoSshConf::exists(const QString &name) const
{
    return m_hosts.contains(name);
}

void QWoSshConf::updatePassword(const QString &name, const QString &password)
{
    if(m_hosts.contains(name)) {
        HostInfo &hi = m_hosts[name];
        hi.password = password;
        save();
    }
}

QList<HostInfo> QWoSshConf::hostList(EHostType type) const
{
    QList<HostInfo> hosts = m_hosts.values();
    std::sort(hosts.begin(), hosts.end(), lessThan);
    if(type != All) {
        for(QList<HostInfo>::iterator iter = hosts.begin(); iter != hosts.end();) {
            const HostInfo& hi = *iter;
            if(type == SshBase && (hi.type == SshWithSftp || hi.type == SftpOnly )) {
                iter++;
                continue;
            }
            if(hi.type != type) {
                iter = hosts.erase(iter);
            }else{
                iter++;
            }
        }
    }
    return hosts;
}

QStringList QWoSshConf::hostNameList(EHostType type) const
{
    if(type == All) {
        QStringList names = m_hosts.keys();
        std::sort(names.begin(), names.end());
        return names;
    }
    QList<HostInfo> hosts = m_hosts.values();
    std::sort(hosts.begin(), hosts.end(), lessThan);
    QStringList names;
    if(type != All) {
        for(QList<HostInfo>::iterator iter = hosts.begin(); iter != hosts.end();iter++) {
            const HostInfo& hi = *iter;
            if(type == SshBase && (hi.type == SshWithSftp || hi.type == SftpOnly )) {
                names.append(hi.name);
            }else if(hi.type == type) {
                names.append(hi.name);
            }
        }
    }
    return names;
}

QList<HostInfo> QWoSshConf::proxyJumpers(const QString& name, int max) const
{
    QList<HostInfo> his;
    HostInfo hi = find(name);
    if(!hi.isValid() || max <= 0) {
        return his;
    }
    his.append(hi);
    if(hi.hasProxyJump()) {
        his.append(proxyJumpers(hi.proxyJump, max-1));
    }
    return his;
}

HostInfo QWoSshConf::find(const QString &name) const {
    return m_hosts.value(name);
}

bool QWoSshConf::find(const QString &name, HostInfo *pinfo) const
{
    if(m_hosts.contains(name)) {
        HostInfo hi = m_hosts.value(name);
        *pinfo = hi;
        return true;
    }
    return false;
}
