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

#include "qwomainwindow.h"
#include "qwosetting.h"
#include "qwoshower.h"
#include "qwowidget.h"
#include "qwotermwidget.h"
#include "qwosessionlist.h"
#include "ui_qwomainwindow.h"
#include "qwosessionmanage.h"
#include "qwoaboutdialog.h"
#include "qwoidentifydialog.h"
#include "version.h"
#include "qkxhttpclient.h"
#include "qwoutils.h"
#include "qwosettingdialog.h"
#include "qwosshconf.h"
#include "qwosessionproperty.h"
#include "qwosessionmoreproperty.h"

#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QTabBar>
#include <QToolBar>
#include <QPushButton>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

QWoMainWindow::QWoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QWoMainWindow)
{
    ui->setupUi(this);
    setMinimumSize(QSize(1024, 700));

    setAttribute(Qt::WA_DeleteOnClose);

    setContentsMargins(3,3,3,3);
    setWindowTitle("WoTerm");

    initMenuBar();
    initToolBar();
    initStatusBar();
    //QMenu *actionsMenu = new QMenu("Actions", ui->menuBar);
    //ui->menuBar->addMenu(actionsMenu);
    //actionsMenu->addAction("Find...", this, SLOT(toggleShowSearchBar()), QKeySequence(Qt::CTRL +  Qt::Key_F));
    //actionsMenu->addAction("About Qt", this, SLOT(aboutQt()));

    m_sessionDock = new QDockWidget("Session Manager", this);
    m_sessionDock->setObjectName("Session Manager");
    m_sessionDock->setFloating(false);
    m_sessionDock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable);
    m_sessionDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);    
    addDockWidget(Qt::LeftDockWidgetArea, m_sessionDock);
    m_sessions = new QWoSessionList(m_sessionDock);
    m_sessionDock->setWidget(m_sessions);
    m_sessionDock->setVisible(false);
    //m_sessionDock->setFixedWidth(150);

    QWoWidget *central = new QWoWidget(this);
    setCentralWidget(central);

    m_tab = new QTabBar(this);
    m_tab->setMovable(true);
    m_tab->setTabsClosable(true);
    m_tab->setExpanding(false);
    m_tab->setUsesScrollButtons(true);
    m_shower = new QWoShower(m_tab, this);

    QObject::connect(m_shower, SIGNAL(tabEmpty()), this, SLOT(onShouldAppExit()));
    QObject::connect(m_shower, SIGNAL(openSessionManage()), this, SLOT(onActionOpenTriggered()));

    QVBoxLayout *layout = new QVBoxLayout(central);
    central->setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(m_tab);
    layout->addWidget(m_shower);

    QObject::connect(m_sessions, SIGNAL(readyToConnect(const QString&, int)), this, SLOT(onSessionReadyToConnect(const QString&, int)));

    restoreLastState();
}

QWoMainWindow::~QWoMainWindow()
{
    delete ui;
}

QWoMainWindow *QWoMainWindow::instance()
{
    static QPointer<QWoMainWindow> main = new QWoMainWindow();
    return main;
}

QWoShower *QWoMainWindow::shower()
{
    return QWoMainWindow::instance()->m_shower;
}

void QWoMainWindow::closeEvent(QCloseEvent *event)
{
    saveLastState();

    QMessageBox::StandardButton btn = QMessageBox::warning(this, "exit", "Exit Or Not?", QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        event->setAccepted(false);
        return ;
    }

    QMainWindow::closeEvent(event);
}

void QWoMainWindow::onNewTerm()
{
    QWoSessionProperty dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(const QString&, int)), this, SLOT(onSessionReadyToConnect(const QString&,int)));
    dlg.exec();
    QWoHostListModel::instance()->refreshList();
}

void QWoMainWindow::onOpenTerm()
{
    QWoSessionManage dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(const QString&,int)), this, SLOT(onSessionReadyToConnect(const QString&,int)));
    dlg.exec();
}

void QWoMainWindow::onLayout()
{
    m_sessionDock->setVisible(!m_sessionDock->isVisible());
}

void QWoMainWindow::onEditConfig()
{
    QString cfg = QDir::cleanPath(QApplication::applicationDirPath() + "/../");
    QDesktopServices::openUrl(QUrl(cfg, QUrl::TolerantMode));
}

