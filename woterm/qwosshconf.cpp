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
#include "qwoidentify.h"
#include "qkxmessagebox.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QBuffer>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

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

#define IGNORE_UNKNOW       ("Group,Password")

#define PASSWORD_ENCRYPT    ("WoTerm@2022-11-6")

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

static bool createServersTable(SQLite::Database& db) {
    QString sql="CREATE TABLE IF NOT EXISTS servers(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql += "type INT,";
    sql += "name VARCHAR(100) NOT NULL,";
    sql += "host VARCHAR(100) NOT NULL,";
    sql += "port INT,";
    sql += "loginName VARCHAR(100),";
    sql += "loginPassword VARCHAR(100),";
    sql += "identityFile VARCHAR(100),";
    sql += "scriptFile VARCHAR(100),";
    sql += "script TEXT,";
    sql += "proxyJump VARCHAR(100),";
    sql += "memo TEXT,";
    sql += "property TEXT,";
    sql += "groupName VARCHAR(100),";
    sql += "baudRate VARCHAR(100),";
    sql += "dataBits VARCHAR(100),";
    sql += "parity VARCHAR(100),";
    sql += "stopBits VARCHAR(100),";
    sql += "flowControl VARCHAR(100),";
    sql += "version INT DEFAULT (0),";
    sql += "delFlag INT DEFAULT (0),";
    sql += "syncFlag INT DEFAULT (0),";
    sql += "ct DATETIME NOT NULL,";
    sql += "dt INT DEFAULT (0)";
    sql +=")";
    db.exec(sql.toUtf8());
    if(db.tableExists("servers")) {
        SQLite::Statement query(db, "PRAGMA INDEX_INFO('servers_idx')");
        if(!query.executeStep()) {
            db.exec("CREATE UNIQUE INDEX servers_idx ON servers (name,delFlag)");
        }else{
            QString v = QString::fromStdString(query.getColumn("name").getString());
            qDebug() << v << query.getColumnCount();
        }
        return true;
    }
    return false;
}

static bool createIdentitiesTable(SQLite::Database& db) {
    QString sql="CREATE TABLE IF NOT EXISTS identities(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql += "name VARCHAR(100) NOT NULL,";
    sql += "prvKey TEXT NOT NULL,";
    sql += "delFlag INT DEFAULT (0),";
    sql += "syncFlag INT DEFAULT (0),";
    sql += "ct DATETIME NOT NULL,";
    sql += "dt INT DEFAULT (0)";
    sql +=")";
    db.exec(sql.toUtf8());
    if(db.tableExists("identities")) {
        SQLite::Statement query(db, "PRAGMA INDEX_INFO('identities_idx')");
        if(!query.executeStep()) {
            db.exec("CREATE UNIQUE INDEX identities_idx ON identities (name,delFlag)");
        }else{
            QString v = QString::fromStdString(query.getColumn("name").getString());
            qDebug() << v << query.getColumnCount();
        }
        return true;
    }
    return false;
}

static bool createGroupsTable(SQLite::Database& db) {
    QString sql="CREATE TABLE IF NOT EXISTS groups(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql += "name VARCHAR(100) UNIQUE NOT NULL,";
    sql += "orderNum INT DEFAULT (0),";
    sql += "syncFlag INT DEFAULT (0),";
    sql += "ct DATETIME NOT NULL";
    sql +=")";
    db.exec(sql.toUtf8());
    if(db.tableExists("groups")) {
        return true;
    }
    return false;
}

QWoSshConf::QWoSshConf(const QString& dbFile, QObject *parent)
    : QObject (parent)
    , m_dbFile(dbFile)
    , m_bInit(false)
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
}

void QWoSshConf::init()
{
    if(m_bInit) {
        return;
    }
    QString bakFile = m_dbFile + ".bak";
    if(QFile::exists(m_dbFile)) {
        if(!databaseValid(m_dbFile)) {
            // bad db file.
            QFile::remove(m_dbFile);
            if(databaseValid(bakFile)) {
                restore(bakFile);
            }else{
                QFile::remove(bakFile);
                init();
            }
        }
    }else if(QFile::exists(bakFile)){
        if(databaseValid(bakFile)) {
            restore(bakFile);
        }else{
            QFile::remove(bakFile);
            init();
        }
    }else{
        QString filePath = QWoSetting::sshServerFilePath();
        if(QFile::exists(filePath)) {
            QWoSshConf::importIdentityToSqlite(QWoSetting::identityFilePath(), m_dbFile);
            QWoSshConf::importConfToSqlite(filePath, m_dbFile);
        }else{
            try{
                SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
                createIdentitiesTable(db);
                createServersTable(db);
                createGroupsTable(db);
            }catch(...) {
                qWarning() << "failed to create database" << m_dbFile;
            }
        }
    }
    m_bInit = databaseValid(m_dbFile);
}

