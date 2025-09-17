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

#include "qwoidentify.h"
#include "qwosetting.h"
#include "qwoutils.h"

#include <QEventLoop>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QCoreApplication>
#include <QTimer>
#include <QDateTime>

#include <SQLiteCpp/SQLiteCpp.h>

#include <libssh/libssh.h>

QWoIdentify::QWoIdentify(QObject *parent)
    : QObject(parent)
{

}

bool QWoIdentify::infomationByPrivateKey(const QString &file, IdentifyInfo *pinfo)
{
    QFile f(file);
    if(!f.open(QFile::ReadOnly)) {
        return false;
    }
    QByteArray buf = f.readAll();
    QByteArray rsa = QWoUtils::fromWotermStream(buf);
    return infomationByPrivateKey(rsa, pinfo);
}

bool QWoIdentify::infomationByPrivateKey(const QByteArray &rsa, IdentifyInfo *pinfo)
{
    ssh_key key = nullptr;
    int err = ssh_pki_import_privkey_base64(rsa.data(), nullptr, nullptr, nullptr, &key);
    if(err != SSH_OK) {
        return false;
    }

    ssh_key pkey = nullptr;
    err = ssh_pki_export_privkey_to_pubkey(key, &pkey);
    if(err != SSH_OK) {
        ssh_key_free(key);
        return false;
    }
    QString type = "unknown";
    ssh_keytypes_e keyType=ssh_key_type(key);
    switch(keyType) {
    case SSH_KEYTYPE_RSA:
    case SSH_KEYTYPE_RSA1:
    case SSH_KEYTYPE_RSA_CERT01:
        type = "rsa";
        break;
    case SSH_KEYTYPE_ED25519:
    case SSH_KEYTYPE_ED25519_CERT01:
        type = "ed25519";
        break;
    case SSH_KEYTYPE_DSS:
    case SSH_KEYTYPE_DSS_CERT01:
        type = "dss";
        break;
    case SSH_KEYTYPE_ECDSA:
    case SSH_KEYTYPE_ECDSA_P256:
    case SSH_KEYTYPE_ECDSA_P384:
    case SSH_KEYTYPE_ECDSA_P521:
    case SSH_KEYTYPE_ECDSA_P256_CERT01:
    case SSH_KEYTYPE_ECDSA_P384_CERT01:
    case SSH_KEYTYPE_ECDSA_P521_CERT01:
        type = "ecdsa";
        break;
    case SSH_KEYTYPE_UNKNOWN:
        type = "unknown";
        break;
    }
    char* b64;
    if(ssh_pki_export_pubkey_base64(pkey, &b64) != SSH_OK) {
        ssh_key_free(pkey);
        ssh_key_free(key);
        return false;
    }
    unsigned char *hash;
    size_t len = 0;
    if(ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &len) != SSH_OK) {
        ssh_key_free(pkey);
        ssh_key_free(key);
        return false;
    }
    pinfo->type = type;
    pinfo->prvKey = rsa;
    pinfo->pubKey = QByteArray(b64);
    pinfo->fingureprint = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256, hash, len);
    ssh_clean_pubkey_hash(&hash);
    ssh_key_free(pkey);
    ssh_key_free(key);
    return true;
}

bool QWoIdentify::isPublicKey(const QString &fileName)
{
    ssh_key key = nullptr;
    int err = ssh_pki_import_pubkey_file(fileName.toUtf8(), &key);
    if(err != SSH_OK) {
        return false;
    }
    err = ssh_key_is_public(key);
    ssh_key_free(key);
    return err == 1;
}

bool QWoIdentify::isPrivateKey(const QString &fileName)
{
    ssh_key key = nullptr;
    int err = ssh_pki_import_privkey_file(fileName.toUtf8(), nullptr, nullptr, nullptr,  &key);
    if(err != SSH_OK) {
        return false;
    }
    err = ssh_key_is_private(key);
    ssh_key_free(key);
    return err == 1;
}

bool QWoIdentify::import(const QString &fileName, IdentifyInfo *pinfo)
{
    return infomationByPrivateKey(fileName, pinfo);
}

bool QWoIdentify::create(const QString &name)
{
    ssh_key key = nullptr;
    int err = ssh_pki_generate(SSH_KEYTYPE_RSA, 1024, &key);
    if(err != SSH_OK) {
        return false;
    }
    QString file = QWoSetting::identityFilePath() + "/" + QWoUtils::nameToPath(name);
    QByteArray path = file.toLocal8Bit();
    err = ssh_pki_export_privkey_file(key, nullptr, nullptr, nullptr, path);
    ssh_key_free(key);
    return err == SSH_OK;
}

bool QWoIdentify::create(const QString &name, const QByteArray &prvKey)
{
    QString dbFile = QWoSetting::sshServerDbPath();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("identities")) {
            return false;
        }
        QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
        SQLite::Statement insert(db, "INSERT INTO identities (name,prvKey,ct) VALUES (@name,@prvKey,@ct)");
        insert.bind("@name", name.toStdString());
        insert.bind("@prvKey", prvKey.toStdString());
        insert.bind("@ct", now.toStdString());
        int cnt = insert.exec();
        qDebug() << "add identities" << name << cnt;
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoIdentify::create" << e.what();
    }
    return false;
}

