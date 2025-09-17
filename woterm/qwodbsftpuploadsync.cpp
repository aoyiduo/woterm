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

#include "qwodbsftpuploadsync.h"
#include "qwossh.h"
#include "qwosshconf.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qkxcipher.h"


#include <QDir>
#include <QFile>
#include <QCryptographicHash>

QWoDBSftpUploadSync::QWoDBSftpUploadSync(QObject *parent)
    : QObject(parent)
{
    m_pathTemp = QWoSetting::tempPath();
}

QWoDBSftpUploadSync::~QWoDBSftpUploadSync()
{
    removeTempFile();
    release();
}

void QWoDBSftpUploadSync::upload(const QString &type, const QString &key)
{
    removeTempFile();

    QString fileClone = m_pathTemp + "/woterm_backup.db";
    runAction(DB_SFTP_UPLOAD_ENCRYPT_FILE, tr("ready to clone the db."));
    if(!QWoSshConf::instance()->backup(fileClone)) {
        finishAction(-1, tr("Failed to clone the backup file:%1.").arg(fileClone));
        return;
    }
    QFile bf(fileClone);
    if(!bf.open(QIODevice::ReadOnly)) {
        finishAction(-2, tr("Failed to read the backup file:%1.").arg(fileClone));
        return;
    }
    QByteArray all = bf.readAll();
    bf.close();

    QByteArray out;
    if(type == "AES-CBC-256") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 32);
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 16);
        if(!QKxCipher::aesCbcEncrypt(all, out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "AES-CTR-256") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 32);
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 16);
        if(!QKxCipher::aesCtrEncrypt(QKxCipher::alignBytes(all, 16), out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "AES-GCM-256") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 32);
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 16);
        if(!QKxCipher::aesGcmEncrypt(QKxCipher::alignBytes(all, 16), out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "DES-CBC") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 24);
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 8);
        if(!QKxCipher::tripleDesCbcEncrypt(all, out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "DES-ECB") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 24);
        if(!QKxCipher::tripleDesEcbEncrypt(all, out, pass, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "DES-OFB64") {
        QByteArray pass = QKxCipher::makeBytes(key.toUtf8(), 24);
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 8);
        if(!QKxCipher::tripleDesOfb64Encrypt(all, out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "RC4") {
        QByteArray pass = key.toUtf8();
        if(!QKxCipher::rc4Encrypt(all, out, pass, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }else if(type == "Blowfish") {
        QByteArray pass = key.toUtf8();
        QByteArray ivec = QKxCipher::makeBytes(key.toUtf8(), 8);
        if(!QKxCipher::blowfishEcbEncrypt(QKxCipher::alignBytes(all, 8), out, pass, ivec, true)) {
            finishAction(-3, tr("Failed to encrypt the backup file:%1.").arg(fileClone));
            return;
        }
    }

    QByteArray sha1 = QCryptographicHash::hash(out, QCryptographicHash::Sha1);
    QString hexFull = sha1.toHex();
    QString hex = hexFull.left(12);
    QDate dt = QDate::currentDate();
    m_nameCrypt = QString("woterm_%1%2%3_%4.db").arg(dt.year()).arg(dt.month(),2,10,QLatin1Char('0')).arg(dt.day(),2,10,QLatin1Char('0')).arg(hex);
    QString fileCrypt = m_pathTemp + "/" + m_nameCrypt;
    QFile uf(fileCrypt);
    if(!uf.open(QIODevice::WriteOnly)) {
        finishAction(-3, tr("Failed to write the encrypt file:%1.").arg(fileCrypt));
        return;
    }
    uf.write(out);
    uf.close();


    reconnect();

    runAction(DB_SFTP_UPLOAD_CHECK_UPLOAD_PATH, tr("Ready to check server upload path."));
    QVariantMap dm;
    dm.insert("pathUpload", m_pathUpload);
    m_sftp->fileInfo(m_pathUpload, dm);
}

void QWoDBSftpUploadSync::release()
{
    if(m_sftp) {
        m_sftp->abort();
        QWoSshFactory::instance()->release(m_sftp);
    }
}

void QWoDBSftpUploadSync::removeTempFile()
{
    if(!m_nameCrypt.isEmpty()) {
        QFile::remove(m_pathTemp + "/" + m_nameCrypt);
    }
}

void QWoDBSftpUploadSync::runAction(int action, const QString &tip)
{
    m_action = action;
    emit infoArrived(action, 0, tip);
}

void QWoDBSftpUploadSync::finishAction(int err, const QString &errDesc)
{
    emit infoArrived(m_action, err, errDesc);
    removeTempFile();
}

void QWoDBSftpUploadSync::onConnectionStart()
{

}

void QWoDBSftpUploadSync::onConnectionFinished(bool ok)
{
    if(!ok) {
        release();
    }
}

void QWoDBSftpUploadSync::onCommandStart(int type, const QVariantMap &userData)
{

}

void QWoDBSftpUploadSync::onCommandFinish(int type, const QVariantMap &userData)
{
    QString reason = userData.value("reason").toString();
    if(reason == "fatal") {
        finishAction(-9, tr("Failed to upload backup file."));
        return;
    }
    int code = userData.value("code").toInt();
    if(type == MT_FTP_FILE_INFO) {
        QString path = userData.value("pathUpload").toString();
        QVariantMap fi = userData.value("fileInfo").toMap();
        if(fi.isEmpty()) {
            if(code == 2) {
                finishAction(-1, tr("The path is not exist:%1").arg(path));
            }else{
                finishAction(-2, tr("Failed to check path:%1.").arg(path));
            }
        }else{
            QString fileRemote = m_pathUpload + "/" + ".ver";
            QVariantMap dm;
            dm.insert("remote", fileRemote);
            dm.insert("command", "fileContent");
            m_sftp->fileContent(fileRemote, 0, 512, dm);
            runAction(DB_SFTP_UPLOAD_CHECK_VERSION, tr("Ready to check version information."));
        }
    }else if(type == MT_FTP_UPLOAD) {
        QString file = userData.value("remote").toString();
        if(reason == "ok") {
            finishAction(0, tr("success to backup file:%1.").arg(file));
        }else{
            finishAction(-1, tr("Failed to upload the encrypt file:%1.").arg(file));
        }
    }else if(type == MT_FTP_READ_FILE_CONTENT) {
        QVariantMap file = userData.value("fileContent").toMap();
        if(file.isEmpty()) {
            runAction(DB_SFTP_UPLOAD_WRITE_VERSION, tr("Ready to write version information."));
            QString fileRemote = m_pathUpload + "/" + ".ver";
            m_sftp->writeFileContent(fileRemote, m_nameCrypt.toUtf8());            
        }else{
            QByteArray fileName = file.value("content").toByteArray();
            QByteArray hex = fileName.mid(fileName.lastIndexOf('_'));
            int idx = m_nameCrypt.lastIndexOf(hex);
            if(idx > 0) {
                // The server already has the same backup, check it's integrity
                QString fileLocal = m_pathTemp + "/" + m_nameCrypt;
                QString fileRemote = m_pathUpload + "/" + fileName;
                runAction(DB_SFTP_UPLOAD_UPLOAD_FILE, tr("Ready to upload the encrypt file."));
                QVariantMap dm;
                dm.insert("remote", fileName);
                m_sftp->upload(fileLocal, fileRemote, QWoSshFtp::TP_Append, dm);
            }else{
                runAction(DB_SFTP_UPLOAD_WRITE_VERSION, tr("Ready to write version information."));
                QString fileRemote = m_pathUpload + "/" + ".ver";
                m_sftp->writeFileContent(fileRemote, m_nameCrypt.toUtf8());
            }
        }
    }else if(type == MT_FTP_WRITE_FILE_CONTENT) {
        if(reason == "error") {
            finishAction(-1, tr("Failed to write version information."));
        }else{
            runAction(DB_SFTP_UPLOAD_UPLOAD_FILE, tr("Ready to upload the encrypt file."));
            QString fileLocal = m_pathTemp + "/" + m_nameCrypt;
            QString fileRemote = m_pathUpload + "/" + m_nameCrypt;
            QVariantMap dm;
            dm.insert("remote", m_nameCrypt);
            m_sftp->upload(fileLocal, fileRemote, QWoSshFtp::TP_Append, dm);
        }
    }
}

void QWoDBSftpUploadSync::onErrorArrived(const QString &err, const QVariantMap &userData)
{
    release();
}

void QWoDBSftpUploadSync::onFinishArrived(int code)
{
    release();
}

void QWoDBSftpUploadSync::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    release();
    finishAction(-9, tr("Failed to login remote server."));
}

void QWoDBSftpUploadSync::reconnect()
{
    release();
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));

    HostInfo hi;
    QVariantMap dm = QWoSetting::value("DBBackup/sftpDetail").toMap();
    hi.host = dm.value("host").toString();
    hi.user = dm.value("name").toString();
    hi.password = dm.value("password").toString();
    hi.identityFile = dm.value("identity").toString();
    hi.port = dm.value("port", 22).toInt();
    m_pathUpload = dm.value("path", "~/woterm_db_backup").toString();
    m_sftp->start(hi, QDateTime::currentSecsSinceEpoch());
}