QWoSshConf *QWoSshConf::instance()
{
    static QWoSshConf sc(QWoSetting::sshServerDbPath());
    return &sc;
}

bool QWoSshConf::restore(const QString &dbBackup)
{
    if(dbBackup.isEmpty()) {
        return false;
    }
    if(!databaseValid(dbBackup)) {
        return false;
    }
    try {
        QFile::remove(m_dbFile);
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        int err = db.backup(dbBackup.toUtf8(), db.Load);
        resetLater();
        return err == SQLite::OK;
    } catch (...) {
        qDebug() << "SQLite initialize failed";
    }

    return false;
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
                hi.flowControl = item.mid(11).trimmed();
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
        if(!hi.flowControl.isEmpty()) {
            QString line(QString("  FlowControl %1\n").arg(hi.flowControl));
            file.write(line.toUtf8());
        }
    }
    return buf;
}

void QWoSshConf::importIdentityToSqlite(const QString &path, const QString &dbFile)
{
    QMap<QString, IdentifyInfo> all = QWoIdentify::loadFromFile();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(createIdentitiesTable(db)) {
            QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
            SQLite::Statement insert(db, "INSERT INTO identities (name,prvKey,ct) VALUES (@name,@prvKey,@ct)");
            for(auto it = all.begin(); it != all.end(); it++) {
                IdentifyInfo info = it.value();
                insert.reset();
                insert.bind("@name", info.name.toStdString());
                insert.bind("@prvKey", info.prvKey.toStdString());
                insert.bind("@ct", now.toStdString());
                int cnt = insert.exec();
                qDebug() << "insert identities" << info.name << cnt;
            }
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "convertConfToSqlite" << err;
    }

}

void QWoSshConf::importConfToSqlite(const QString &conf, const QString &dbFile)
{
    QFile file(conf);
    if(!file.exists()) {
        return;
    }
    if(!file.open(QFile::ReadOnly)) {
        return;
    }
    QByteArray buf = file.readAll();
    buf = QWoUtils::fromWotermStream(buf);
    QHash<QString, HostInfo> hosts = parse(buf);
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(createServersTable(db)) {
            QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
            std::string sql = "INSERT INTO servers ";
            sql += "(type,name,host,port,loginName,loginPassword,identityFile,scriptFile,script,proxyJump,memo,property,groupName,baudRate,dataBits,parity,stopBits,flowControl,ct)";
            sql += " VALUES ";
            sql += "(@type,@name,@host,@port,@loginName,@loginPassword,@identityFile,@scriptFile,@script,@proxyJump,@memo,@property,@groupName,@baudRate,@dataBits,@parity,@stopBits,@flowControl,@ct)";
            SQLite::Statement insert(db, sql);
            for(auto it = hosts.begin(); it != hosts.end(); it++) {
                QString name = it.key();
                const HostInfo& hi = it.value();
                insert.reset();
                insert.bind("@type", (int)hi.type);
                insert.bind("@name", hi.name.toStdString());
                insert.bind("@host", hi.host.toStdString());
                insert.bind("@port", hi.port);
                insert.bind("@loginName", hi.user.toStdString());
                insert.bind("@loginPassword", QWoUtils::aesEncrypt(hi.password.toUtf8(), PASSWORD_ENCRYPT).toStdString());
                QString identityFile = hi.identityFile;
                if(hi.identityFile.startsWith("woterm:")) {
                    identityFile = identityFile.mid(7);
                    identityFile = QWoUtils::pathToName(identityFile);
                }
                insert.bind("@identityFile", identityFile.toStdString());
                insert.bind("@scriptFile", hi.scriptFile.toStdString());
                insert.bind("@script", hi.script.toStdString());
                insert.bind("@proxyJump", hi.proxyJump.toStdString());
                insert.bind("@memo", hi.memo.toStdString());
                insert.bind("@property", hi.property.toStdString());
                insert.bind("@groupName", hi.group.toStdString());
                insert.bind("@baudRate", hi.baudRate.toStdString());
                insert.bind("@dataBits", hi.dataBits.toStdString());
                insert.bind("@parity", hi.parity.toStdString());
                insert.bind("@stopBits", hi.stopBits.toStdString());
                insert.bind("@flowControl", hi.flowControl.toStdString());
                insert.bind("@ct", now.toStdString());
                int cnt = insert.exec();
                qDebug() << "insert server" << cnt << hi.name;
            }
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "convertConfToSqlite" << err;
    }
}

