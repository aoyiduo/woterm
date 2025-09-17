/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qmoaboutassist.h"
#include "qkxhttpclient.h"
#include "version.h"
#include "qwosetting.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

#define UPGRADE_FILE_SAVE_PATH      ("woterm_upgrade")
QMoAboutAssist::QMoAboutAssist(QObject *parent)
    : QObject(parent)
{
    // version check.
    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if(path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if(path.isEmpty()) {
            path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
    }
    QDir dir(path);
    if(!dir.exists(UPGRADE_FILE_SAVE_PATH)) {
        dir.mkdir(UPGRADE_FILE_SAVE_PATH);
    }
    if(dir.exists(UPGRADE_FILE_SAVE_PATH)) {
        m_pathSave = QString("%1/%2").arg(path).arg(UPGRADE_FILE_SAVE_PATH);
    }else{
        m_pathSave = path;
    }
    {
        // clean old apk package.
        QDir dir(m_pathSave);
        QStringList filters;
        filters << "*.apk";
        QFileInfoList apks = dir.entryInfoList(filters);
        for(auto it = apks.begin(); it != apks.end(); it++) {
            QFileInfo fi = *it;
            QString absPath = fi.absoluteFilePath();
            QFile::remove(absPath);
        }
    }
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QMoAboutAssist::~QMoAboutAssist()
{
    if(m_http) {
        m_http->deleteLater();
    }
}

QString QMoAboutAssist::currentVersion() const
{
    return WOTERM_VERSION;
}

QString QMoAboutAssist::latestVersion() const
{
    return m_latestVersion;
}

void QMoAboutAssist::upgradeVersion()
{
    findTargetFileName();
}

void QMoAboutAssist::stopUpgrade()
{
    if(m_http) {
        m_http->deleteLater();
    }
}

void QMoAboutAssist::checkLatestVersion()
{
    QKxHttpClient *http = new QKxHttpClient(this);
    m_http = http;
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionCheck(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get("http://down.woterm.com/.mver");
}

void QMoAboutAssist::onVersionCheck(int code, const QByteArray &body)
{
    if(code != 200) {
        return;
    }
    QString verBody(body.trimmed());
    if(body[0] == 'v') {
        verBody = verBody.mid(1);
    }
    m_latestVersion = verBody;
    emit latestVersionArrived();
}

void QMoAboutAssist::onFileNameGet(int code, const QByteArray &body)
{
    static const QString platform = "-android-";
    static const QString pathDown = "http://down.woterm.com/android/";

    QByteArrayList lines = body.split('\n');
    for(auto it = lines.begin(); it != lines.end(); it++) {
        QString line = *it;
        line = line.simplified();
        if(line.toLower().contains(platform)) {
            QString file = pathDown + "/" + line;
            QString msg = tr("ready to download file:") + line;
            emit upgradeMessageArrived(msg);
            downloadFile(file);
            return;
        }
    }

#ifdef QT_DEBUG
    {
        QString msg = "ready to download file:woterm-android-test.apk";
        emit upgradeMessageArrived(msg);
        QString file = "http://down.aoyiduo.com/other/woterm-android-test.apk";
        downloadFile(file);
        return;
    }
#endif

    QString msg = tr("No suitable upgrade version found.");
    emit upgradeMessageArrived(msg);
}

void QMoAboutAssist::onDownloadFinished(int code, const QByteArray &body)
{
    if(code == 200) {
        QKxHttpClient *http = qobject_cast<QKxHttpClient*>(sender());
        m_http = http;
        QString fileSave = http->fileSavePath();
        emit upgradeMessageArrived(tr("Ready to install..."));
        emit installArrived(fileSave);
    }else{
        emit upgradeMessageArrived(tr("Failed to upgrade version, try it again later."));
    }
}

void QMoAboutAssist::findTargetFileName()
{
    QKxHttpClient *http = new QKxHttpClient(this);
    m_http = http;
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onFileNameGet(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    QObject::connect(http, SIGNAL(finished()), this, SIGNAL(finished()));
    QObject::connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
    http->get("http://down.woterm.com/.softver");
    emit upgradeMessageArrived(tr("try to find target filename."));
}

void QMoAboutAssist::downloadFile(const QString &file)
{
    QUrl url(file);
    QString fileName = url.fileName();
    QKxHttpClient *http = new QKxHttpClient(this);
    m_http = http;
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onDownloadFinished(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    QObject::connect(http, SIGNAL(finished()), this, SIGNAL(finished()));
    QObject::connect(http, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));

    http->fileGet(file, m_pathSave + "/" + fileName);
    emit upgradeMessageArrived(tr("download filename: %1").arg(fileName));
}

void QMoAboutAssist::init()
{
#ifdef Q_OS_ANDROID
    {
        QStringList permission;
        permission.append("android.permission.READ_EXTERNAL_STORAGE");
        permission.append("android.permission.WRITE_EXTERNAL_STORAGE");
        permission.append("android.permission.INSTALL_PACKAGES");
        permission.append("android.permission.REQUEST_INSTALL_PACKAGES");
        permission.append("android.permission.RESTART_PACKAGES");
        QtAndroid::requestPermissionsSync(permission);
    }
#endif
    checkLatestVersion();
}
