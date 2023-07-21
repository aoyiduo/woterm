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
#include "qwosessionttyproperty.h"
#include "qwosessionrdpproperty.h"
#include "qwosessionvncproperty.h"
#include "qwosystemoptiondialog.h"
#include "qwosessiontoolconfiguredialog.h"
#include "qwosshconf.h"
#include "qwodbbackupdialog.h"
#include "qwodbrestoredialog.h"
#include "qwodbpowerrestoredialog.h"
#include "qkxprocesslaunch.h"
#include "qkxmessagebox.h"
#include "qwomenubutton.h"
#include "qworecenthistory.h"
#include "qwolicensedialog.h"
#include "qkxbuttonassist.h"
#include "qkxfilterlineedit.h"
#include "qwoversionupgradetipdialog.h"
#include "qwolicensetrialapplydialog.h"
#include "qwoplaybookmanagedialog.h"
#include "qwotunneldialog.h"
#include "qwotheme.h"
#include "qkxdockwidget.h"
#include "qkxver.h"
#include "version.h"
#include "qwotunnelserver.h"

#include "qwotermwidgetimpl.h"

#include <QApplication>
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
#include <QToolButton>
#include <QSslSocket>
#include <QSpacerItem>

QWoMainWindow::QWoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QWoMainWindow)
{
    ui->setupUi(this);
    setMinimumSize(QSize(800, 600));
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(QIcon(":/woterm/resource/images/woterm2.png"));

    QKxVer *ver = QKxVer::instance();
    QKxVer::ELicenseType type = ver->licenseType();
    QString typeVersion;
    if(type == QKxVer::ETrialVersion) {
        typeVersion = tr("Trial");
        if(ver->isExpired()) {
            typeVersion += tr("[expired]");
            typeVersion += " - ";
            typeVersion += tr("Please purchase the license to support us");
        }
        typeVersion += tr("");
    }else if(type == QKxVer::EUltimateVersion) {
        typeVersion = tr("Ultimate");
        if(ver->isExpired()) {
            typeVersion += tr("[expired]");
            typeVersion += " - ";
            typeVersion += tr("Please purchase the license to support us");
        }
    } else {
        typeVersion = tr("Free");
        typeVersion += " - ";
        typeVersion += tr("Please purchase the license to support us");
    }
    setWindowTitle(tr("WoTerm") + " " + typeVersion);

    initMenuBar();
    initToolBar();
    initStatusBar();

    m_recent = new QWoRecentHistory(this);
    QObject::connect(m_recent, SIGNAL(readyToConnect(QString,int)), this, SLOT(onSessionReadyToConnect(QString,int)));

    m_sessionDock = new QKxDockWidget(tr("Session Manager"), this);
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

    QObject::connect(m_tab, SIGNAL(currentChanged(int)), this, SLOT(onTabCurrentChanged(int)));


    m_shower = new QWoShower(m_tab, this);

    QObject::connect(m_shower, SIGNAL(tabEmpty()), this, SLOT(onShouldAppExit()));
    QObject::connect(m_shower, SIGNAL(openSessionManage()), this, SLOT(onActionOpenRemoteTriggered()));

    QVBoxLayout *layout = new QVBoxLayout(central);
    central->setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    setContentsMargins(0,0,0,0);

    layout->addWidget(m_tab);
    layout->addWidget(m_shower);

    QObject::connect(m_sessions, SIGNAL(readyToConnect(QString,int)), this, SLOT(onSessionReadyToConnect(QString,int)));


    restoreLastState();
}

QWoMainWindow::~QWoMainWindow()
{
    delete ui;
    if(m_dlgTunnel) {
        delete m_dlgTunnel;
    }
}

QWoMainWindow *QWoMainWindow::instance()
{
    return qobject_cast<QWoMainWindow*>(QWoApplication::instance()->mainWindow());
}

QWoShower *QWoMainWindow::shower()
{
    return QWoMainWindow::instance()->m_shower;
}

