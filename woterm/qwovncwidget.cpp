/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwovncwidget.h"
#include "qwosshconf.h"

#include "qwoloadingwidget.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwoutils.h"
#include "qwosetting.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QUuid>
#include <QPainter>
#include <QScrollArea>


QWoVncWidget::QWoVncWidget(const QString &target, QWidget *parent)
    : QKxVncWidget(parent)
    , m_target(target)
    , m_parent(parent)
{
    parent->installEventFilter(this);
    m_isPasswdOk = true;
    m_savePassword = false;
    m_loading = new QWoLoadingWidget(QColor("#1296DB"), parent);
    m_mask = new QWoTermMask(parent);
    QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseSession()), Qt::QueuedConnection);
    QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));

    QObject::connect(this, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(this, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(this, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(this, SIGNAL(errorArrived(QByteArray)), this, SLOT(onErrorArrived(QByteArray)));
    QObject::connect(this, SIGNAL(passwordResult(QByteArray,bool)), this, SLOT(onPasswordResult(QByteArray,bool)));

    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QWoVncWidget::~QWoVncWidget()
{
    if(m_parent) {
        m_parent->removeEventFilter(this);
    }
}

void QWoVncWidget::reconnect()
{
    if(m_passInput) {
        m_passInput->deleteLater();
    }
    HostInfo hi = QWoSshConf::instance()->find(m_target);
    QVariantMap mdata = config(hi.merge(hi.type, QWoSetting::vncDefault()));
    QKxVNC::EPixelFormat fmt;
    QKxVNC::EProtoVersion ver;
    QVector<QKxVNC::EEncodingType> encs;
    {
        QString v = mdata.value("vncProto", "RFB_38").toString();
        if(v == "RFB_33") {
            ver = QKxVNC::RFB_33;
        }else if(v == "RFB_37") {
            ver = QKxVNC::RFB_37;
        }else {
            ver = QKxVNC::RFB_38;
        }
    }
    {
        QString v = mdata.value("vncPixel", "RGB16_565").toString();
        if(v == "RGB32_888") {
            fmt = QKxVNC::RGB32_888;
        }else if(v == "RGB16_565") {
            fmt = QKxVNC::RGB16_565;
        }else if(v == "RGB15_555") {
            fmt = QKxVNC::RGB15_555;
        }else if(v == "RGB8_332") {
            fmt = QKxVNC::RGB8_332;
        }else if(v == "RGB8_Map") {
            fmt = QKxVNC::RGB8_Map;
        }else {
            fmt = QKxVNC::RGB16_565;
        }
    }

    if(mdata.value("vncJPEG", true).toBool()) {
        encs << QKxVNC::JPEG;
    }
    if(mdata.value("vncH264", true).toBool()) {
        encs << QKxVNC::OpenH264;
    }
    if(mdata.value("vncZRLE", true).toBool()) {
        encs << QKxVNC::ZRLE;
    }
    if(mdata.value("vncTRLE", true).toBool()) {
        encs << QKxVNC::TRLE;
    }
    if(mdata.value("vncZRLE2", true).toBool()) {
        encs << QKxVNC::ZRLE2;
    }
    if(mdata.value("vncTRLE2", true).toBool()) {
        encs << QKxVNC::TRLE2;
    }
    if(mdata.value("vncZRLE3", true).toBool()) {
        encs << QKxVNC::ZRLE3;
    }
    if(mdata.value("vncTRLE3", true).toBool()) {
        encs << QKxVNC::TRLE3;
    }
    if(mdata.value("vncHextile", true).toBool()) {
        encs << QKxVNC::Hextile;
    }
    if(mdata.value("vncRRE", true).toBool()) {
        encs << QKxVNC::RRE;
    }
    if(mdata.value("vncCopyRect", true).toBool()) {
        encs << QKxVNC::CopyRect;
    }
    encs << QKxVNC::Raw;
    if(mdata.value("vncDeskResize", true).toBool()) {
        encs << QKxVNC::DesktopSizePseudoEncoding;
    }
    bool viewOnly = mdata.value("vncViewOnly", false).toBool();
    QString passwd = "";
    if(m_isPasswdOk) {
        if(!m_passwdLast.isEmpty()) {
            passwd = m_passwdLast;
        }else{
            passwd = hi.password;
        }
    }

    if(encs.contains(QKxVNC::OpenH264)) {
        fmt = QKxVNC::H264_Normal;
    }else {
        fmt = QKxVNC::RGB16_565;
    }
    QKxVncWidget::start(hi.host, hi.port, passwd, fmt, ver, encs);
    QKxVncWidget::setViewOnly(viewOnly);

    m_mask->hide();
    m_loading->show();
}

void QWoVncWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

void QWoVncWidget::onSetQualityLevel(int lv)
{
    setQualityLevel(lv);
}

void QWoVncWidget::onBlackScreen()
{
    setBlackScreen();
}

void QWoVncWidget::onLockScreen()
{
    setLockScreen();
}

void QWoVncWidget::onNetScreenRatioRequest()
{
    setNextRatio();
}

void QWoVncWidget::onNextScreenModeRequest()
{
    setNextScreen();
}

void QWoVncWidget::onForceToCloseSession()
{
    if(m_loading) {
        delete m_loading;
    }
    if(m_mask) {
        delete m_mask;
    }
    if(m_passInput) {
        delete m_passInput;
    }

    emit forceToClose();
}

void QWoVncWidget::onSessionReconnect()
{
    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

void QWoVncWidget::onFinished()
{
    m_mask->setVisible(true);
    m_loading->setVisible(false);
}

void QWoVncWidget::onConnectionStart()
{

}

void QWoVncWidget::onConnectionFinished(bool ok)
{
    m_loading->setVisible(false);
    if(!ok) {
        m_mask->setVisible(true);
    }
}

void QWoVncWidget::onErrorArrived(const QByteArray &buf)
{
    qDebug() << "errorArrived" << buf;
}

void QWoVncWidget::onPasswordResult(const QByteArray &passwd, bool ok)
{
    if(m_savePassword && ok) {
        QWoSshConf::instance()->updatePassword(m_target, passwd);
    }
    m_isPasswdOk = ok;
    if(!ok) {
        showPasswordInput(tr("Password"), tr("Please input the right password."), false);
    }    
}

void QWoVncWidget::onPasswordInputResult(const QString &pass, bool isSave)
{
    if(m_passInput) {
        m_passInput->close();
    }

    m_isPasswdOk = true;
    m_savePassword = isSave;
    m_passwdLast = pass;
    m_loading->hide();
    m_mask->hide();
    reconnect();
}

void QWoVncWidget::onAdjustPosition()
{
    if(m_passInput) {
        QSize sz = m_passInput->minimumSize();
        if(sz.width() == 0 || sz.height() == 0) {
            sz = m_passInput->size();
        }
        QRect rt(0, 0, sz.width(), sz.height());
        rt.moveCenter(QPoint(m_parent->width() / 2, m_parent->height() / 2));
        m_passInput->setGeometry(rt);
    }
}

QVariantMap QWoVncWidget::config(const QVariantMap &prop)
{
    QVariantMap result = prop;
    QString val = QWoSetting::value("property/default").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    if(!result.contains("vncProto")) {
        result.insert("vncProto", mdata.value("vncProto"));
    }
    if(!result.contains("vncPixel")) {
        result.insert("vncPixel", mdata.value("vncPixel"));
    }
    if(!result.contains("vncZRLE")) {
        result.insert("vncZRLE", mdata.value("vncZRLE"));
    }
    if(!result.contains("vncTRLE")) {
        result.insert("vncTRLE", mdata.value("vncTRLE"));
    }
    if(!result.contains("vncHextile")) {
        result.insert("vncHextile", mdata.value("vncHextile"));
    }
    if(!result.contains("vncRRE")) {
        result.insert("vncRRE", mdata.value("vncRRE"));
    }
    if(!result.contains("vncCopyRect")) {
        result.insert("vncCopyRect", mdata.value("vncCopyRect"));
    }
    if(!result.contains("vncRaw")) {
        result.insert("vncRaw", mdata.value("vncRaw"));
    }
    if(!result.contains("vncDeskResize")) {
        result.insert("vncDeskResize", mdata.value("vncDeskResize"));
    }
    if(!result.contains("vncViewOnly")) {
        result.insert("vncViewOnly", mdata.value("vncViewOnly"));
    }
    return result;
}

void QWoVncWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(m_parent);
        QObject::connect(m_passInput, SIGNAL(result(QString,bool)), this, SLOT(onPasswordInputResult(QString,bool)));
    }

    QMetaObject::invokeMethod(m_mask, "hide", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_loading, "hide", Qt::QueuedConnection);

    m_passInput->reset(title, prompt, echo);
    m_passInput->adjustSize();
    m_passInput->showNormal();
    m_passInput->raise();
    QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
}

bool QWoVncWidget::eventFilter(QObject *w, QEvent *e)
{
    QEvent::Type t = e->type();
    if(t == QEvent::Resize) {
        QResizeEvent *re = (QResizeEvent*)e;
        const QSize &sz = re->size();
        if(m_passInput) {
            QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
        }
        if(m_loading) {
            m_loading->setGeometry(0, 0, sz.width(), sz.height());
        }
        if(m_mask) {
            m_mask->setGeometry(0, 0, sz.width(), sz.height());
        }
    }
    return QWidget::eventFilter(w, e);
}
