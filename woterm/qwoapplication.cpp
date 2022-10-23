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

#include <QStyleFactory>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>


QWoApplication::QWoApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setAttribute(Qt::AA_DontUseNativeMenuBar);
    setWindowIcon(QIcon(":/woterm/resource/skin/woterm4.png"));

    QStringList styles = QStyleFactory::keys();
    qDebug() << "embeded style list: " << styles;
    setStyle("fusion");

    QFile f(":/woterm/resource/qss/default.qss");
    f.open(QFile::ReadOnly);
    QByteArray qss = f.readAll();
    f.close();
    setStyleSheet(qss);


    QString path = applicationDirPath();
    addLibraryPath(path);
    QString libPath = QDir::cleanPath(path + "/../lib");
    if(QFile::exists(libPath)) {
        addLibraryPath(libPath);
    }

    QStringList libpaths = libraryPaths();
    qDebug() << libpaths;
}