void QWoMainWindow::updateHistory(const QString &target, int type)
{
    m_recent->update(target, type);
}

void QWoMainWindow::closeEvent(QCloseEvent *event)
{
    saveLastState();

    QMessageBox::StandardButton btn = QKxMessageBox::warning(this, tr("Confirm"), tr("Exit Or Not?"), QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        event->setAccepted(false);
        return ;
    }

    QMainWindow::closeEvent(event);
}


void QWoMainWindow::onNewSession()
{
    QWoSessionProperty dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(QString,int)), this, SLOT(onSessionReadyToConnect(QString,int)));
    int retVal = dlg.exec();
    if(retVal == QWoSessionProperty::Save || retVal == QWoSessionProperty::Connect) {
        QWoHostListModel::instance()->refreshList();
    }
}

void QWoMainWindow::onJavascriptRunner()
{

}

void QWoMainWindow::onOpenRemoteSession()
{
    QWoSessionManage dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(QString,int)), this, SLOT(onSessionReadyToConnect(QString,int)));
    int retVal = dlg.exec();
    if(retVal == QWoSessionProperty::Save || retVal == QWoSessionProperty::Connect) {
        QWoHostListModel::instance()->refreshList();
    }
}

void QWoMainWindow::onOpenLocalSession()
{
    m_shower->openLocalShell();
}

void QWoMainWindow::onOpenSerialPort()
{
    m_shower->openSerialPort();
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
    default:
        break;
    }
}

void QWoMainWindow::onAppStart()
{
    QWoVersionUpgradeTipDialog::check(this, true);
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
                    m_shower->openSerialPort();
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
    if(QKxVer::instance()->isFullFeather()) {
        QKxVer *ver = QKxVer::instance();
        if(true || QWoSetting::shouldReportLicense()) { // make sure to check invalid every time.
            QWoSetting::setIgnoreTodayReportLicense();
            QKxHttpClient *http = new QKxHttpClient(this);
            QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onReportResult(int,QByteArray)));
            QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
            QJsonObject obj;
            obj.insert("mid", ver->machineID());
            obj.insert("info", ver->reportInformation());
            QJsonDocument doc;
            doc.setObject(obj);
            QByteArray json = doc.toJson(QJsonDocument::Compact);
            http->post("http://key.woterm.com/report", json, "application/json; charset=utf-8");
        }
    }else{
        if(QWoSetting::shouldPopupUpgradeUltimate()) {
            QWoSetting::setIgnoreTodayUpgradeUltimate();
            QMessageBox dlg(QMessageBox::Information, tr("Upgrade to ultimate version"), tr("The current version is free. It is recommended to upgrade to the ultimate version."), QMessageBox::Retry|QMessageBox::Yes|QMessageBox::No, this);
            {
                QAbstractButton *btn = dlg.button(QMessageBox::Retry);
                btn->setText(tr("Evalute ultimate"));
            }
            {
                QAbstractButton *btn = dlg.button(QMessageBox::No);
                btn->setText(tr("No"));
            }
            {
                QAbstractButton *btn = dlg.button(QMessageBox::Yes);
                btn->setText(tr("Yes"));
            }
            int ret = dlg.exec();
            if(ret == QMessageBox::Yes) {
                QMetaObject::invokeMethod(this, "onActionLicenseTriggered", Qt::QueuedConnection);
            }else if(ret == QMessageBox::Retry) {
                QMetaObject::invokeMethod(this, "tryToMakeLicenseTrial", Qt::QueuedConnection);
            }
        }
    }

    if(QWoSetting::localTerminalOnAppStart()) {
        QMetaObject::invokeMethod(this, "onOpenLocalSession", Qt::QueuedConnection);
    }
}


void QWoMainWindow::onShouldAppExit()
{
    if(m_shower->tabCount()) {
        return;
    }
    QApplication::exit();
}

