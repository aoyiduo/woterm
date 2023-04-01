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

#include "qmoapplication.h"
#include "qwosetting.h"

#include "qmorecenthistorymodel.h"
#include "qmomainwindow.h"
#include "qmodbsftpdetailassist.h"
#include "qmosftptransferassist.h"
#include "qwodbsftpdownsync.h"
#include "qmodirassist.h"
#include "qmoaboutassist.h"

#include <QStyleFactory>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLibrary>
#include <QScreen>

#include <QQmlEngine>
#include <QJSEngine>
#include <QClipboard>
#include <QDateTime>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

QMoApplication::QMoApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setWindowIcon(QIcon(":/woterm/resource/skin/woterm4.png"));
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

    qmlRegisterType<QMoDbSftpDetailAssist>("MoDbSftpDetail", 1,0, "MoDbSftpDetail");
    qmlRegisterType<QWoDBSftpDownSync>("MoDBSftpDownSync", 1,0, "MoDBSftpDownSync");
    qmlRegisterType<QMoDirAssist>("MoDir", 1,0, "MoDir");
    qmlRegisterType<QMoAboutAssist>("MoAbout", 1,0, "MoAbout");
}

QMoApplication *QMoApplication::instance()
{
    return qobject_cast<QMoApplication*>(QCoreApplication::instance());
}

QMoMainWindow *QMoApplication::mainWindow()
{
    return QMoApplication::instance()->m_main;
}

qint64 QMoApplication::elapse()
{
    qint64 tmStart = QMoApplication::instance()->m_timeStart;
    qint64 tmNow = QDateTime::currentSecsSinceEpoch();
    return tmNow - tmStart;
}

void QMoApplication::qmlCopyText(const QString &txt)
{
    QGuiApplication::clipboard()->setText(txt);
}

void QMoApplication::init()
{
    m_main = new QMoMainWindow(nullptr);
#if defined (Q_OS_ANDROID)
    m_main->showMaximized();
    QtAndroid::hideSplashScreen(500);
#else
    m_main->resize(360, 520);
    QScreen * screen = primaryScreen();
    QRect rt = screen->geometry();
    m_main->move((rt.width() - 360) / 2, (rt.height() - 520) / 2);
    m_main->show();
#endif
    QMetaObject::invokeMethod(m_main, "onAppStart", Qt::QueuedConnection);
}
