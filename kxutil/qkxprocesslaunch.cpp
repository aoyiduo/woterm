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

#include "qkxprocesslaunch.h"
#include <QProcess>
#include <QDir>
#include <QLibraryInfo>
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
bool QKxProcessLaunch::startDetached(const QString &program, const QStringList &arguments)
{
    QString path = QDir::toNativeSeparators(program);
    QString args = arguments.join(' ');
    HINSTANCE hinst = ::ShellExecuteA(NULL, "open", path.toUtf8(), args.toUtf8(), NULL, SW_SHOWDEFAULT);
    return (int)hinst > 32;
}
#else
bool QKxProcessLaunch::startDetached(const QString &program, const QStringList &arguments)
{
    initQtLibrary();
    return QProcess::startDetached(program, arguments);
}

void QKxProcessLaunch::initQtLibrary()
{
    QByteArray libPath = qgetenv("LD_LIBRARY_PATH");
    QByteArrayList paths = libPath.split(':');
    {
        QStringList libPaths = QCoreApplication::libraryPaths();
        for(auto it = libPaths.begin(); it != libPaths.end(); it++) {
            QString _tmp = *it;
            QByteArray tmp = _tmp.toUtf8();
            if(!paths.contains(tmp)) {
                paths.append(tmp);
            }
        }
    }
    {
        QByteArray path = QLibraryInfo::location(QLibraryInfo::BinariesPath).toUtf8();
        if(!paths.contains(path)) {
            paths.append(path);
        }
    }
    {
        QByteArray path = QLibraryInfo::location(QLibraryInfo::LibrariesPath).toUtf8();
        if(!paths.contains(path)) {
            paths.append(path);
        }
    }
    {
        QByteArray path = QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath).toUtf8();
        if(!paths.contains(path)) {
            paths.append(path);
        }
    }
    {
        QByteArray path = QLibraryInfo::location(QLibraryInfo::PluginsPath).toUtf8();
        if(!paths.contains(path)) {
            paths.append(path);
        }
    }
    for(auto it = paths.begin(); it != paths.end();) {
        QByteArray tmp = *it;
        if(tmp.isEmpty()) {
            it = paths.erase(it);
        }else{
            it++;
        }
    }
    libPath = paths.join(':');
    qputenv("LD_LIBRARY_PATH", libPath);
    qInfo() << "LD_LIBRARY_PATH will be: " << paths;
}
#endif