void QWoMainWindow::onButtonAssistClicked(QToolButton *btn)
{
    QRect rt = btn->rect();
    QPoint pt = btn->mapToGlobal(rt.bottomLeft());
    QMenu menu(this);
    menu.addAction(QIcon("../private/skins/black/nodes.png"), tr("Open remote session"), this, SLOT(onOpenRemoteSession()));
    menu.addAction(QIcon("../private/skins/black/console.png"), tr("Open local session"), this, SLOT(onOpenLocalSession()));
    menu.addAction(QIcon("../private/skins/black/serialport.png"), tr("Open serialport session"), this, SLOT(onOpenSerialPort()));
    menu.exec(pt);
}

void QWoMainWindow::onRecentMenuAboutToShow()
{
    QMenu *menu = qobject_cast<QMenu*>(sender());
    menu->clear();
    menu->addAction(tr("Clear all"), m_recent, SLOT(onClearAll()));
    menu->addSeparator();
    m_recent->buildMenu(menu);
}

void QWoMainWindow::onReportResult(int code, const QByteArray &body)
{
    //qDebug() << "onReportResult" << code << body;
    QJsonDocument jsdoc = QJsonDocument::fromJson(body);
    if(!jsdoc.isObject()) {
        return;
    }
    QJsonObject obj = jsdoc.object();
    if(!obj.contains("error")) {
        return;
    }
    QVariant err = obj.value("error");
    int ierr = err.toInt();
    if(ierr != 0) {
        QKxVer *ver = QKxVer::instance();
        QKxVer::ELicenseType type = ver->licenseType();
        if( type == QKxVer::ESchoolVersion || type == QKxVer::EUltimateVersion ) {
            // key has been migrate to another machine.
            QKxVer::instance()->remove();
            QString desc = obj.value("desc").toString();
            QKxMessageBox::information(this, tr("Error Information"), tr("The license code has been invalidated as follow reason")+"\r\n"+desc);
            QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
        }
    }
}

void QWoMainWindow::onTabMergeButtonClicked()
{
    if(m_tab->count() == 0) {
        QKxMessageBox::information(this, tr("Merge information"), tr("No tabs to merge."));
        return;
    }
    m_shower->mergeFromRightTab();
}

void QWoMainWindow::onTabSeperateButtonClicked()
{
    m_shower->seperateToRightTab();
}

void QWoMainWindow::onTabCurrentChanged(int idx)
{
    /*
    QVariant vImpl = m_tab->tabData(idx);
    QWoTermWidgetImpl *impl = vImpl.value<QWoTermWidgetImpl*>();
    if(impl == nullptr) {
        m_btnTabMerge->setEnabled(false);
        m_btnTabSeperate->setEnabled(false);
        return;
    }
    m_btnTabMerge->setEnabled(true);
    m_btnTabSeperate->setEnabled(impl->termCount() > 1);
    */
}

void QWoMainWindow::onPlaybookButtonClicked()
{
    QWoPlaybookManageDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_shower->openPlayBook(dlg.name(), dlg.path());
    }
}

void QWoMainWindow::onPlaybookAssistButtonClicked(QToolButton* btn)
{
    QRect rt = btn->rect();
    QPoint pt = btn->mapToGlobal(rt.bottomLeft());
    QMenu menu(this);
    menu.addAction(QIcon("../private/skins/black/nodes.png"), tr("Open remote session"), this, SLOT(onOpenRemoteSession()));
    menu.addAction(QIcon("../private/skins/black/console.png"), tr("Open local session"), this, SLOT(onOpenLocalSession()));
    menu.addAction(QIcon("../private/skins/black/serialport.png"), tr("Open serialport session"), this, SLOT(onOpenSerialPort()));
    menu.exec(pt);
}

