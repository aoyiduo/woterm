/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qmomainwindow.h"
#include "ui_qmomainwindow.h"

#include "qkxmessagebox.h"
#include "qkxhttpclient.h"

#include "qmoapplication.h"
#include "qmorecenthistorymodel.h"

#include "version.h"
#include "qwosetting.h"
#include "qwohostlistmodel.h"
#include "qwosshconf.h"
#include "qwosortfilterproxymodel.h"
#include "qwoutils.h"
#include "qmoptytermwidgetimpl.h"
#include "qmosshtermwidgetimpl.h"
#include "qmotelnettermwidgetimpl.h"
#include "qmorlogintermwidgetimpl.h"
#include "qmosftpwidgetimpl.h"
#include "qmomessageboxassist.h"
#include "qmodbsftpdetailassist.h"
#include "qwodbsftpdownsync.h"
#include "qmodirassist.h"
#include "qmosftpremotemodel.h"
#include "qmosftpassist.h"
#include "qmoquickassist.h"
#include "qmoandroidassist.h"
#include "qmovncwidgetimpl.h"

#include "qwoidentify.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>
#include <QDesktopServices>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

QMoMainWindow::QMoMainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QMoMainWindow)
    , m_closeHasConfirm(false)
{
    ui->setupUi(this);

    setWindowTitle(tr("WoTerm"));

    QWoSshConf::instance()->refresh();

    m_recentAccess = new QMoRecentHistoryModel(this);

    m_qmlContext = ui->quick->rootContext();
    ui->quick->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlEngine *engine = ui->quick->engine();
    QObject::connect(engine, SIGNAL(quit()), this, SLOT(onWindowCloseArrived()));

    QWoUtils::injectJS(engine, QStringList() << ":/woterm/qml/shot.js");

    m_qmlContext->setContextProperty("quick", new QMoQuickAssist(ui->quick));
    m_qmlContext->setContextProperty("gApp", QMoApplication::instance());
    m_qmlContext->setContextProperty("gUtils", new QWoUtils(this));
    m_qmlContext->setContextProperty("gRecentHistory", m_recentAccess);
    m_qmlContext->setContextProperty("gMainWindow", this);
    m_qmlContext->setContextProperty("gMessageBox", new QMoMessageBoxAssist(this));
    m_qmlContext->setContextProperty("gSetting", new QWoSetting(this));
    m_qmlContext->setContextProperty("gSshConf", QWoSshConf::instance());
    m_qmlContext->setContextProperty("gIdentify", new QWoIdentify(this));
    m_qmlContext->setContextProperty("gSystem", QMoAndroidAssist::instance());

    QSortFilterProxyModel *proxyModel = new QWoSortFilterProxyModel(1, this);
    proxyModel->setRecursiveFilteringEnabled(true);
    proxyModel->setFilterRole(ROLE_REFILTER);
    proxyModel->setSourceModel(QWoHostListModel::instance());
    m_qmlContext->setContextProperty("gHostListModel", proxyModel);



    ui->quick->setSource(QUrl(QStringLiteral("qrc:/woterm/qml/main.qml")));


    QTimer::singleShot(3000, this, SLOT(onInitLater()));
}

QMoMainWindow::~QMoMainWindow()
{
    delete ui;
}

QMoMainWindow *QMoMainWindow::instance()
{
    return QMoApplication::mainWindow();
}

QSize QMoMainWindow::size() const
{
    return ui->quick->size();
}

