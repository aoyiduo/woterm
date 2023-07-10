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

#include "qmotelnettermwidget.h"

#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotelnet.h"

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxmessagebox.h"

#include "qwomodem.h"


#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QResizeEvent>

QMoTelnetTermWidget::QMoTelnetTermWidget(const QString& target,  QWidget *parent)
    : QMoTermWidget(target, ETTRemoteTarget, parent)
    , m_savePassword(false)
    , m_stateConnected(ESC_Ready)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(QByteArray)), this, SLOT(onSendData(QByteArray)));
    QObject::connect(m_term, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChanged(QString)));
    m_modem = QWoModemFactory::instance()->create(true);
    QObject::connect(m_modem, SIGNAL(dataArrived(QByteArray)), this, SLOT(onZmodemDataArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(statusArrived(QByteArray)), this, SLOT(onZmodemStatusArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(finished()), this, SLOT(onZmodemFinished()));

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QMoTelnetTermWidget::~QMoTelnetTermWidget()
{
    QWoTelnetFactory::instance()->release(m_telnet);
    QWoModemFactory::instance()->release(m_modem);
}

void QMoTelnetTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    m_stateConnected = ESC_Disconnected;
    showLoading(false);
    m_term->parseError("\r\nthe session is closed");
    m_term->parseError("\r\npress any key to popup selection dialog.");
}

void QMoTelnetTermWidget::onDataArrived(const QByteArray &buf)
{
    static QRegExp rgxUser(".* login: $");
    static QRegExp rgxPwd(".*Password: $");

    m_stateConnected = ESC_Connected;

    showLoading(false);
    if(m_modem->isRunning()) {
        if(!m_modem->onReceive(buf)) {
            //qDebug() << "onDataArrived" << buf;
        }
    }else{
        if(m_term->appMode()) {
            m_term->parse(buf);
        }else{
            if(m_loginCount == 0) {
                if(rgxUser.exactMatch(buf)) {
                    HostInfo hi = QWoSshConf::instance()->find(m_target);
                    m_telnet->write(hi.user.toUtf8() + "\r\n");
                    m_loginCount = 1;
                }
            }else if(m_loginCount == 1) {
                if(rgxPwd.exactMatch(buf)) {
                    HostInfo hi = QWoSshConf::instance()->find(m_target);
                    m_telnet->write(hi.password.toUtf8() + "\r\n");
                    m_loginCount = 2;
                }
            }

            int err = isZmodemCommand(buf);
            m_term->parse(buf);
            if(err == 0) {
                QMetaObject::invokeMethod(this, "onZmodemRecv", Qt::QueuedConnection, Q_ARG(bool, false));
            }else if(err == 1) {
                QMetaObject::invokeMethod(this, "onZmodemSend", Qt::QueuedConnection, Q_ARG(bool, false));
            }
        }
    }
}

void QMoTelnetTermWidget::onErrorArrived(const QByteArray &buf)
{
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf);
}

void QMoTelnetTermWidget::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword){
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QMoTelnetTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_telnet) {
        m_telnet->updateSize(columns, lines);
    }
}

void QMoTelnetTermWidget::onSendData(const QByteArray &buf)
{
    qDebug() << "onSendData" << buf;
    if(m_stateConnected == ESC_Disconnected) {
        if(m_dlgConfirm == nullptr) {
            m_dlgConfirm = new QMessageBox(QMessageBox::Question, tr("Reconnection confirmation"), tr("Continue to connect to the server?"), QMessageBox::Yes|QMessageBox::No, this);
            if(m_dlgConfirm->exec() == QMessageBox::Yes) {
                QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
            }
            m_dlgConfirm->deleteLater();
        }
    }else if(m_telnet) {
        m_term->scrollToEnd();
        if(!m_modem->isRunning()) {
            if(buf.length() == 1 && buf.at(0) < 0x1f) {
                m_telnet->sendControl(buf.at(0));
            }else {
                m_telnet->write(buf);
            }
        }else{
            if(buf == "\003") {
                onZmodemAbort();
            }
        }
    }
}

void QMoTelnetTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QMoTelnetTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QMoTelnetTermWidget::onForceToReconnect()
{
    reconnect();
}

void QMoTelnetTermWidget::onSessionReconnect()
{
    reconnect();
}


void QMoTelnetTermWidget::onCloseThisSession()
{

}

void QMoTelnetTermWidget::onForceToCloseThisSession()
{

}

void QMoTelnetTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QMoTelnetTermWidget::onDuplicateInNewWindow()
{
    QWoUtils::openself("telnet", m_target, false);
}

void QMoTelnetTermWidget::onModifyThisSession()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QKxMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete ago"));
        return;
    }
}

void QMoTelnetTermWidget::onZmodemSend(bool local)
{
    if(local) {
        if(!checkZmodemInstall()) {
            return;
        }
    }
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Files"), pathLast);
    //qDebug() << "zmodem send " << files;
    if(files.isEmpty()) {
        return;
    }
    QStringList args;
    QString path = files.front();
    int idx = path.lastIndexOf('/');
    if(idx > 0) {
        path = path.left(idx);
        QWoSetting::setValue("zmodem/lastPath", path);
    }

    m_modem->ZSendFiles(files);
}

void QMoTelnetTermWidget::onZmodemRecv(bool local)
{
    QString path = QWoSetting::value("zmodem/lastPath").toString();
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), path,  QFileDialog::ShowDirsOnly);
    qDebug() << "filePath" << filePath;
    if(filePath.isEmpty()) {
        onZmodemAbort();
        return;
    }
    filePath = QDir::toNativeSeparators(filePath);
    QWoSetting::setValue("zmodem/lastPath", filePath);
    m_modem->ZReceive(filePath);
}

void QMoTelnetTermWidget::onZmodemAbort()
{
    m_modem->abort();
}

void QMoTelnetTermWidget::onZmodemDataArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        //qDebug() << "onZmodemDataArrived" << objectName() << buf;
        m_telnet->write(buf);
    }
}

void QMoTelnetTermWidget::onZmodemStatusArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        m_term->parse(buf);
    }
}

void QMoTelnetTermWidget::onZmodemFinished()
{
    //qDebug() << "onZmodemFinished" << objectName();
    m_telnet->write("\r");
    m_term->parse("\033[?25h");
}

void QMoTelnetTermWidget::onTitleChanged(const QString &title)
{
    m_loginCount = 100;
}

int QMoTelnetTermWidget::isZmodemCommand(const QByteArray &data)
{
    bool isApp = m_term->appMode();
    if(isApp || data.length() < 6) {
        return -1;
    }
    // hex way.
    //char zmodem_init_hex[] = {'*','*','\030', 'B', '0', '0', '\0'};
    const char *buf = data.data();
    for(int i = 0; i < data.length() && i < 100; i++) {
        if(buf[i] == '*' && buf[i+1] == '*' && buf[i+2] == '\030'
                && buf[i+3] == 'B' && buf[i+4] == '0') {
            if(buf[i+5] == '1') {
                // shell trigger rz command.
                return 1;
            }else if(buf[i+5] == '0') {
                // shell trigger sz command.
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

bool QMoTelnetTermWidget::checkZmodemInstall()
{
    if(m_modem->isRunning()) {
        return false;
    }
    if(m_term->appMode()) {
        return false;
    }
    QString content, reason;
    QString yes = "Yes, lrzsz has been installed.";
    QString no = "No, you need to install lrzsz package to execute zmodem.";
    QString cmd = QString("which rz && echo \"%1\" || echo \"%2\"").arg(yes, no);
    if(!m_term->executeCommand(cmd, content, reason)) {
        return false;
    }
    QString result = content;
    int pos = result.indexOf(cmd);
    if(pos >= 0) {
        result = result.mid(pos+cmd.length());
    }
    return result.contains(yes);
}
void QMoTelnetTermWidget::reconnect()
{
    showLoading(true);
    m_loginCount = 0;
    if(m_telnet) {
        QWoTelnetFactory::instance()->release(m_telnet);
    }
    m_term->resetState();
    m_telnet = QWoTelnetFactory::instance()->create();
    m_telnet->start(m_target);
    QObject::connect(m_telnet, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_telnet, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));
    QObject::connect(m_telnet, SIGNAL(errorArrived(QByteArray)), this, SLOT(onErrorArrived(QByteArray)));
    QSize sz = m_term->termSize();
    m_telnet->updateSize(sz.width(), sz.height());
    m_stateConnected = ESC_Connecting;
}

void QMoTelnetTermWidget::resizeEvent(QResizeEvent *ev)
{
    QMoTermWidget::resizeEvent(ev);
    QSize sz = ev->size();
    //qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
}

void QMoTelnetTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{

}
