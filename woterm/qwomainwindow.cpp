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
#include "qwoadmindialog.h"
#include "qwoidentifydialog.h"
#include "version.h"
#include "qkxhttpclient.h"
#include "qwoutils.h"
#include "qwosettingdialog.h"
#include "qwosshconf.h"
#include "qwohostlistmodel.h"
#include "qwohosttreemodel.h"
#include "qwosessionproperty.h"
#include "qwosessionmoreproperty.h"
#include "qwosshconf.h"
#include "qwodbrestoredialog.h"
#include "qkxprocesslaunch.h"
#include "qkxver.h"
#include "version.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
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
#include <QProcess>
#include <QInputDialog>

QWoMainWindow::QWoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QWoMainWindow)
{
    ui->setupUi(this);
    setMinimumSize(QSize(1024, 700));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("WoTerm"));

    initMenuBar();
    initToolBar();
    initStatusBar();

    m_sessionDock = new QDockWidget(tr("Session Manager"), this);
    m_sessionDock->setObjectName("Session Manager");
    m_sessionDock->setFloating(false);
    m_sessionDock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable);
    m_sessionDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);    
    addDockWidget(Qt::LeftDockWidgetArea, m_sessionDock);
    m_sessions = new QWoSessionList(m_sessionDock);
    m_sessionDock->setWidget(m_sessions);
    m_sessionDock->setVisible(false);

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

    QMessageBox::StandardButton btn = QMessageBox::warning(this, tr("Confirm"), tr("Exit Or Not?"), QMessageBox::Ok|QMessageBox::No);
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
        QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionCheck(int,QByteArray)));
        QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
        http->get("http://down.woterm.com/.ver");
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
    {
        if(!checkAdminLogin()) {
            QCoreApplication::quit();
        }
    }
}

