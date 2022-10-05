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

#include "qkxwallpaper.h"

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>


QKxWallPaper::QKxWallPaper(QObject *parent)
    : QObject(parent)
    , m_status(ES_None)
{
    clean();
}

QKxWallPaper::~QKxWallPaper()
{
    restore();
}

void QKxWallPaper::disable(bool yes)
{
    if(yes) {
        clean();
    }else{        
        restore();
    }
}

void QKxWallPaper::invert()
{
    if(m_status == ES_Black) {
        restore();
    }else{
        clean();
    }
}

bool QKxWallPaper::_clean()
{
    QString path = QDir::tempPath() + "/" + "wallpaper.cfg";
    qDebug() << "wallpaper.cfg" << path;
    QSettings settings(path, QSettings::IniFormat, this);
    int pid = settings.value("processId").toInt();
    int pidNow = QCoreApplication::applicationPid();
    if(pidNow == pid) {
        return doClean(&settings);
    }
    if(pid > 0) {
        if(isActiveProcess(pid)) {
            return false;
        }
    }
    return doClean(&settings);
}

bool QKxWallPaper::doClean(QSettings *settings)
{
    bool bSync = false;
    if(cleanWallPaper(settings)) {
        settings->setValue("doWallPaper", 1);
        bSync = true;
    }
    if(cleanWindowDrag(settings)) {
        settings->setValue("doWindowDrag", 1);
        bSync = true;
    }

    if(bSync) {
        settings->setValue("processId", QCoreApplication::applicationPid());
        settings->sync();
        m_status = ES_Black;
    }
    return bSync;
}

bool QKxWallPaper::doRestore(QSettings *settings)
{
    bool bSync = false;
    if(settings->value("doWallPaper").toInt() == 1) {
        if(restoreWallPaper(settings)) {
            bSync = true;
        }
    }
    if(settings->value("doWindowDrag").toInt() == 1) {
        bSync = true;
    }
    restoreWindowDrag(settings);
    if(bSync) {
        QString iniFile = settings->fileName();
        QFile::remove(iniFile);
        m_status = ES_WallPaper;
    }
    return bSync;
}

bool QKxWallPaper::restore()
{
    try{
        return _restore();
    }catch(...){
        qDebug() << "restore";
    }
    return false;
}

bool QKxWallPaper::clean()
{
    try{
        return _clean();
    }catch(...){
        qDebug() << "clean";
    }
    return false;
}

bool QKxWallPaper::_restore()
{
    QString path = QDir::tempPath() + "/" + "wallpaper.cfg";
    qDebug() << "wallpaper.cfg" << path;
    QSettings settings(path, QSettings::IniFormat, this);
    int pid = settings.value("processId").toInt();
    int pidNow = QCoreApplication::applicationPid();
    if(pidNow == pid) {
        return doRestore(&settings);
    }
    if(pid > 0) {
        if(isActiveProcess(pid)) {
            return false;
        }
        return doRestore(&settings);
    }
    return false;
}

bool QKxWallPaper::cleanWallPaper(QSettings *settings)
{
    return false;
}

bool QKxWallPaper::restoreWallPaper(QSettings *settings)
{
    return false;
}

bool QKxWallPaper::cleanWindowDrag(QSettings *settings)
{

    return false;
}

bool QKxWallPaper::restoreWindowDrag(QSettings *settings)
{
    // drag show window content.    
    return true;
}

bool QKxWallPaper::isActiveProcess(int pid)
{
    return false;
}

qint64 QKxWallPaper::desktopColor()
{
    return qint64(0);
}

void QKxWallPaper::setDesktopColor(qint64 clr)
{
}
