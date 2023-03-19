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

#include <QApplication>
#include <QtDebug>
#include <QIcon>
#include <QMenuBar>
#include <QProcess>
#include <QList>
#include <QByteArray>
#include <QStyleFactory>
#include <QSharedPointer>
#include <QPointer>
#include <QWeakPointer>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "qkxcipher.h"

#include "qwoglobal.h"
#include "qwoapplication.h"
#include "qwomainwindow.h"
#include "qwotermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwotermstyle.h"
#include "qwoutils.h"
#include "qwossh.h"
#include "qkxutils.h"
#include "qkxmessagebox.h"

#include "qmoapplication.h"
#include "qmomainwindow.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
      qDebug() << "Debug: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtInfoMsg:
      qDebug() << "Info: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtWarningMsg:
      qDebug() << "Warning: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtCriticalMsg:
      qDebug() << "Critical: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtFatalMsg:
      qDebug() << "Fatal: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  }
}

void test()
{

}


int main_pc(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageOutput);
    QGuiApplication::setApplicationName("woterm");
    QGuiApplication::setOrganizationName("aoyiduo");
    QGuiApplication::setOrganizationDomain("aoyiduo.com");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#ifdef Q_OS_WIN
    static QWoApplication app(argc, argv);
#else
    QWoApplication app(argc, argv);
#endif
    test();
    QStringList styles = QStyleFactory::keys();
    qDebug() << "embeded style list: " << styles;
    QApplication::setStyle("fusion");
    QFile f(":/woterm/resource/qss/desk_default.qss");
    f.open(QFile::ReadOnly);
    QByteArray qss = f.readAll();
    f.close();
    app.setStyleSheet(qss);    

    QTranslator translator;
    QString lang = QWoSetting::languageFile();
    if(!lang.isEmpty() && translator.load(lang)){
        app.installTranslator(&translator);
    }

    QString fontPath = QWoSetting::fontBackupPath();
    QKxUtils::setCustomFontPath(fontPath);
    QKxUtils::availableFontFamilies();

    return app.exec();
}

int main_qml(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageOutput);
#if defined (Q_OS_ANDROID) || defined (Q_OS_IOS)
    QGuiApplication::setApplicationName("woterm");
#else
    // prevent overwriting existing local configurations
    QGuiApplication::setApplicationName("moterm");
#endif
    QGuiApplication::setOrganizationName("aoyiduo");
    QGuiApplication::setOrganizationDomain("aoyiduo.com");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#ifdef Q_OS_WIN
    static QMoApplication app(argc, argv);
#else
    QMoApplication app(argc, argv);
#endif
    test();

    QTranslator translator;
    QString lang = QWoSetting::languageFile();
    if(!lang.isEmpty() && translator.load(lang)){
        app.installTranslator(&translator);
    }

    QString fontPath = QWoSetting::fontBackupPath();
    QKxUtils::setCustomFontPath(fontPath);
    QKxUtils::availableFontFamilies();

    QQuickStyle::setStyle("Material");
    QQuickStyle::setFallbackStyle("Material");

    QFile f(":/woterm/resource/qss/mobile_default.qss");
    f.open(QFile::ReadOnly);
    QByteArray qss = f.readAll();
    f.close();
    app.setStyleSheet(qss);
    return app.exec();
}

int main(int argc, char *argv[])
{
#if defined (Q_OS_ANDROID) || defined(QML_MODE)
    return main_qml(argc, argv);
#else
    return main_pc(argc, argv);
#endif
}
