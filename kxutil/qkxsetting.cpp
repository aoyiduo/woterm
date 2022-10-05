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

#include "qkxsetting.h"

#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QTranslator>
#include <QTcpServer>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QCryptographicHash>
#include <QCoreApplication>

#define APPLICATION_DATA_PATH ("APP_DATA_PATH")

Q_GLOBAL_STATIC_WITH_ARGS(QSettings, gSettings, (QKxSetting::applicationConfigPath(), QSettings::IniFormat))

void QKxSetting::setValue(const QString &key, const QVariant &v)
{
    gSettings->setValue(key, v);
    gSettings->sync();
}

QVariant QKxSetting::value(const QString &key, const QVariant &defval)
{
    qDebug() << "fileName" << gSettings->fileName();
    return gSettings->value(key, defval);
}

void QKxSetting::remove(const QString &key)
{
    gSettings->remove(key);
}

void QKxSetting::sync()
{
    gSettings->sync();
}

QString QKxSetting::applicationName()
{
    static QString name;
    if(name.isEmpty()) {
        QString tmp = applicationFilePath();
        QStringList dps = tmp.split('/');
        name = dps.last();
        if(name.endsWith(".exe")) {
            int idx = name.lastIndexOf(".exe");
            name = name.left(idx);
        }
    }
    return name;
}

QString QKxSetting::applicationDirPath()
{
    static QString path;
    if(path.isEmpty()) {
        QString tmp = applicationFilePath();
        QStringList dps = tmp.split('/');
        dps.pop_back();
        path = dps.join('/');
    }
    return path;
}

#if defined (Q_OS_WIN)
#include <windows.h>
QString QKxSetting::applicationFilePath()
{
    static QString path;
    if(path.isEmpty()) {
        char buffer[1024] = {0};
        ::GetModuleFileNameA(0, buffer, 1023);
        path = buffer;
    }
    path = QDir::cleanPath(path);
    return path;
}
#elif defined (Q_OS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
# include <objc/runtime.h>
# include <objc/message.h>
#else
# include <objc/objc-runtime.h>
#endif
QString QKxSetting::applicationFilePath()
{
    static QString path;
    if(!path.isEmpty()) {
        return path;
    }

    CFURLRef bundleURL(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    if(bundleURL) {
        CFStringRef cfPath(CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle));
        if(cfPath){
            path = QString::fromCFString(cfPath);
        }
    }

    if(path.isEmpty()) {
        char tmp[1024] = {0};
        int cnt = 1024;
        if(_NSGetExecutablePath(tmp, (uint32_t*)&cnt) == 0) {
            path = QString::fromLatin1(tmp);
        }
    }
    if(path.isEmpty()) {
        path = QCoreApplication::applicationFilePath();
    }
    path = QDir::cleanPath(path);
    return path;
}
#else
QString QKxSetting::applicationFilePath()
{
    static QString path;
    if(path.isEmpty()) {
        char szFile[128] = {0};
        sprintf(szFile, "/proc/self/cmdline");
        FILE *f = fopen(szFile, "r");
        if(f == nullptr) {
            return "";
        }
        QByteArray cmd;
        cmd.resize(1024);
        int n = fread(cmd.data(), 1, 1023, f);
        cmd.resize(n);
        QByteArrayList args = cmd.split(' ');
        QFileInfo fi(args[0]);
        QString tmp = fi.absoluteFilePath();
        path = QDir::cleanPath(tmp);
    }
    return path;
}
#endif

void QKxSetting::updateStartTime()
{
    QDateTime t = QDateTime::currentDateTime();
    QString tm = t.toString(Qt::ISODate);
    QKxSetting::setValue("startTime", tm);
}

QByteArray QKxSetting::uuid()
{
    QString path = QKxSetting::applicationDataPath();
    path.append("/uid");
    QDir d(path);
    path = d.absoluteFilePath(path);
    QFile f(path);
    if(f.exists()) {
        if(f.open(QFile::ReadOnly)) {
            QByteArray line = f.readLine(100);
            QUuid uid = QUuid::fromRfc4122(QByteArray::fromBase64(line));
            if(!uid.isNull()) {
                return line;
            }
        }
        f.remove();
    }
    if(f.open(QFile::WriteOnly)) {
        QUuid uid = QUuid::createUuid();
        QByteArray id = uid.toRfc4122().toBase64(QByteArray::OmitTrailingEquals);
        f.write(id);
        return id;
    }
    return "";
}

QString QKxSetting::ensurePath(const QString &id)
{
    QString path = QKxSetting::applicationDataPath() + "/" + id;
    QFileInfo fi(path);
    if(fi.exists()) {
        if(!fi.isDir()) {
            if(!fi.isFile()) {
                return QString("");
            }
            if(!QFile::remove(path)) {
                return QString("");
            }
            QDir d;
            d.mkpath(path);
            return path;
        }
    }else{
        QDir d;
        d.mkpath(path);
    }
    return path;
}

QString QKxSetting::applicationConfigPath()
{
    QString appName = applicationName();
    static QString path = QDir::cleanPath(QString("%1/%2.ini").arg(QKxSetting::applicationDataPath()).arg(appName));
    return path;
}

QString QKxSetting::logFilePath()
{
    return specialFilePath("log");
}

QString QKxSetting::applicationDataPath()
{
    static QString userDataPath;
    QString appName = applicationName();

    if(userDataPath.isEmpty()){
        QByteArray wap = qgetenv(APPLICATION_DATA_PATH);
        if(wap.isEmpty()){
            QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            if(path.isEmpty()) {
                path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                if(path.isEmpty()) {
                    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                }
            }
            if(!path.isEmpty()) {
                QString dataPath;
                if(path.endsWith(appName)) {
                    dataPath = path;
                }else if(path.at(0) == '/'){
                    dataPath = path + "/"+appName+"/";
                }else {
                    dataPath = path + "/" + appName+"/";
                }
                QString dataPathOld = path + QString("/.%1").arg(appName);
                QDir d(dataPathOld);
                if(d.exists()) {
                    QStringList files = d.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
                    for(int i = 0; i < files.length(); i++) {
                        d.rename(files.at(i), dataPath + QString("/%1").arg(files.at(i)));
                    }
                    QFile f(dataPathOld);
                    if(!f.remove()) {
                        qInfo() << QString("Failed to delete directory:%1").arg(dataPathOld) << f.errorString();
                    }
                }
                userDataPath = QDir::cleanPath(dataPath);
            }
        }else{
            userDataPath = wap;
        }
        QFileInfo fi(userDataPath);
        if(!fi.exists() || !fi.isDir()) {
            QDir dir;
            dir.rmpath(userDataPath);
            dir.mkpath(userDataPath);
        }
        QProcess::execute(QString("chmod -R a+rw %1").arg(userDataPath));
        qputenv(APPLICATION_DATA_PATH, userDataPath.toUtf8());
    }
    return userDataPath;
}

QString QKxSetting::specialFilePath(const QString &name)
{
    QString path = QDir::cleanPath(applicationDataPath() + "/" + name);
    QFileInfo fi(path);
    if(!fi.exists() || !fi.isDir()) {
        QDir dir;
        dir.rmpath(path);
        dir.mkpath(path);
    }
    return path;
}
