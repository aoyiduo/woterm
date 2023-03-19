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

#include <QStyleFactory>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLibrary>
#include <QTimer>


QWoApplication::QWoApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setWindowIcon(QIcon(":/woterm/resource/skin/woterm4.png"));

    QString path = applicationDirPath();
    addLibraryPath(path);
    QString libPath = QDir::cleanPath(path + "/../lib");
    if(QFile::exists(libPath)) {
        addLibraryPath(libPath);
    }

    QStringList libpaths = libraryPaths();
    qDebug() << libpaths;
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QWoApplication *QWoApplication::instance()
{
    return qobject_cast<QWoApplication*>(QCoreApplication::instance());
}

QWoMainWindow *QWoApplication::mainWindow()
{
    return QWoApplication::instance()->m_main;
}

void QWoApplication::init()
{
    m_main = new QWoMainWindow();
    m_main->show();
    QMetaObject::invokeMethod(m_main, "onAppStart", Qt::QueuedConnection);
}
