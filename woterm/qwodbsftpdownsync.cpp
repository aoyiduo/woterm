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

#include "qwodbsftpdownsync.h"
#include "qwossh.h"
#include "qwosshconf.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qkxcipher.h"
#include "qkxmessagebox.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUrl>

QWoDBSftpDownSync::QWoDBSftpDownSync(QObject *parent)
    : QObject(parent)
{
    m_pathTemp = QWoSetting::tempPath();
}

QWoDBSftpDownSync::~QWoDBSftpDownSync()
{

}

QString QWoDBSftpDownSync::downloadPath() const
{
    return m_pathTemp;
}

void QWoDBSftpDownSync::setDownloadPath(const QString &path)
{
    m_pathTemp = path;
}

void QWoDBSftpDownSync::listAll()
{
    if(m_sftp == nullptr || !m_sftp->hasRunning()) {
        reconnect();
    }
    m_sftp->openDir(m_pathUpload);
}

void QWoDBSftpDownSync::fetchLatest()
{
    if(m_sftp == nullptr || !m_sftp->hasRunning()) {
        reconnect();
    }
    QString fileName = m_pathUpload + "/" + ".ver";
    runAction(DB_SFTP_DOWNLOAD_CHECK_VERSION, tr("Ready to fetch the latest version information."));
    m_sftp->fileContent(fileName, 0, 512);
}

void QWoDBSftpDownSync::fetch(const QString &fileName)
{
    if(m_sftp == nullptr || !m_sftp->hasRunning()) {
        reconnect();
    }
    download(fileName);
}

QVariant QWoDBSftpDownSync::qmlDecryptFile(const QString &fileNameSrc, const QString &fileNameDst, const QString &cryptType, const QString &cryptKey)
{
    QString errMsg;
    if(!decryptFile(fileNameSrc, fileNameDst, cryptType, cryptKey, errMsg)) {
        return errMsg;
    }
    return true;
}

QVariant QWoDBSftpDownSync::qmlAbsolutePath(const QString &fileName)
{
    QString path = m_pathTemp + "/" + fileName;
    if(QFile::exists(path)) {
        return QUrl::fromLocalFile(path);
    }
    return false;
}

bool QWoDBSftpDownSync::decrypt(const QByteArray& in, const QByteArray &type, const QByteArray &key, QByteArray &out)
{
    if(type == "AES-CBC-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesCbcEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "AES-CTR-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesCtrEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "AES-GCM-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesGcmEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "DES-CBC") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::tripleDesCbcEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "DES-ECB") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        if(!QKxCipher::tripleDesEcbEncrypt(in, out, pass, false)) {
            return false;
        }
    }else if(type == "DES-OFB64") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::tripleDesOfb64Encrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "RC4") {
        QByteArray pass = key;
        if(!QKxCipher::rc4Encrypt(in, out, pass, false)) {
            return false;
        }
    }else if(type == "Blowfish") {
        QByteArray pass = key;
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::blowfishEcbEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }
    // strncmp(header, "SQLite format 3\000", 16)
    QByteArray header = out.left(16);
    if(!header.startsWith("SQLite format 3")) {
        return false;
    }
    return true;
}

bool QWoDBSftpDownSync::decryptFile(const QString &fileNameSrc, const QString &fileNameDst, const QString &cryptType, const QString &cryptKey, QString &errMsg)
{
    QString path = m_pathTemp + "/" + fileNameSrc;
    QFile lf(path);
    if(!lf.open(QIODevice::ReadOnly)) {
        errMsg = tr("Failed to open file:%1").arg(fileNameSrc);
        return false;
    }
    QByteArray all = lf.readAll();
    lf.close();
    QByteArray out;
    if(!decrypt(all, cryptType.toUtf8(), cryptKey.toUtf8(), out)) {
        errMsg = tr("Failed to decrypt the backup file:%1.").arg(fileNameSrc);
        return false;
    }
    qDebug() << "ready to do more";
    QString fileDecrypt = m_pathTemp + "/" + fileNameDst;
    QFile::remove(fileDecrypt);
    QFile df(fileDecrypt);
    if(!df.open(QIODevice::WriteOnly)) {
        errMsg = tr("Failed to write decrypt result to file:%1").arg(fileDecrypt);
        return false;
    }
    df.write(out);
    df.close();
    return true;
}

void QWoDBSftpDownSync::onConnectionStart()
{

}