void QWoMainWindow::onSessionReadyToConnect(const QString &target, int type)
{
    switch (type) {
    case EOT_SSH:
        m_shower->openSsh(target);
        break;
    case EOT_SFTP:
        m_shower->openSftp(target);
        break;
    case EOT_TELNET:
        m_shower->openTelnet(target);
        break;
    case EOT_RLOGIN:
        m_shower->openRLogin(target);
        break;
    case EOT_MSTSC:
        m_shower->openMstsc(target);
        break;
    case EOT_VNC:
        m_shower->openVnc(target);
        break;
    case EOT_SERIALPORT:
        m_shower->openSerial(target);
        break;
    default:
        break;
    }
}

void QWoMainWindow::onAppStart()
{
    {
        QKxHttpClient *http = new QKxHttpClient(this);
        QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
        http->get("https://hm.baidu.com/hm.js?bbffebc017090c1957c90f7deca2582e");
    }
    {
        // version check.
        QKxHttpClient *http = new QKxHttpClient(this);
        QObject::connect(http, SIGNAL(result(int,const QByteArray&)), this, SLOT(onVersionCheck(int,const QByteArray&)));
        QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
        http->get("http://down.aoyiduo.com/woterm/.ver");

        //QObject::connect(m_httpClient, SIGNAL(finished()), m_httpClient, SLOT(deleteLater()));
    }
    {
        //check target
        QStringList argvs = QApplication::arguments();
        argvs.takeFirst();
        QString target;
        QString type;
        for(int i = 0; i < argvs.count(); i++){
            QString argv = argvs.at(i);
            if(argv.startsWith("--target=")){
                target = argv.mid(9);
            }else if(argv.startsWith("--type=")) {
                type = argv.mid(7);
            }
        }
        if(!target.isEmpty()){
            HostInfo hi = QWoSshConf::instance()->find(target);
            if(hi.isValid()){
                switch(hi.type) {
                case SshWithSftp:
                    if(type == "ssh") {
                        m_shower->openSsh(target);
                    }else{
                        m_shower->openSftp(target);
                    }
                    break;
                case SftpOnly:
                    m_shower->openSftp(target);
                    break;
                case Telnet:
                    m_shower->openTelnet(target);
                    break;
                case RLogin:
                    m_shower->openRLogin(target);
                    break;
                case SerialPort:
                    m_shower->openSerial(target);
                    break;
                case Mstsc:
                    m_shower->openMstsc(target);
                    break;
                case Vnc:
                    m_shower->openVnc(target);
                    break;
                }
            }
        }
    }
}

void QWoMainWindow::onVersionCheck(int code, const QByteArray &body)
{
    qDebug() << code << body;
    if(code == 200) {

    }
}

void QWoMainWindow::onShouldAppExit()
{
    if(m_shower->tabCount()) {
        return;
    }
    QApplication::exit();
}

void QWoMainWindow::onActionNewTriggered()
{
    onNewTerm();
}

void QWoMainWindow::onActionOpenTriggered()
{
    onOpenTerm();
}

void QWoMainWindow::onActionDisconnectTriggered()
{

}

void QWoMainWindow::onActionReconnectTriggered()
{

}

void QWoMainWindow::onActionReconnectAllTriggered()
{

}

void QWoMainWindow::onActionImportTriggered()
{

}

void QWoMainWindow::onActionExportTriggered()
{

}

void QWoMainWindow::onActionSaveTriggered()
{

}

void QWoMainWindow::onActionTransferTriggered()
{

}

void QWoMainWindow::onActionLogTriggered()
{

}

void QWoMainWindow::onActionExitTriggered()
{

}

void QWoMainWindow::onActionProxyTriggered()
{

}

void QWoMainWindow::onActionConfigDefaultTriggered()
{
    QWoSessionMoreProperty dlg(this);
    dlg.exec();
    QString lang = dlg.language();
    if(!lang.isEmpty()) {
        QString langNow = QWoSetting::languageFile();
        if(lang != langNow) {
            QWoSetting::setLanguageFile(lang);
            QMessageBox::warning(this, tr("Language"), tr("The Language has been changed, Please restart application to take effect."), QMessageBox::Ok);
        }
    }
}