void QWoMainWindow::onVersionCheck(int code, const QByteArray &body)
{
    if(body.isEmpty()) {
        return;
    }
    if(code == 200) {
        qDebug() << code << body;
        QString verBody = body.trimmed();
        if(body[0] == 'v') {
            verBody = verBody.mid(1);
        }
        int verLatest = QWoUtils::parseVersion(verBody);
        int verCurrent = QWoUtils::parseVersion(WOTERM_VERSION);
        if(verCurrent < verLatest) {
            bool pop = QWoSetting::shouldPopupUpgradeMessage(verBody);
            if(!pop) {
                return;
            }
            QWoSetting::setIgnoreTodayUpgrade(verBody);
            int ret = QMessageBox::question(this, tr("Version check"), tr("a new version of %1 is found, do you want to update it?").arg(verBody), QMessageBox::Yes|QMessageBox::No);
            if(ret == QMessageBox::Yes) {
                QDesktopServices::openUrl(QUrl("http://woterm.com"));
            }
        }
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

void QWoMainWindow::onActionBackupTriggered()
{
    QString path = QWoSetting::lastBackupPath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Session Database"), path, "SQLite3 (*.db *.bak)");
    qDebug() << "fileName" << fileName;
    if(fileName.isEmpty()) {
        return;
    }
    QFileInfo fi(fileName);
    QString last = fi.absolutePath();
    QWoSetting::setLastBackupPath(last);
    if(!QWoSshConf::instance()->backup(fileName)) {
        QMessageBox::warning(this, tr("Failure"), tr("failed to backup the session list."));
    }
}

void QWoMainWindow::onActionRestoreTriggered()
{
    QWoDBRestoreDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionExitTriggered()
{
    close();
}

void QWoMainWindow::onActionToolbarTriggered()
{
    ui->mainToolBar->setVisible(!ui->mainToolBar->isVisible());
}

void QWoMainWindow::onActionSessionListTriggered()
{
    onLayout();
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
            if(QMessageBox::warning(this, tr("Language"), tr("The language has been changed, restart application to take effect right now."), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
                QString path = QCoreApplication::instance()->applicationFilePath();                
                if(::QKxProcessLaunch::startDetached(path)) {
                    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
                }
            }
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
    QDesktopServices::openUrl(QUrl("http://woterm.com/doc/"));
}

void QWoMainWindow::onActionWebsiteTriggered()
{
    QDesktopServices::openUrl(QUrl("http://woterm.com/"));
}

void QWoMainWindow::onActionScriptRunTriggered()
{
    m_shower->openScriptRuner("script");
}

void QWoMainWindow::onActionSshKeyManageTriggered()
{
    QWoIdentifyDialog::open(true, this);
}

void QWoMainWindow::onActionAdminTriggered()
{
    if(!QWoUtils::isUltimateVersion(this)) {
        return;
    }
    QString pass = QWoSetting::adminPassword();
    QInputDialog input(this);
    input.setWindowFlags(input.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    input.setMinimumWidth(350);
    input.setWindowTitle(tr("Password input"));
    input.setLabelText(pass.isEmpty() ? tr("Login to the configuration of administrator for the first time.\r\nPlease input password to activate it.") : tr("Please input password to verify."));
    input.setTextEchoMode(QLineEdit::Password);
    int err = input.exec();
    if(err == 0) {
        return;
    }
    QString hitTxt = input.textValue();
    if(!pass.isEmpty()) {
        if(pass != hitTxt) {
            QMessageBox::information(this, tr("Password error"), tr("the password is not right."));
            return;
        }
    }else{
        QWoSetting::setAdminPassword(hitTxt.toUtf8());
    }
    QWoAdminDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::initMenuBar()
{
    ui->menuBar->setNativeMenuBar(false);
    QObject::connect(ui->actionSessionNew, SIGNAL(triggered()), this, SLOT(onActionNewTriggered()));
    QObject::connect(ui->actionSessionManage, SIGNAL(triggered()), this, SLOT(onActionOpenTriggered()));
    QObject::connect(ui->actionBackup, SIGNAL(triggered()), this, SLOT(onActionBackupTriggered()));
    QObject::connect(ui->actionRestore, SIGNAL(triggered()), this, SLOT(onActionRestoreTriggered()));
    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onActionExitTriggered()));
    QObject::connect(ui->actionToolBar, SIGNAL(triggered()), this, SLOT(onActionToolbarTriggered()));
    QObject::connect(ui->actionSessionList, SIGNAL(triggered()), this, SLOT(onActionSessionListTriggered()));
    QObject::connect(ui->actionOption, SIGNAL(triggered()), this, SLOT(onActionConfigDefaultTriggered()));
    QObject::connect(ui->actionIdentityManage, SIGNAL(triggered()), this, SLOT(onActionSshKeyManageTriggered()));
    QObject::connect(ui->actionDocument, SIGNAL(triggered()), this, SLOT(onActionHelpTriggered()));
    QObject::connect(ui->actionWetsite, SIGNAL(triggered()), this, SLOT(onActionWebsiteTriggered()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionAboutTriggered()));
    QObject::connect(ui->actionAdministrator, SIGNAL(triggered()), this, SLOT(onActionAdminTriggered()));
}

void QWoMainWindow::initToolBar()
{
    QToolBar *tool = ui->mainToolBar;
    tool->setWindowTitle(tr("Toolbar"));
    tool->addAction(QIcon(":/woterm/resource/skin/add2.png"), tr("New"), this, SLOT(onNewTerm()));
    tool->addAction(QIcon(":/woterm/resource/skin/nodes.png"), tr("Manage"), this, SLOT(onOpenTerm()));
    tool->addAction(QIcon(":/woterm/resource/skin/layout.png"), tr("List"), this, SLOT(onLayout()));

//    QAction *import = tool->addAction(QIcon(":/woterm/resource/skin/import.png"), tr("Import"));
//    QObject::connect(import, SIGNAL(triggered()), this, SLOT(onActionImportTriggered()));

    tool->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Option"), this, SLOT(onActionConfigDefaultTriggered()));
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

bool QWoMainWindow::checkAdminLogin()
{
    if(!QKxVer::isUltimate()) {
        return true;
    }
    QString pass = QWoSetting::adminPassword();
    if(pass.isEmpty()) {
        return true;
    }
    if(!QWoSetting::startupByAdmin()) {
        return true;
    }

    for(int i = 4; i >= 0; i--) {
        QInputDialog input(this);
        input.setWindowFlags(input.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        input.setMinimumWidth(350);
        input.setWindowTitle(tr("Administrator login"));
        input.setLabelText(tr("Please input password to login application."));
        input.setTextEchoMode(QLineEdit::Password);
        int err = input.exec();
        if(err == 0) {
            return false;
        }
        QString hitTxt = input.textValue();
        if(pass == hitTxt) {
            return true;
        }
        QMessageBox::information(this, tr("Login failure"), tr("The password is wrong, %1 times left to try.").arg(i));
    }
    return false;
}