void QWoDBSftpDownSync::onConnectionFinished(bool ok)
{
    if(!ok) {
        release();
    }
}

void QWoDBSftpDownSync::onCommandStart(int type, const QVariantMap &userData)
{

}

void QWoDBSftpDownSync::onCommandFinish(int type, const QVariantMap &userData)
{
    QString reason = userData.value("reason").toString();
    if(reason == "fatal") {
        finishAction(-9, tr("Failed to download the backup file."));
        return;
    }
    int code = userData.value("code").toInt();
    if(type == MT_FTP_READ_FILE_CONTENT) {
        QVariantMap file = userData.value("fileContent").toMap();
        if(file.isEmpty()) {
            if(code == 2) {
                finishAction(-1, tr("No relevant version information was found."));
            }else{
                finishAction(-2, tr("Unknow error was found."));
            }
        }else{
            QString fileName = file.value("content").toString();
            download(fileName);
        }
    }else if(type == MT_FTP_DOWNLOAD) {
        QString fileName = userData.value("fileName").toString();
        if(reason == "ok") {
            finishAction(0, tr("Success to download file:%1").arg(fileName));
            emit fetchFinished(0, fileName);
        }else{            
            finishAction(-1, tr("Failed to download file:%1").arg(fileName));
            emit fetchFinished(-1, fileName);
        }
    }else if(type == MT_FTP_LISTFILE) {
        if(reason == "ok") {
            finishAction(0, tr("Success to list file."));
        }else{
            finishAction(-1, tr("Failed to list file"));
        }
    }
}

void QWoDBSftpDownSync::onErrorArrived(const QString &err, const QVariantMap &userData)
{
    finishAction(-11, err);
    release();
}

void QWoDBSftpDownSync::onFinishArrived(int code)
{
    release();
    finishAction(-9, tr("Failed to login remote server."));
}

void QWoDBSftpDownSync::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    release();
    finishAction(-9, tr("Failed to login remote server."));
}

void QWoDBSftpDownSync::onDirOpen(const QString &path, const QVariantList &data, const QVariantMap &user)
{
    QVariantList lsfile = data;
    std::sort(lsfile.begin(), lsfile.end(),[](const QVariant& v1, const QVariant& v2){
        QVariantMap vm1 = v1.toMap();
        QVariantMap vm2 = v2.toMap();
        QDateTime t1 = QDateTime::fromString(vm1.value("date").toString(), "dd.MM.yyyy hh:mm:ss");
        QDateTime t2 = QDateTime::fromString(vm2.value("date").toString(), "dd.MM.yyyy hh:mm:ss");
        return t1 > t2;
    });
    QStringList fileNames;
    for(int i = 0; i < lsfile.length(); i++) {
        QMap<QString, QVariant> mdata = lsfile.at(i).toMap();
        QString name = mdata.value("name").toString();
        if(name == ".ver" || name == "." || name == "..") {
            continue;
        }
        fileNames.append(name);
    }
    emit listArrived(fileNames);
}

void QWoDBSftpDownSync::release()
{
    if(m_sftp) {
        m_sftp->abort();
        QWoSshFactory::instance()->release(m_sftp);
    }
}

void QWoDBSftpDownSync::reconnect()
{
    release();
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(dirOpen(QString,QVariantList,QVariantMap)), this, SLOT(onDirOpen(QString,QVariantList,QVariantMap)));

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

void QWoDBSftpDownSync::download(const QString &fileName)
{
    QString fileRemote = m_pathUpload + "/" + fileName;
    QString fileLocal = m_pathTemp + "/" + fileName;
    QVariantMap dm;
    dm.insert("remote", fileRemote);
    dm.insert("local", fileLocal);
    dm.insert("fileName", fileName);
    dm.insert("command", "download");
    runAction(DB_SFTP_DOWNLOAD_DOWNLOAD_FILE, tr("Ready to download file:%1").arg(QString(fileName)));
    m_sftp->download(fileRemote, fileLocal, QWoSshFtp::TP_Append, dm);

}

void QWoDBSftpDownSync::removeTempFile()
{

}

void QWoDBSftpDownSync::runAction(int action, const QString &tip)
{
    m_action = action;
    emit infoArrived(action, 0, tip);
}

void QWoDBSftpDownSync::finishAction(int err, const QString &errDesc)
{
    emit infoArrived(m_action, err, errDesc);
    removeTempFile();
}