QHash<QString, HostInfo> QWoSshConf::loadServerFromSqlite(const QString &dbFile)
{
    QHash<QString, HostInfo> hosts;
    try{
        bool hasDefault = false;
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "select svc.*,ids.prvKey from servers as svc left join identities as ids on svc.identityFile=ids.name and ids.delFlag=0 where svc.delFlag=0");
        while(query.executeStep()) {
            HostInfo hi;
            hi.type = EHostType(query.getColumn("type").getInt());
            if(hi.type == SerialPort) {
                continue;
            }
            hi.name = QString::fromStdString(query.getColumn("name").getString());
            hi.host = QString::fromStdString(query.getColumn("host").getString());
            hi.port = query.getColumn("port").getInt();
            hi.user = QString::fromStdString(query.getColumn("loginName").getString());
            hi.password = QWoUtils::aesDecrypt(QByteArray::fromStdString(query.getColumn("loginPassword").getString()), PASSWORD_ENCRYPT);
            hi.identityFile = QString::fromStdString(query.getColumn("identityFile").getString());
            hi.identityContent = QString::fromStdString(query.getColumn("prvKey").getString());
            hi.scriptFile = QString::fromStdString(query.getColumn("scriptFile").getString());
            hi.script = QString::fromStdString(query.getColumn("script").getString());
            hi.proxyJump = QString::fromStdString(query.getColumn("proxyJump").getString());
            hi.memo = QString::fromStdString(query.getColumn("memo").getString());
            hi.property = QString::fromStdString(query.getColumn("property").getString());
            hi.group = QString::fromStdString(query.getColumn("groupName").getString());
            if(hi.group.isEmpty()) {
                hi.group = "Default"; // don't translate it.
                hasDefault = true;
            }
            hi.baudRate = QString::fromStdString(query.getColumn("baudRate").getString());
            hi.dataBits = QString::fromStdString(query.getColumn("dataBits").getString());
            hi.parity = QString::fromStdString(query.getColumn("parity").getString());
            hi.stopBits = QString::fromStdString(query.getColumn("stopBits").getString());
            hi.flowControl = QString::fromStdString(query.getColumn("flowControl").getString());
            hosts.insert(hi.name, hi);
        }
        if(hasDefault) {
            SQLite::Statement update(db, "UPDATE servers SET groupName=\"Default\" WHERE groupName=\"\"");
            int cnt = update.exec();
            qDebug() << "recorect group to Default" << cnt;
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "convertConfToSqlite" << err;
    }
    return hosts;
}

QList<GroupInfo> QWoSshConf::loadGroupFromSqlite(const QString& dbFile)
{
    QList<GroupInfo> names;
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        SQLite::Statement query(db, "select name,orderNum from groups order by orderNum asc");
        while(query.executeStep()) {
            GroupInfo gi;
            gi.name = QString::fromStdString(query.getColumn("name").getString());
            gi.order = query.getColumn("orderNum").getInt();
            names.append(gi);
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "groupNameList" << err;
    }
    return names;
}