bool QWoIdentify::infomation(const QByteArray &name, IdentifyInfo *pinfo)
{
    QString dbFile = QWoSetting::sshServerDbPath();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        if(!db.tableExists("identities")) {
            return false;
        }
        SQLite::Statement query(db, "select * from identities where delFlag=0 and where name=@name");
        query.bind("@name", name.toStdString());
        while(query.executeStep()) {
            IdentifyInfo info;
            QByteArray rsa = QByteArray::fromStdString(query.getColumn("prvKey").getString());
            if(!infomationByPrivateKey(rsa, &info)) {
                continue;
            }
            info.name = QString::fromStdString(query.getColumn("name").getString());
            *pinfo = info;
            return true;
        }
    }catch(std::exception& e) {
        qDebug() << "Identify-information" << e.what();
    }
    return false;
}

bool QWoIdentify::rename(const QString &nameOld, const QString &nameNew)
{
    QString dbFile = QWoSetting::sshServerDbPath();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("identities")) {
            return false;
        }
        QString now = QDateTime::currentDateTime().toString("yyyy/dd/MM hh:mm:ss");
        SQLite::Statement insert(db, QString("UPDATE identities SET name=@name WHERE name='%1'").arg(nameOld).toUtf8());
        insert.bind("@name", nameNew.toStdString());
        int cnt = insert.exec();
        qDebug() << "update identities" << nameOld << nameNew << cnt;
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoIdentify::rename" << e.what();
    }
    return false;
}

bool QWoIdentify::remove(const QString &name)
{
    QString dbFile = QWoSetting::sshServerDbPath();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        if(!db.tableExists("identities")) {
            return false;
        }
        SQLite::Statement insert(db, QString("DELETE FROM identities WHERE name='%1'").arg(name).toUtf8());
        int cnt = insert.exec();
        qDebug() << "remove identity" << name << cnt;
        return true;
    }catch(std::exception& e) {
        qDebug() << "QWoIdentify::remove" << e.what();
    }
    return false;
}

QMap<QString, IdentifyInfo> QWoIdentify::loadFromSqlite()
{
    QMap<QString, IdentifyInfo> all;
    QString dbFile = QWoSetting::sshServerDbPath();
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        if(!db.tableExists("identities")) {
            return all;
        }
        SQLite::Statement query(db, "select * from identities where delFlag=0");
        while(query.executeStep()) {
            IdentifyInfo info;
            QByteArray rsa = QByteArray::fromStdString(query.getColumn("prvKey").getString());
            if(!infomationByPrivateKey(rsa, &info)) {
                continue;
            }
            info.name = QString::fromStdString(query.getColumn("name").getString());
            all.insert(info.name, info);
        }
    }catch(std::exception& e) {
        qDebug() << "Identify-loadFromSqlite" << e.what();
    }

    return all;
}

QMap<QString, IdentifyInfo> QWoIdentify::loadFromFile()
{
    QDir dir(QWoSetting::identityFilePath());
    QStringList items = dir.entryList(QDir::Files);
    QMap<QString, IdentifyInfo> all;
    for(int i = 0; i < items.length(); i++) {
        IdentifyInfo info;
        if(!infomationByPrivateKey(dir.path() + "/" + items.at(i), &info)) {
            continue;
        }
        info.name = QWoUtils::pathToName(items.at(i));
        all.insert(info.fingureprint, info);
    }
    return all;
}

QStringList QWoIdentify::qmlFileNames()
{
    QMap<QString, IdentifyInfo> identities = loadFromSqlite();
    return identities.keys();
}

QVariantList QWoIdentify::qmlLoadFromSqlite()
{
    QVariantList lsv;
    QMap<QString, IdentifyInfo> identities = loadFromSqlite();
    for(auto it = identities.begin(); it != identities.end(); it++) {
        const IdentifyInfo& info = *it;
        QVariantMap dm;
        dm.insert("name", info.name);
        dm.insert("figureprint", info.fingureprint);
        dm.insert("prvKey", QString(info.prvKey));
        dm.insert("pubKey", QString(info.pubKey));
        dm.insert("type", info.type);
        lsv.append(dm);
    }
    return lsv;
}

QVariant QWoIdentify::qmlImport(const QString &file, const QString& nameSave)
{
    IdentifyInfo info;
    if(!QWoIdentify::import(file, &info)) {
        return tr("the identify's file is bad");
    }
    QFile f(file);
    f.open(QFile::ReadOnly);
    QByteArray buf = f.readAll();
    QByteArray data = QWoUtils::fromWotermStream(buf);
    f.close();
    if(QWoIdentify::create(nameSave, info.prvKey)) {
        return true;
    }
    return tr("The name already exists. Please enter a new name.");
}
