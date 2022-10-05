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

#ifdef Q_OS_WIN
#include <Windows.h>
#include <tlhelp32.h>
#endif

QKxWallPaper::QKxWallPaper(QObject *parent)
    : QObject(parent)
    , m_status(ES_None)
{
    // clean();
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
    if(pidNow == pid || 1) { // let all process can do same thing.
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
    wchar_t tmp[255] = {0};
    if(::SystemParametersInfoW(SPI_GETDESKWALLPAPER, 254, tmp, 0)) {
        QString path = QString::fromWCharArray(tmp);
        if(!path.isEmpty()) {
            qint64 clr = desktopColor();
            if(::SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, L"", SPIF_UPDATEINIFILE)) {                
                setDesktopColor(0);
                settings->setValue("background", qint64(clr));
                settings->setValue("wallpaper", path);
                return true;
            }
        }
    }
    return false;
}

bool QKxWallPaper::restoreWallPaper(QSettings *settings)
{
    wchar_t tmp[255] = {0};
    if(::SystemParametersInfoW(SPI_GETDESKWALLPAPER, 254, tmp, 0)) {
        QString path = QString::fromWCharArray(tmp);
        if(path.isEmpty()) {            
            // wall pager
            QString path = settings->value("wallpaper").toString();
            if(path.isEmpty()) {
                return false;
            }
            if(::SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (LPWSTR)path.data(), SPIF_UPDATEINIFILE)){
                qint64 clr = settings->value("background").toLongLong();
                setDesktopColor(clr);                
                return true;
            }
        }
    }
    return false;
}

bool QKxWallPaper::cleanWindowDrag(QSettings *settings)
{
    int isDrag;
    if(::SystemParametersInfoW(SPI_GETDRAGFULLWINDOWS, 0, &isDrag, 0)) {
        //SPI_GETDRAGFULLWINDOWS
        if(isDrag) {
            SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS, FALSE, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            settings->setValue("windowDrag", isDrag);
            return true;
        }
    }
    return false;
}

bool QKxWallPaper::restoreWindowDrag(QSettings *settings)
{
    // drag show window content.
    SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS, TRUE, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    return true;
}

bool QKxWallPaper::isActiveProcess(int pid)
{
    try{
        HANDLE hdl = ::OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
        if(hdl != NULL) {
            DWORD code = 0;
            if(::GetExitCodeProcess(hdl, &code)) {
                if(code == STILL_ACTIVE) {
                    ::CloseHandle(hdl);
                    return true;
                }
            }
            ::CloseHandle(hdl);
        }
    }catch(...){
        qDebug() << "isActiveProcess" << pid;
    };
    return false;
}

qint64 QKxWallPaper::desktopColor()
{
    DWORD clr = GetSysColor(COLOR_BACKGROUND);
    return qint64(clr);
}

void QKxWallPaper::setDesktopColor(qint64 clr)
{
    int aElements[1] = {COLOR_BACKGROUND};
    DWORD aNewColors[1];
    aNewColors[0] = DWORD(clr);
    SetSysColors(1, aElements, aNewColors);
}