bool QWoSshConf::save(const HostInfo &hi)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("servers")) {
            return false;
        }
        SQLite::Statement query(db, "select name from servers where name=@name and delFlag=0");
        query.bind("@name", hi.name.toStdString());
        if(query.executeStep()) {
            std::string sql = "update servers set type=@type,host=@host,port=@port,loginName=@loginName,";
            sql += "loginPassword=@loginPassword,identityFile=@identityFile,scriptFile=@scriptFile,script=@script,";
            sql += "proxyJump=@proxyJump,memo=@memo,property=@property,groupName=@groupName,baudRate=@baudRate,";
            sql += "dataBits=@dataBits,parity=@parity,stopBits=@stopBits,flowControl=@flowControl ";
            sql += "where name=@name and delFlag=0";
            SQLite::Statement update(db, sql);
            update.bind("@type", (int)hi.type);
            update.bind("@host", hi.host.toStdString());
            update.bind("@port", hi.port);
            update.bind("@loginName", hi.user.toStdString());
            update.bind("@loginPassword", QWoUtils::aesEncrypt(hi.password.toUtf8(), PASSWORD_ENCRYPT).toStdString());
            update.bind("@identityFile", hi.identityFile.toStdString());
            update.bind("@scriptFile", hi.scriptFile.toStdString());
            update.bind("@script", hi.script.toStdString());
            update.bind("@proxyJump", hi.proxyJump.toStdString());
            update.bind("@memo", hi.memo.toStdString());
            update.bind("@property", hi.property.toStdString());
            update.bind("@groupName", hi.group.toStdString());
            update.bind("@baudRate", hi.baudRate.toStdString());
            update.bind("@dataBits", hi.dataBits.toStdString());
            update.bind("@parity", hi.parity.toStdString());
            update.bind("@stopBits", hi.stopBits.toStdString());
            update.bind("@flowControl", hi.flowControl.toStdString());
            update.bind("@name", hi.name.toStdString());
            int cnt = update.exec();
            qDebug() << "update server" << cnt << hi.name;
        }else{
            QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
            std::string sql = "INSERT INTO servers ";
            sql += "(type,name,host,port,loginName,loginPassword,identityFile,scriptFile,script,proxyJump,memo,property,groupName,baudRate,dataBits,parity,stopBits,flowControl,ct)";
            sql += " VALUES ";
            sql += "(@type,@name,@host,@port,@loginName,@loginPassword,@identityFile,@scriptFile,@script,@proxyJump,@memo,@property,@groupName,@baudRate,@dataBits,@parity,@stopBits,@flowControl,@ct)";
            SQLite::Statement insert(db, sql);
            insert.reset();
            insert.bind("@type", (int)hi.type);
            insert.bind("@name", hi.name.toStdString());
            insert.bind("@host", hi.host.toStdString());
            insert.bind("@port", hi.port);
            insert.bind("@loginName", hi.user.toStdString());
            insert.bind("@loginPassword", QWoUtils::aesEncrypt(hi.password.toUtf8(), PASSWORD_ENCRYPT).toStdString());
            insert.bind("@identityFile", hi.identityFile.toStdString());
            insert.bind("@scriptFile", hi.scriptFile.toStdString());
            insert.bind("@script", hi.script.toStdString());
            insert.bind("@proxyJump", hi.proxyJump.toStdString());
            insert.bind("@memo", hi.memo.toStdString());
            insert.bind("@property", hi.property.toStdString());
            insert.bind("@groupName", hi.group.toStdString());
            insert.bind("@baudRate", hi.baudRate.toStdString());
            insert.bind("@dataBits", hi.dataBits.toStdString());
            insert.bind("@parity", hi.parity.toStdString());
            insert.bind("@stopBits", hi.stopBits.toStdString());
            insert.bind("@flowControl", hi.flowControl.toStdString());
            insert.bind("@ct", now.toStdString());
            int cnt = insert.exec();
            qDebug() << "insert server" << cnt << hi.name;
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "convertConfToSqlite" << err;
    }
    return true;
}

bool QWoSshConf::databaseValid(const QString &dbFile)
{
    try {
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        SQLite::Statement health(db, "PRAGMA quick_check");
        if(health.executeStep()) {
            QByteArrayList dbset = {"identities", "servers"};
            for(auto it = dbset.begin(); it != dbset.end(); it++) {
                QByteArray name = *it;
                if(!db.tableExists(name)) {
                    return false;
                }
            }
            return true;
        }
    } catch (...) {
        qWarning() << "databaseValid" << dbFile;
    }
    return false;
}

QList<GroupInfo> QWoSshConf::groupList() const
{
    return m_groups;
}

