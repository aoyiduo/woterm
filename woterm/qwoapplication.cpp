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
#include "qwotunneldialog.h"
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
#include <QMenu>
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
#include <QProcess>

QWoApplication::QWoApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setWindowIcon(QIcon(":/woterm/resource/images/woterm2.png"));

    QString path = applicationDirPath();
    addLibraryPath(path);
    QString libPath = QDir::cleanPath(path + "/../lib");
    if(QFile::exists(libPath)) {
        addLibraryPath(libPath);
    }

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QWoApplication *QWoApplication::instance()
{
    return qobject_cast<QWoApplication*>(QCoreApplication::instance());
}

QWoMainApplication::QWoMainApplication(int &argc, char **argv)
    : QWoApplication(argc, argv)
{
    qmlRegisterType<QKxScriptRemoteCommand>("RemoteCommand", 1,0, "RemoteCommand");
    qmlRegisterType<QKxScriptLocalCommand>("LocalCommand", 1,0, "LocalCommand");
    qmlRegisterType<QKxScriptFileTransferCommand>("FileTransferCommand", 1,0, "FileTransferCommand");
    qmlRegisterType<QKxScriptFileListCommand>("FileListCommand", 1,0, "FileListCommand");
    qmlRegisterType<QKxScriptFileContentCommand>("FileContentCommand", 1,0, "FileContentCommand");
    qmlRegisterType<QKxScriptBigScriptCommand>("BigScriptCommand", 1,0, "BigScriptCommand");
    qmlRegisterType<QKxDirAssist>("LocalDir", 1,0, "LocalDir");
    qmlRegisterType<QKxFileAssist>("LocalFile", 1,0, "LocalFile");
}

QWidget *QWoMainApplication::mainWindow()
{
    return m_main;
}

void QWoMainApplication::init()
{
    m_main = new QWoMainWindow();
    m_main->show();
    QMetaObject::invokeMethod(m_main, "onAppStart", Qt::QueuedConnection);
}


QWoTunnelApplication::QWoTunnelApplication(int &argc, char **argv)
    : QWoApplication(argc, argv)
{
    setQuitOnLastWindowClosed(false);
    QObject::connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
}

QWidget *QWoTunnelApplication::mainWindow()
{
    return m_main;
}

void QWoTunnelApplication::init()
{
    QMenu *menu = new QMenu();
    menu->addAction(QIcon(":/woterm/resource/images/tunnel.png"), QObject::tr("Show tunnel window"), this, SLOT(onShowWindow()));
    menu->addAction(QIcon(":/woterm/resource/images/woterm2.png"), QObject::tr("New session"), this, SLOT(onNewSessionWindow()));
    menu->addAction(QIcon(":/woterm/resource/images/exit.png"), QObject::tr("Exit"), this, SLOT(onAboutToQuit()));
    m_menu = menu;

    m_tray.setIcon(QIcon(":/woterm/resource/images/tunnel.png"));
    m_tray.setToolTip(QObject::tr("WoTerm tunnel daemon"));
    m_tray.setContextMenu(menu);
    m_tray.show();

    QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(onMenuAboutToShow()));
    QObject::connect(&m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayActive(QSystemTrayIcon::ActivationReason)));

    m_main = new QWoTunnelDialog();
    m_main->show();
}

void QWoTunnelApplication::onTrayActive(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick/*|| reason == QSystemTrayIcon::Context || reason == QSystemTrayIcon::Trigger*/) {
        onShowWindow();
    }
}

void QWoTunnelApplication::onMenuAboutToShow()
{

}

void QWoTunnelApplication::onShowWindow()
{
    if(m_main->isVisible()) {
        return;
    }
    m_main->show();
}

void QWoTunnelApplication::onNewSessionWindow()
{
    QString pathApp = QCoreApplication::applicationFilePath();
    QString cmd = QString("\"%1\"").arg(pathApp);
    QProcess::startDetached(cmd);
}

void QWoTunnelApplication::onAboutToQuit()
{
    m_tray.setVisible(false);
    QCoreApplication::quit();
}

void QWoTunnelApplication::onMessageReceived(const QString &msg)
{
    if(msg == "active") {
        onDelayActiveWindow();
        //QTimer::singleShot(1000, this, SLOT(onDelayActiveWindow()));
    }
}
#ifdef Q_OS_WIN
#include <Windows.h>
void QWoTunnelApplication::onDelayActiveWindow()
{
    onShowWindow();
    m_main->activateWindow();
    m_main->raise();
    WId wid = m_main->winId();
    ::SetForegroundWindow(HWND(wid));
}
#else
void QWoTunnelApplication::onDelayActiveWindow()
{
    onShowWindow();
    m_main->activateWindow();
    m_main->raise();
}
#endif