void QMoMainWindow::openLocalShell()
{
    if(m_show == nullptr) {
        m_show = new QMoPtyTermWidgetImpl(this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
}

bool QMoMainWindow::openSsh(const QString &target)
{
    if(m_show) {
        m_show->setVisible(false);
        m_show->deleteLater();
    }
    if(m_show == nullptr) {
        m_show = new QMoSshTermWidgetImpl(target, this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
    m_recentAccess->update(target, EOT_SSH);
    return true;
}

bool QMoMainWindow::openSftp(const QString &target)
{
    if(m_show) {
        m_show->setVisible(false);
        m_show->deleteLater();
    }
    if(m_show == nullptr) {
        m_show = new QMoSftpWidgetImpl(target, this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
    m_recentAccess->update(target, EOT_SFTP);
    return true;
}

bool QMoMainWindow::openTelnet(const QString &target)
{
    if(m_show) {
        m_show->setVisible(false);
        m_show->deleteLater();
    }
    if(m_show == nullptr) {
        m_show = new QMoTelnetTermWidgetImpl(target, this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
    m_recentAccess->update(target, EOT_TELNET);
    return true;
}

bool QMoMainWindow::openRLogin(const QString &target)
{
    if(!QWoUtils::hasUnprivilegedPortPermission()) {
        QKxMessageBox::information(this, tr("Permission error"), tr("Rlogin client need root permission to bind port which below 1023."));
        return false;
    }
    if(m_show) {
        m_show->setVisible(false);
        m_show->deleteLater();
    }
    if(m_show == nullptr) {
        m_show = new QMoRLoginTermWidgetImpl(target, this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
    m_recentAccess->update(target, EOT_RLOGIN);
    return false;
}

bool QMoMainWindow::openMstsc(const QString &target)
{
    m_recentAccess->update(target, EOT_MSTSC);
    return false;
}

bool QMoMainWindow::openVnc(const QString &target)
{
    if(m_show) {
        m_show->setVisible(false);
        m_show->deleteLater();
    }
    if(m_show == nullptr) {
        m_show = new QMoVncWidgetImpl(target, this);
    }
    QRect rt = rect();
    m_show->setGeometry(rt);
    m_show->show();
    m_recentAccess->update(target, EOT_VNC);
    return true;
}

bool QMoMainWindow::openRecent(const QString &target, int type)
{
    switch (type) {
    case EOT_SSH:
        return openSsh(target);
    case EOT_SFTP:
        return openSftp(target);
    case EOT_TELNET:
        return openTelnet(target);
    case EOT_VNC:
        return openVnc(target);
    case EOT_RLOGIN:
        return openRLogin(target);
    }
    return false;
}

bool QMoMainWindow::openSerialPort()
{
    return false;
}

void QMoMainWindow::onWindowCloseArrived()
{
    close();
}

void QMoMainWindow::onInitLater()
{
#ifdef Q_OS_ANDROID
    /* QAndroidJniObject intent("InstallApk");
    QAndroidJniObject jsText = QAndroidJniObject::fromString("/storage/emulated/0/myall/woterm.apk");
    jint ret = intent.callMethod<jint>("installApp","(Ljava/lang/String;)I",jsText.object<jstring>());*/
    //QAndroidJniObject path = QAndroidJniObject::fromString("/storage/emulated/0/myall/woterm.apk");
    //QAndroidJniObject::callStaticMethod<void>("com/aoyiduo/woterm/MainActivity", "install", "(Ljava/lang/String;)V", path.object<jstring>());
#endif
}

void QMoMainWindow::onAppStart()
{
    // version check.
    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionCheck(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get("http://down.woterm.com/.mver");
}

void QMoMainWindow::onVersionCheck(int code, const QByteArray &body)
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
            bool pop = QWoSetting::shouldPopupUpgradeVersionMessage(verBody);
            if(!pop) {
                return;
            }
            QWoSetting::setIgnoreTodayUpgradeVersion(verBody);
            int ret = QKxMessageBox::question(this, tr("Version check"), tr("a new version of %1 is found, do you want to update it?").arg(verBody), QMessageBox::Yes|QMessageBox::No);
            if(ret == QMessageBox::Yes) {
                emit readyUpgradeVersion();
            }
        }
    }
}

void QMoMainWindow::closeEvent(QCloseEvent *ev)
{
    if(!m_closeHasConfirm) {
        if(QKxMessageBox::warning(this, tr("Close confirm"), tr("Continue to exit application?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
            ev->setAccepted(false);
            return;
        }
    }

    QWidget::closeEvent(ev);
}

void QMoMainWindow::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    if(m_show) {
        m_show->resize(ev->size());
    }
}