QStringList QWoSshConf::groupNameList() const
{
    QStringList names;
    for(auto it = m_groups.begin(); it != m_groups.end(); it++) {
        const GroupInfo &gi = *it;
        names.append(gi.name);
    }
    return names;
}

QStringList QWoSshConf::tableList() const
{
    return {"servers","groups","identities"};
}

bool QWoSshConf::renameGroup(const QString &nameNew, const QString &nameOld)
{
    if(_renameGroup(nameNew, nameOld)) {
        for(auto it = m_groups.begin(); it != m_groups.end(); it++) {
            GroupInfo& gi = *it;
            if(nameOld == gi.name) {
                gi.name = nameNew;
                resetLater();
                return true;
            }
        }
    }
    return false;
}

bool QWoSshConf::updateGroup(const QString &name, int order)
{
    if(_updateGroup(name, order)) {
        for(auto it = m_groups.begin(); it != m_groups.end(); it++) {
            GroupInfo& gi = *it;
            if(name == gi.name) {
                m_groups.erase(it);
                break;
            }
        }
        GroupInfo gi;
        gi.name = name;
        gi.order = order;
        m_groups.append(gi);
        std::sort(m_groups.begin(), m_groups.end());
        resetLater();
        return true;
    }
    return false;
}

bool QWoSshConf::removeGroup(const QString &name)
{
    if(_removeGroup(name)) {
        for(auto it = m_groups.begin(); it != m_groups.end(); it++) {
            GroupInfo& gi = *it;
            if(name == gi.name) {
                m_groups.erase(it);
                resetLater();
                return true;
            }
        }
    }
    return true;
}

bool QWoSshConf::_renameGroup(const QString &nameNew, const QString &nameOld)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("groups")) {
            if(!createGroupsTable(db)) {
                return false;
            }
        }
        SQLite::Statement update(db, "UPDATE groups SET name=@nameNew WHERE name=@nameOld");
        update.reset();
        update.bind("@nameNew", nameNew.toStdString());
        update.bind("@nameOld", nameOld.toStdString());
        int cnt = update.exec();
        qDebug() << "renameGroup" << nameOld << nameNew << cnt;
        return cnt > 0;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::renameGroup" << e.what();
    }
    return false;
}

bool QWoSshConf::_updateGroup(const QString &name, int order)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("groups")) {
            if(!createGroupsTable(db)) {
                return false;
            }
        }
        SQLite::Statement update(db, "UPDATE groups SET orderNum=@orderNum WHERE name=@name");
        update.reset();
        update.bind("@name", name.toStdString());
        update.bind("@orderNum", order);
        int cnt = update.exec();
        if(cnt == 1) {
            qDebug() << "update group" << name << cnt;
            return true;
        }
        QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
        SQLite::Statement insert(db, "INSERT INTO groups (name,orderNum,ct) VALUES (@name,@orderNum,@ct)");
        insert.reset();
        insert.bind("@name", name.toStdString());
        insert.bind("@orderNum", order);
        insert.bind("@ct", now.toStdString());
        cnt = insert.exec();
        qDebug() << "insert group" << name << cnt;
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::updateGroup" << e.what();
    }
    return false;
}

bool QWoSshConf::_removeGroup(const QString &name)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("groups")) {
            if(!createGroupsTable(db)) {
                return false;
            }
        }
        SQLite::Statement task(db, QString("DELETE FROM groups WHERE name='%1'").arg(name).toUtf8());
        int cnt = task.exec();
        qDebug() << "delete" << name << cnt;
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::removeGroup" << e.what();
    }
    return false;
}

void QWoSshConf::resetLater()
{
    if(m_timer == nullptr) {
        m_timer = new QTimer(this);
        QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onResetLater()));
        m_timer->setSingleShot(true);
    }
    m_timer->start(100);
}

bool QWoSshConf::backup(const QString &dbBackup)
{
    try {
        if(!QFile::exists(m_dbFile)) {
            return false;
        }
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READONLY);
        SQLite::Statement health(db, "PRAGMA quick_check");
        if(!health.executeStep()) {
            return false;
        }
        QFile::remove(dbBackup);
        int err = db.backup(dbBackup.toUtf8(), db.Save);
        return err == SQLite::OK;
    } catch (...) {
        qWarning() << "QWoSshConf::backup" << m_dbFile;
    }
    return false;
}