void QWoMainWindow::onTunnelButtonClicked()
{
    if(QWoSetting::tunnelRunAsDaemon()) {
        if(m_dlgTunnel) {
            m_dlgTunnel->deleteLater();
        }
        QWoTunnelFactory::instance()->releaseAll();
        QString pathApp = QCoreApplication::applicationFilePath();
        QString cmd = QString("\"%1\" --tunnel").arg(pathApp);
        QProcess::startDetached(cmd);
    }else{
        if(m_dlgTunnel == nullptr) {
            m_dlgTunnel = new QWoTunnelDialog(this);
        }
        m_dlgTunnel->exec();
    }
}

void QWoMainWindow::onTunnelAssistButtonClicked(QToolButton *btn)
{

}

void QWoMainWindow::onActionNewTriggered()
{
    onNewSession();
}

void QWoMainWindow::onActionOpenRemoteTriggered()
{
    onOpenRemoteSession();
}

void QWoMainWindow::onActionOpenLocalTriggered()
{
    onOpenLocalSession();
}

void QWoMainWindow::onActionOpenSerialportTriggered()
{
    onOpenSerialPort();
}

void QWoMainWindow::onActionBackupTriggered()
{
    if(QKxVer::instance()->isFullFeather()) {
        QWoDbBackupDialog dlg(this);
        dlg.exec();
    }else{
        QString path = QWoSetting::lastBackupPath();
        QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Session Database"), path, "SQLite3 (*.db)");
        qDebug() << "fileName" << fileName;
        if(fileName.isEmpty()) {
            return;
        }
        if(!fileName.endsWith(".db")) {
            fileName += ".db";
        }
        QFileInfo fi(fileName);
        QString last = fi.absolutePath();
        QWoSetting::setLastBackupPath(last);
        if(!QWoSshConf::instance()->backup(fileName)) {
            QKxMessageBox::warning(this, tr("Failure"), tr("failed to backup the session list."));
        }
    }
}

