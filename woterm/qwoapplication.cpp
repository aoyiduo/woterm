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

#include "qwoapplication.h"
#include "qwosetting.h"
#include "qkxprocesslaunch.h"
#include "qwomainwindow.h"
#include "qkxmessagebox.h"
#include "qwotheme.h"

#include "qkxscriptremotecommand.h"
#include "qkxscriptfilecontentcommand.h"
#include "qkxscriptfiletransfercommand.h"
#include "qkxscriptlocalcommand.h"
#include "qkxscriptfilelistcommand.h"
#include "qkxscriptbigscriptcommand.h"
#include "qkxdirassist.h"
#include "qkxfileassist.h"

#include <QStyleFactory>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLibrary>
#include <QTimer>
#include <QQmlEngine>
#include <QJSEngine>
#include <QQuickStyle>

QWoApplication::QWoApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setWindowIcon(QIcon(":/woterm/resource/images/woterm2.png"));
    m_timeStart = QDateTime::currentSecsSinceEpoch();

    QString path = applicationDirPath();
    addLibraryPath(path);
    QString libPath = QDir::cleanPath(path + "/../lib");
    if(QFile::exists(libPath)) {
        addLibraryPath(libPath);
    }

    QStringList libpaths = libraryPaths();
    qDebug() << libpaths;
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);

    qmlRegisterType<QKxScriptRemoteCommand>("RemoteCommand", 1,0, "RemoteCommand");
    qmlRegisterType<QKxScriptLocalCommand>("LocalCommand", 1,0, "LocalCommand");
    qmlRegisterType<QKxScriptFileTransferCommand>("FileTransferCommand", 1,0, "FileTransferCommand");
    qmlRegisterType<QKxScriptFileListCommand>("FileListCommand", 1,0, "FileListCommand");
    qmlRegisterType<QKxScriptFileContentCommand>("FileContentCommand", 1,0, "FileContentCommand");
    qmlRegisterType<QKxScriptBigScriptCommand>("BigScriptCommand", 1,0, "BigScriptCommand");
    qmlRegisterType<QKxDirAssist>("LocalDir", 1,0, "LocalDir");
    qmlRegisterType<QKxFileAssist>("LocalFile", 1,0, "LocalFile");
}

QWoApplication *QWoApplication::instance()
{
    return qobject_cast<QWoApplication*>(QCoreApplication::instance());
}

QWoMainWindow *QWoApplication::mainWindow()
{
    return QWoApplication::instance()->m_main;
}

qint64 QWoApplication::elapse()
{
    qint64 tmStart = QWoApplication::instance()->m_timeStart;
    qint64 tmNow = QDateTime::currentSecsSinceEpoch();
    return tmNow - tmStart;
}

void QWoApplication::init()
{
    m_main = new QWoMainWindow();
    m_main->show();
    QMetaObject::invokeMethod(m_main, "onAppStart", Qt::QueuedConnection);
}
