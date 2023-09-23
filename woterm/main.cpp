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
#include "qwotheme.h"
#include "qkxlocalpeer.h"

static QFile g_fileLog;
static QMutex g_mutexFileLog;

#ifdef Q_OS_WIN
#define WITH_SID        (false)
#else
#define WITH_SID        (true)
#endif

static void MyMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString & text)
{
    const QDateTime datetime = QDateTime::currentDateTime();
    const char * typeText = NULL;
    switch (type)
    {
    case QtDebugMsg:
    case QtInfoMsg:
        typeText = "Info";
        break;
    case QtWarningMsg:
        typeText = "Warning";
        break;
    case QtCriticalMsg:
        typeText = "Critical";
        break;
    case QtFatalMsg:
        typeText = "Fatal";
        break;
    }
    const QString finalText = QString("%1 %2 %3\n").arg(datetime.toString("yyyyMMdd/hh:mm:ss.zzz")).arg(typeText).arg(text);
    if (g_fileLog.isOpen())
    {
        QMutexLocker locker(&g_mutexFileLog);
        if (g_fileLog.size() == 0)
            g_fileLog.write("\xef\xbb\xbf");
        g_fileLog.write(finalText.toUtf8());
        g_fileLog.flush();
    }
}

void setDebugMessageToFile(const QString& name, bool tryDelete = false)
{
#ifdef QT_DEBUG
    return;
#endif
    QString path = QWoSetting::applicationDataPath();
    QString fullFile = path + "/" + name;
    qputenv("APP_LOG_PATH", fullFile.toUtf8());
    QFileInfo fi(fullFile);
    int fs = fi.size();
    if(fs > 1024 * 1024 || tryDelete) {
        QFile::remove(fullFile);
    }
    qInstallMessageHandler(MyMessageHandler);
    g_fileLog.setFileName(fullFile);
    g_fileLog.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void test()
{

}


int main_pc_tunnel(int argc, char *argv[])
{
#ifdef QT_DEBUG
    QGuiApplication::setApplicationName("wotermd");
#else
    QGuiApplication::setApplicationName("woterm");
#endif
    QGuiApplication::setOrganizationName("aoyiduo");
    QGuiApplication::setOrganizationDomain("aoyiduo.com");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QGuiApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#ifdef Q_OS_WIN
    static QWoTunnelApplication app(argc, argv);
#else
    QWoTunnelApplication app(argc, argv);
#endif

    if(!QWoSetting::tunnelRunAsDaemon()) {
        return 0;
    }

    if(app.isClient()) {
        app.sendMessage("active");
        return 0;
    }

    test();
    setDebugMessageToFile("tunnel.log", true);

    QWoTheme::instance();

    QTranslator translator;
    QString lang = QWoSetting::absoluteLanguageFilePath();
    if(!lang.isEmpty() && translator.load(lang)){
        app.installTranslator(&translator);
    }

    return app.exec();
}

int main_pc_main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    QGuiApplication::setApplicationName("wotermd");
#else
    QGuiApplication::setApplicationName("woterm");
#endif
    QGuiApplication::setOrganizationName("aoyiduo");
    QGuiApplication::setOrganizationDomain("aoyiduo.com");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QGuiApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#ifdef Q_OS_WIN
    static QWoMainApplication app(argc, argv);
#else
    QWoMainApplication app(argc, argv);
#endif

    test();
    setDebugMessageToFile("trace.log", true);

    QWoTheme::instance();

    QTranslator translator;
    QString lang = QWoSetting::absoluteLanguageFilePath();
    if(!lang.isEmpty() && translator.load(lang)){
        app.installTranslator(&translator);
    }

    QString fontPath = QWoSetting::fontBackupPath();
    QKxUtils::setCustomFontPath(fontPath);
    QKxUtils::availableFontFamilies();

    return app.exec();
}



int main_pc(int argc, char *argv[])
{
    for(int i = 0; i < argc; i++) {
        if(qstricmp(argv[i], "--tunnel") == 0) {
            return main_pc_tunnel(argc, argv);
        }
    }
    return main_pc_main(argc, argv);
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
    QGuiApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#ifdef Q_OS_WIN
    static QMoApplication app(argc, argv);
#else
    QMoApplication app(argc, argv);
#endif
    setDebugMessageToFile("trace.log", true);
    test();

    QTranslator translator;
    QString lang = QWoSetting::absoluteLanguageFilePath();
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