void QWoMainWindow::onActionRestoreTriggered()
{
    if(QKxVer::instance()->isFullFeather()) {
        QWoDBPowerRestoreDialog dlg(this);
        dlg.exec();
    }else{
        QWoDBRestoreDialog dlg(this);
        dlg.exec();
    }
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

void QWoMainWindow::onActionTTYOptionsTriggered()
{
    QWoSessionTTYProperty dlg(QWoSessionTTYProperty::ETTY_Default, this);
    dlg.exec();
}

void QWoMainWindow::onActionVNCOptionsTriggered()
{
    QWoSessionVNCProperty dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionRDPOptionsTriggered()
{
    QWoSessionRDPProperty dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionToolOptionsTriggered()
{
    QWoSessionToolConfigureDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionSystemOptionsTriggered()
{
    QWoSystemOptionDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        QString lang = dlg.language();
        QString skin = dlg.skin();
        if(!lang.isEmpty() || !skin.isEmpty()) {
            QString langNow = QWoSetting::languageFileName();
            QString skinNow = QWoTheme::instance()->skinUniqueName();
            if(lang != langNow || skin != skinNow ) {
                QWoSetting::setLanguageFileName(lang);
                QWoTheme::instance()->setSkinUniqueName(skin);
                if(QKxMessageBox::warning(this, tr("Configure information"), tr("The configure has been changed, restart application to take effect right now."), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
                    QString path = QCoreApplication::instance()->applicationFilePath();
                    if(QKxProcessLaunch::startDetached(path)) {
                        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
                    }
                }
            }            
        }
    }
}

void QWoMainWindow::onActionRestartOptionsTriggered()
{
    QString path = QCoreApplication::instance()->applicationFilePath();
    if(QKxProcessLaunch::startDetached(path)) {
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
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
    input.setLabelText(pass.isEmpty() ? tr("Login to the configuration of administrator for the first time, Please input password to activate it.") : tr("Please input password to verify."));
    input.setTextEchoMode(QLineEdit::Password);
    int err = input.exec();
    if(err == 0) {
        return;
    }
    QString hitTxt = input.textValue();
    if(!pass.isEmpty()) {
        if(pass != hitTxt) {
            QKxMessageBox::information(this, tr("Password error"), tr("the password is not right."));
            return;
        }
    }else{
        QWoSetting::setAdminPassword(hitTxt.toUtf8());
    }
    QWoAdminDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionLicenseTriggered()
{
    QWoLicenseDialog dlg(this);
    dlg.exec();
}

void QWoMainWindow::onActionUpgradeTriggered()
{
    QWoVersionUpgradeTipDialog::check(this, false);
}

void QWoMainWindow::onActionPlaybookOptionsTriggered()
{
    QWoPlaybookManageDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_shower->openPlayBook(dlg.name(), dlg.path());
    }
}

void QWoMainWindow::onActionactionPurchaseLicenseTriggered()
{
    QDesktopServices::openUrl(QUrl("http://woterm.com/prices"));
}

void QWoMainWindow::onFilterArrivedArrived(const QString &name, int type)
{
    onSessionReadyToConnect(name, type);
}

void QWoMainWindow::onFilterCreateArrived(const QString &name)
{
    QWoSessionProperty dlg(this);
    dlg.setName(name);
    if(name.contains(':')) {
        dlg.setHostPort(name);
    }
    QObject::connect(&dlg, SIGNAL(readyToConnect(QString, int)), this, SLOT(onSessionReadyToConnect(QString,int)));
    int retVal = dlg.exec();
    if(retVal == QWoSessionProperty::Save || retVal == QWoSessionProperty::Connect) {
        QWoHostListModel::instance()->refreshList();
    }
}

void QWoMainWindow::initMenuBar()
{
    ui->menuBar->setNativeMenuBar(false);
    QObject::connect(ui->menuRecent, SIGNAL(aboutToShow()), this, SLOT(onRecentMenuAboutToShow()));
    QObject::connect(ui->actionSessionNew, SIGNAL(triggered()), this, SLOT(onActionNewTriggered()));
    QObject::connect(ui->actionOpenRemote, SIGNAL(triggered()), this, SLOT(onActionOpenRemoteTriggered()));
    QObject::connect(ui->actionOpenRemote2, SIGNAL(triggered()), this, SLOT(onActionOpenRemoteTriggered()));
    QObject::connect(ui->actionOpenLocal, SIGNAL(triggered()), this, SLOT(onActionOpenLocalTriggered()));
    QObject::connect(ui->actionOpenSerialport, SIGNAL(triggered()), this, SLOT(onActionOpenSerialportTriggered()));
    QObject::connect(ui->actionBackup, SIGNAL(triggered()), this, SLOT(onActionBackupTriggered()));
    QObject::connect(ui->actionRestore, SIGNAL(triggered()), this, SLOT(onActionRestoreTriggered()));
    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onActionExitTriggered()));
    QObject::connect(ui->actionToolBar, SIGNAL(triggered()), this, SLOT(onActionToolbarTriggered()));
    QObject::connect(ui->actionSessionList, SIGNAL(triggered()), this, SLOT(onActionSessionListTriggered()));
    QObject::connect(ui->actionTTYOptions, SIGNAL(triggered()), this, SLOT(onActionTTYOptionsTriggered()));
    QObject::connect(ui->actionVNCOptions, SIGNAL(triggered()), this, SLOT(onActionVNCOptionsTriggered()));
    QObject::connect(ui->actionRDPOptions, SIGNAL(triggered()), this, SLOT(onActionRDPOptionsTriggered()));
    QObject::connect(ui->actionToolOptions, SIGNAL(triggered()), this, SLOT(onActionToolOptionsTriggered()));
    QObject::connect(ui->actionSystemOptions, SIGNAL(triggered()), this, SLOT(onActionSystemOptionsTriggered()));
    QObject::connect(ui->actionIdentityManage, SIGNAL(triggered()), this, SLOT(onActionSshKeyManageTriggered()));
    QObject::connect(ui->actionDocument, SIGNAL(triggered()), this, SLOT(onActionHelpTriggered()));
    QObject::connect(ui->actionWetsite, SIGNAL(triggered()), this, SLOT(onActionWebsiteTriggered()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionAboutTriggered()));
    QObject::connect(ui->actionLicense, SIGNAL(triggered()), this, SLOT(onActionLicenseTriggered()));
    QObject::connect(ui->actionUpgrade, SIGNAL(triggered()), this, SLOT(onActionUpgradeTriggered()));
    QObject::connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(onActionRestartOptionsTriggered()));

    QObject::connect(ui->actionPlaybooks, SIGNAL(triggered()), this, SLOT(onActionPlaybookOptionsTriggered()));
    QObject::connect(ui->actionPurchaseLicense, SIGNAL(triggered()), this, SLOT(onActionactionPurchaseLicenseTriggered()));

    QObject::connect(ui->actionAdministrator, SIGNAL(triggered()), this, SLOT(onActionAdminTriggered()));
    ui->actionAdministrator->setEnabled(QKxVer::instance()->isFullFeather());

}

void QWoMainWindow::initToolBar()
{
    QToolBar *tool = ui->mainToolBar;
    tool->setWindowTitle(tr("Toolbar"));
    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/history.png"), tr("History"), tool);
        btn->setFlat(true);
        btn->setObjectName("btnHistory");
        QMenu *menu = new QMenu(btn);
        QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(onRecentMenuAboutToShow()));
        btn->setMenu(menu);
        tool->addWidget(btn);
    }


    QPushButton *btn = new QPushButton(QIcon("../private/skins/black/nodes.png"), tr("Open"), tool);
    btn->setFlat(true);
    btn->setObjectName("btnOpen");

    {
        QKxButtonAssist *btnAssist = new QKxButtonAssist("../private/skins/black/arrowdown.png", btn);
        QObject::connect(btnAssist, SIGNAL(pressed(QToolButton*)), this, SLOT(onButtonAssistClicked(QToolButton*)));
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onOpenRemoteSession()));
        tool->addWidget(btn);
    }

    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/add2.png"), tr("New"), tool);
        btn->setFlat(true);
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onNewSession()));
        tool->addWidget(btn);
    }
    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/layout.png"), tr("List"), tool);
        btn->setFlat(true);
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onLayout()));
        tool->addWidget(btn);
    }

    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/merge.png"), tr("Merge"), tool);
        btn->setFlat(true);
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onTabMergeButtonClicked()));
        tool->addWidget(btn);
        m_btnTabMerge = btn;
    }

    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/js.png"), tr("Playbooks"), tool);
        btn->setFlat(true);
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onPlaybookButtonClicked()));
        tool->addWidget(btn);
    }

    {
        QPushButton *btn = new QPushButton(QIcon("../private/skins/black/magento.png"), tr("Tunnel"), tool);
        btn->setFlat(true);
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onTunnelButtonClicked()));
        tool->addWidget(btn);
    }

    {
        QLineEdit *input = new QKxFilterLineEdit(tool);        
        input->setMaximumWidth(250);
        input->setPlaceholderText(tr("Enter keyword to search"));
        input->setObjectName(tr("filterBox"));
        tool->addWidget(input);
        QObject::connect(input, SIGNAL(createArrived(QString)), this, SLOT(onFilterCreateArrived(QString)));
        QObject::connect(input, SIGNAL(targetArrived(QString,int)), this, SLOT(onFilterArrivedArrived(QString,int)));
    }
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
    if(!QKxVer::instance()->isFullFeather()) {
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
        QKxMessageBox::information(this, tr("Login failure"), tr("The password is wrong, %1 times left to try.").arg(i));
    }
    return false;
}

void QWoMainWindow::tryToMakeLicenseTrial()
{
    QWoLicenseTrialApplyDialog dlg(this);
    dlg.exec();
}
