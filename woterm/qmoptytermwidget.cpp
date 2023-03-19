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

#include "qmoptytermwidget.h"

#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwopty.h"
#include "qwoglobal.h"

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxmessagebox.h"


#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QResizeEvent>
#include <QPushButton>

QMoPtyTermWidget::QMoPtyTermWidget(const QString& target, QWidget *parent)
    : QMoTermWidget(target, ETTLocalShell,  parent)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(QByteArray)), this, SLOT(onSendData(QByteArray)));
    QObject::connect(m_term, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChanged(QString)));
    QTimer::singleShot(100, this, [=](){
        QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    });
}

QMoPtyTermWidget::~QMoPtyTermWidget()
{
    QWoPtyFactory::instance()->release(m_pty);
}

void QMoPtyTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    showLoading(false);
    if(m_stateConnected != ESC_Disconnected) {
        m_stateConnected = ESC_Disconnected;
        m_term->parseError("\r\nthe session is closed");
        m_term->parseError("\r\npress any key to popup selection dialog.");
    }
}

void QMoPtyTermWidget::onConnectionFinished(bool ok)
{
    showLoading(false);
}

void QMoPtyTermWidget::onDataArrived(const QByteArray &buf)
{
    //qDebug() << buf;
    m_term->parse(buf);
}

void QMoPtyTermWidget::onErrorArrived(const QByteArray &buf)
{
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf);
}

void QMoPtyTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_pty == nullptr) {
        return;
    }
    m_pty->updateSize(columns, lines);
}

void QMoPtyTermWidget::onSendData(const QByteArray &buf)
{
    if(m_pty == nullptr) {
        return;
    }
    if(m_stateConnected == ESC_Disconnected) {
        if(m_dlgConfirm == nullptr) {
            m_dlgConfirm = new QKxMessageBox(QMessageBox::Question, tr("Reconnection confirmation"), tr("Continue to connect to this session?"), QMessageBox::Yes|QMessageBox::No, this);
            int code = m_dlgConfirm->exec();
            if(code == QMessageBox::Yes) {
                QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
            }
            m_dlgConfirm->deleteLater();
        }
    }else if(m_pty) {
        m_term->scrollToEnd();
        m_pty->write(buf);
    }
}

void QMoPtyTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QMoPtyTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QMoPtyTermWidget::onForceToReconnect()
{
    reconnect();
}

void QMoPtyTermWidget::onSessionReconnect()
{
    reconnect();
}

void QMoPtyTermWidget::onCloseThisSession()
{

}

void QMoPtyTermWidget::onForceToCloseThisSession()
{

}

void QMoPtyTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QMoPtyTermWidget::onModifyThisSession()
{

}

void QMoPtyTermWidget::onTitleChanged(const QString &title)
{

}

void QMoPtyTermWidget::reconnect()
{
    showLoading(true);
    if(m_pty) {
        QWoPtyFactory::instance()->release(m_pty);
    }
    m_stateConnected = ESC_Connecting;
    m_pty = QWoPtyFactory::instance()->create();
    QObject::connect(m_pty, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_pty, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));
    QObject::connect(m_pty, SIGNAL(errorArrived(QByteArray)), this, SLOT(onErrorArrived(QByteArray)));
    QObject::connect(m_pty, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    m_term->resetState();
    QSize sz = m_term->termSize();
    QString val = QWoSetting::value("property/localShell").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    QString path = mdata.value("shellPath").toString();
    if(!QFile::exists(path)) {
        path = QWoUtils::findShellPath();
    }
    path = QDir::toNativeSeparators(path);
    m_pty->start(sz.width(), sz.height(), path);
}

QString QMoPtyTermWidget::shellPath() const
{
    QVariantMap mdata = QWoSetting::localShell();
    QString path = mdata.value("shellPath").toString();
    return path;
}

void QMoPtyTermWidget::resizeEvent(QResizeEvent *ev)
{
    QMoTermWidget::resizeEvent(ev);
}
