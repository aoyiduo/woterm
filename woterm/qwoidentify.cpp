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

#include "libssh/libssh.h"

bool QWoIdentify::infomation(const QString &file, IdentifyInfo *pinfo)
{
    QFile f(file);
    if(!f.open(QFile::ReadOnly)) {
        return false;
    }
    ssh_key key = nullptr;
    QByteArray buf = f.readAll();
    QByteArray rsa = QWoUtils::fromWotermStream(buf);
    int err = ssh_pki_import_privkey_base64(rsa.data(), nullptr, nullptr, nullptr, &key);
    if(err != SSH_OK) {
        return false;
    }
    QFileInfo fi(f);
    QString name = fi.fileName();
    QString fileAbs = QDir::cleanPath(file);
    if(fileAbs.startsWith(QWoSetting::identifyFilePath())) {
        name = QWoUtils::pathToName(name);
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
    if(ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_MD5, &hash, &len) != SSH_OK) {
        ssh_key_free(pkey);
        ssh_key_free(key);
        return false;
    }
    QByteArray base64(b64);    
    pinfo->name = name;
    pinfo->key = base64;
    pinfo->type = type;
    pinfo->path = file;
    pinfo->fingureprint = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_MD5, hash, len);
    ssh_clean_pubkey_hash(&hash);
    ssh_key_free(pkey);
    ssh_key_free(key);
    return true;
}

bool QWoIdentify::create(const QString &name)
{
    ssh_key key = nullptr;
    int err = ssh_pki_generate(SSH_KEYTYPE_RSA, 1024, &key);
    if(err != SSH_OK) {
        return false;
    }
    QString file = QWoSetting::identifyFilePath() + "/" + QWoUtils::nameToPath(name);
    QByteArray path = file.toLocal8Bit();
    err = ssh_pki_export_privkey_file(key, nullptr, nullptr, nullptr, path);
    ssh_key_free(key);
    return err == SSH_OK;
}

QMap<QString, IdentifyInfo> QWoIdentify::all()
{
    QDir dir(QWoSetting::identifyFilePath());
    QStringList items = dir.entryList(QDir::Files);
    QMap<QString, IdentifyInfo> all;
    for(int i = 0; i < items.length(); i++) {
        IdentifyInfo info;
        if(!infomation(dir.path() + "/" + items.at(i), &info)) {
            continue;
        }
        all.insert(info.fingureprint, info);
    }
    return all;
}