bool QWoSshConf::refresh()
{
    init();
    m_hosts = loadServerFromSqlite(m_dbFile);
    m_groups = loadGroupFromSqlite(m_dbFile);
    QStringList adds;
    for(auto it = m_hosts.begin(); it != m_hosts.end(); it++) {
        const HostInfo& hi = it.value();
        bool find = false;
        for(auto gt = m_groups.begin(); gt != m_groups.end(); gt++) {
            const GroupInfo& gi = *gt;
            if(hi.group == gi.name) {
                find = true;
                break;
            }
        }
        if(!find) {
            if(!adds.contains(hi.group)) {
                adds.append(hi.group);
            }
        }
    }
    for(int i = 0; i < adds.length(); i++) {
        QString name = adds.at(i);
        if(!name.isEmpty()){
            GroupInfo gi;
            _updateGroup(name, 0);
            gi.name = name;
            gi.order = -1;
            m_groups.append(gi);
        }
    }
    return true;
}

bool QWoSshConf::exportToFile(const QString &path)
{
    QByteArray buf = toStream();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)){
        return false;
    }
    file.write(buf);
    return true;
}

bool QWoSshConf::removeServer(const QString &name)
{
    if(!m_hosts.contains(name)) {
        return false;
    }
    m_hosts.remove(name);
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("identities")) {
            return false;
        }
        SQLite::Statement insert(db, QString("DELETE FROM servers WHERE name='%1'").arg(name).toUtf8());
        int cnt = insert.exec();
        qDebug() << "remove server" << name << cnt;
        resetLater();
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::remove" << e.what();
    }
    return false;
}

bool QWoSshConf::removeServerByGroup(const QString &name)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("identities")) {
            return false;
        }
        SQLite::Statement insert(db, QString("DELETE FROM servers WHERE groupName='%1'").arg(name).toUtf8());
        int cnt = insert.exec();
        qDebug() << "remove server" << name << cnt;
        resetLater();
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::removeAllByGroup" << e.what();
    }
    return false;
}

bool QWoSshConf::renameServerGroup(const QString &nameNew, const QString &nameOld)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("servers")) {
            return false;
        }
        SQLite::Statement update(db, "UPDATE servers SET groupName=@nameNew WHERE groupName=@nameOld");
        update.reset();
        update.bind("@nameNew", nameNew.toStdString());
        update.bind("@nameOld", nameOld.toStdString());
        int cnt = update.exec();
        qDebug() << "renameServerGroup" << nameOld << nameNew << cnt;
        resetLater();
        return cnt > 0;
    }catch(std::exception& e) {
        qDebug() << "QWoSshConf::renameServerGroup" << e.what();
    }
    return false;
}

bool QWoSshConf::modify(const HostInfo &hi)
{
    m_hosts.insert(hi.name, hi);
    resetLater();
    return save(hi);
}

bool QWoSshConf::append(const HostInfo &hi)
{
    if(m_hosts.contains(hi.name)) {
        return false;
    }
    m_hosts.insert(hi.name, hi);
    resetLater();
    return save(hi);
}

bool QWoSshConf::modifyOrAppend(const HostInfo &hi)
{
    m_hosts.insert(hi.name, hi);
    resetLater();
    return save(hi);
}

void QWoSshConf::resetAllProperty(const QString &v)
{
    try{
        SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("servers")) {
            return;
        }
        SQLite::Statement update(db, "UPDATE servers SET property=@property");
        for(QHash<QString, HostInfo>::iterator iter = m_hosts.begin(); iter != m_hosts.end(); iter++) {
            HostInfo& hi = iter.value();
            if(hi.property.isEmpty()) {
                continue;
            }
            hi.property = v;
            update.reset();
            update.bind("@property", hi.property.toStdString());
            update.exec();
        }
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "convertConfToSqlite" << err;
    }
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
        save(hi);
    }
}

QList<HostInfo> QWoSshConf::hostList(EHostType type) const
{
    QList<HostInfo> hosts = m_hosts.values();
    std::sort(hosts.begin(), hosts.end());
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
    std::sort(hosts.begin(), hosts.end());
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

void QWoSshConf::onAboutToQuit()
{
    backup(m_dbFile + ".bak");
}

void QWoSshConf::onResetLater()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    timer->stop();
    emit dataReset();
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
