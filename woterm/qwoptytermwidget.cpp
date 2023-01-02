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

#include "qwoptytermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotermwidgetimpl.h"
#include "qwotelnet.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwohostsimplelist.h"
#include "qwopty.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qwosessionproperty.h"
#include "qwosessionttyproperty.h"
#include "qwofloatwindow.h"
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

QWoPtyTermWidget::QWoPtyTermWidget(const QString& target, int gid, QWidget *parent)
    : QWoTermWidget(target, gid, ETTLocalShell, parent)
    , m_stateConnected(ESC_Ready)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(const QByteArray&)), this, SLOT(onSendData(const QByteArray&)));
    QObject::connect(m_term, SIGNAL(titleChanged(const QString&)), this, SLOT(onTitleChanged(const QString&)));
    QTimer::singleShot(100, this, [=](){
        QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    });
}

QWoPtyTermWidget::~QWoPtyTermWidget()
{
    QWoPtyFactory::instance()->release(m_pty);
}

void QWoPtyTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    showLoading(false);
    if(m_stateConnected != ESC_Disconnected) {
        m_stateConnected = ESC_Disconnected;
        m_term->parseError("\r\nthe session is closed");
        m_term->parseError("\r\npress any key to popup selection dialog.");
    }
}

void QWoPtyTermWidget::onConnectionFinished(bool ok)
{
    showLoading(false);
}

void QWoPtyTermWidget::onDataArrived(const QByteArray &buf)
{
    //qDebug() << buf;
    m_term->parse(buf);
}

void QWoPtyTermWidget::onErrorArrived(const QByteArray &buf)
{
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf);
}

void QWoPtyTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_pty == nullptr) {
        return;
    }
    m_pty->updateSize(columns, lines);
}

void QWoPtyTermWidget::onSendData(const QByteArray &buf)
{
    if(m_pty == nullptr) {
        return;
    }
    if(m_stateConnected == ESC_Disconnected) {
        if(m_dlgConfirm == nullptr) {
            m_dlgConfirm = new QKxMessageBox(QMessageBox::Question, tr("Reconnection confirmation"), tr("Continue to connect to the server?"), QMessageBox::Yes|QMessageBox::No, this);
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

void QWoPtyTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QWoPtyTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QWoPtyTermWidget::onForceToReconnect()
{
    reconnect();
}

void QWoPtyTermWidget::onSessionReconnect()
{    
    reconnect();
}

void QWoPtyTermWidget::onVerticalSplitView()
{
    splitWidget(m_target, m_gid, true);
}

void QWoPtyTermWidget::onHorizontalSplitView()
{
    splitWidget(m_target, m_gid, false);
}

void QWoPtyTermWidget::onVerticalInviteView()
{
    QWoHostSimpleList dlg(Telnet, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), true);
    }
}

void QWoPtyTermWidget::onHorizontalInviteView()
{
    QWoHostSimpleList dlg(Telnet, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), false);
    }
}

void QWoPtyTermWidget::onCloseThisSession()
{
    closeAndDelete();
}

void QWoPtyTermWidget::onForceToCloseThisSession()
{
    closeAndDelete();
}

void QWoPtyTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QWoPtyTermWidget::onModifyThisSession()
{
    QVariantMap prop = QWoSetting::localShell();
    QWoSessionTTYProperty dlg(QWoSessionTTYProperty::ETTY_LocalShell, this);
    dlg.setCustom(prop);
    dlg.exec();
    QVariantMap result = dlg.result();
    if(!result.isEmpty()) {
        QWoSetting::setLocalShell(result);
        initCustom();
    }
}

void QWoPtyTermWidget::onTitleChanged(const QString &title)
{

}

void QWoPtyTermWidget::reconnect()
{
    showLoading(true);
    if(m_pty) {
        QWoPtyFactory::instance()->release(m_pty);
    }
    m_stateConnected = ESC_Connecting;
    m_pty = QWoPtyFactory::instance()->create();
    QObject::connect(m_pty, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_pty, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onDataArrived(const QByteArray&)));
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

QString QWoPtyTermWidget::shellPath() const
{
    QVariantMap mdata = QWoSetting::localShell();
    QString path = mdata.value("shellPath").toString();
    return path;
}

void QWoPtyTermWidget::resizeEvent(QResizeEvent *ev)
{
    QWoTermWidget::resizeEvent(ev);
    // QSize sz = ev->size();
    // qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
}

void QWoPtyTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    if(m_rkeyPaste) {
        if(m_term->isOverSelection(ev->pos())) {
            QString txtSel = m_term->selectedText();
            m_term->directSendData(txtSel.toUtf8());
            return;
        }
    }
    if(m_menu == nullptr) {
        m_menu = new QMenu(this);
        m_copy = m_menu->addAction(tr("Copy"));
        QObject::connect(m_copy, SIGNAL(triggered()), this, SLOT(onCopyToClipboard()));
        m_paste = m_menu->addAction(tr("Paste"));
        m_menu->addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Force reload"), this, SLOT(onForceToReconnect()));
        QObject::connect(m_paste, SIGNAL(triggered()), this, SLOT(onPasteFromClipboard()));
        QAction *vsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/vsplit.png"), tr("Split vertical"));
        QObject::connect(vsplit, SIGNAL(triggered()), this, SLOT(onVerticalSplitView()));
        QAction *hsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/hsplit.png"), tr("Split horizontal"));
        QObject::connect(hsplit, SIGNAL(triggered()), this, SLOT(onHorizontalSplitView()));

        QAction *close = m_menu->addAction(tr("Close session"));
        QObject::connect(close, SIGNAL(triggered()), this, SLOT(onCloseThisSession()));
        QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
        if(wfloat == nullptr) {
            m_menu->addAction(tr("Float this tab"), this, SLOT(onFloatThisTab()));
        }

        m_menu->addAction(QIcon(":/woterm/resource/skin/find.png"), tr("Find..."), this, SLOT(onShowFindBar()), QKeySequence(Qt::CTRL +  Qt::Key_F));
        m_menu->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
        m_output = m_menu->addAction(tr("Output history to file"), this, SLOT(onOutputHistoryToFile()));
        m_stop = m_menu->addAction(tr("Stop history to file"), this, SLOT(onStopOutputHistoryFile()));
    }
    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    //qDebug() << "selectText" << selTxt;
    m_copy->setDisabled(selTxt.isEmpty());
    m_output->setVisible(m_historyFile.isEmpty());
    m_stop->setVisible(!m_historyFile.isEmpty());

    QClipboard *clip = QGuiApplication::clipboard();
    QString clipTxt = clip->text();
    m_paste->setDisabled(clipTxt.isEmpty());
    m_menu->exec(QCursor::pos());
    m_menu->deleteLater();
}

QList<QString> QWoPtyTermWidget::collectUnsafeCloseMessage()
{
    QList<QString> all;
    return all;
}