void QWoMainWindow::onActionSettingTriggered()
{
    QWoSettingDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionFindTriggered()
{
    m_shower->openFindDialog();
}

void QWoMainWindow::onActionAboutTriggered()
{
    QWoAboutDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionHelpTriggered()
{
    QDesktopServices::openUrl(QUrl("http://aoyiduo.com/woterm/doc/"));
}

void QWoMainWindow::onActionScriptRunTriggered()
{
    m_shower->openScriptRuner("script");
}

void QWoMainWindow::onActionSshKeyManageTriggered()
{
    QWoIdentifyDialog::open(true, this);
}

void QWoMainWindow::initMenuBar()
{
    QObject::connect(ui->actionDisconect, SIGNAL(triggered()), this, SLOT(onActionDisconnectTriggered()));
    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onActionExitTriggered()));
    QObject::connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(onActionExportTriggered()));
    QObject::connect(ui->actionImport, SIGNAL(triggered()), this, SLOT(onActionImportTriggered()));
    QObject::connect(ui->actionLog, SIGNAL(triggered()), this, SLOT(onActionLogTriggered()));
    QObject::connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(onActionNewTriggered()));
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
    QObject::connect(ui->actionReconnect, SIGNAL(triggered()), this, SLOT(onActionReconnectTriggered()));
    QObject::connect(ui->actionReconnectAll, SIGNAL(triggered()), this, SLOT(onActionReconnectAllTriggered()));
    QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onActionSaveTriggered()));
    QObject::connect(ui->actionTransfer, SIGNAL(triggered()), this, SLOT(onActionTransferTriggered()));
    QObject::connect(ui->actionDefault, SIGNAL(triggered()), this, SLOT(onActionConfigDefaultTriggered()));
    QObject::connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(onActionFindTriggered()));
    setMenuBar(nullptr);
}

void QWoMainWindow::initToolBar()
{
    QToolBar *tool = ui->mainToolBar;
    tool->addAction(QIcon(":/woterm/resource/skin/add2.png"), tr("New"), this, SLOT(onNewTerm()));
    tool->addAction(QIcon(":/woterm/resource/skin/nodes.png"), tr("Manage"), this, SLOT(onOpenTerm()));
    tool->addAction(QIcon(":/woterm/resource/skin/layout.png"), tr("List"), this, SLOT(onLayout()));

//    QAction *import = tool->addAction(QIcon(":/woterm/resource/skin/import.png"), tr("Import"));
//    QObject::connect(import, SIGNAL(triggered()), this, SLOT(onActionImportTriggered()));

    tool->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Style"), this, SLOT(onActionConfigDefaultTriggered()));
    //tool->addAction(QIcon(":/woterm/resource/skin/js.png"), tr("Script"), this, SLOT(onActionScriptRunTriggered()));
    tool->addAction(QIcon(":/woterm/resource/skin/keyset.png"), tr("Keys"), this, SLOT(onActionSshKeyManageTriggered()));
    //tool->addAction(QIcon(":/woterm/resource/skin/setting.png"), tr("Setting"), this, SLOT(onActionSettingTriggered()));
    tool->addAction(QIcon(":/woterm/resource/skin/help.png"), tr("Help"), this, SLOT(onActionHelpTriggered()));
    tool->addAction(QIcon(":/woterm/resource/skin/about.png"), tr("About"), this, SLOT(onActionAboutTriggered()));
}

void QWoMainWindow::initStatusBar()
{
    //QStatusBar *bar = ui->statusBar;
    setStatusBar(nullptr);
}

void QWoMainWindow::restoreLastState()
{
    QByteArray geom = QWoSetting::value("mainwindow/geometry").toByteArray();
    if(!geom.isEmpty()) {
        restoreGeometry(geom);
    }

    QByteArray buf = QWoSetting::value("mainwindow/lastLayout").toByteArray();
    if(!buf.isEmpty()) {
        restoreState(buf);
    }

    m_sessionDock->setFloating(false);
}

void QWoMainWindow::saveLastState()
{
    QByteArray state = saveState();
    QWoSetting::setValue("mainwindow/lastLayout", state);
    QByteArray geom = saveGeometry();
    QWoSetting::setValue("mainwindow/geometry", geom);
}
