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

#include "qwordpwidget.h"
#include "qwosshconf.h"

#include "qwoloadingwidget.h"
#include "qwotermmask.h"

#include "qrdpdef.h"
#include "qrdpwork.h"
#include "qwoutils.h"
#include "qwoshower.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QUuid>
#include <QPainter>


QWoRdpWidget::QWoRdpWidget(const QString &target, QWidget *parent)
    : QWidget(parent)
    , m_target(target)
{
    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    m_loading = new QWoLoadingWidget(QColor("#1296DB"), this);
    m_mask = new QWoTermMask(this);
    QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseThisSession()));
    QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(50);

    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

QWoRdpWidget::~QWoRdpWidget()
{
    if(m_rdp) {
        QWoRdpFactory::instance()->release(m_rdp);
    }
}

void QWoRdpWidget::setFullScreen(bool full)
{

}

void QWoRdpWidget::reconnect()
{
    if(m_rdp) {
        QWoRdpFactory::instance()->release(m_rdp);
    }
    m_rdp = QWoRdpFactory::instance()->create();
    QObject::connect(m_rdp, SIGNAL(finished()), this, SLOT(onFinished()));
    QDesktopWidget desk;
    QRect rt = desk.screenGeometry(this);
    int width = rt.width();
    int height = rt.height();

    /* FIXME: desktopWidth has a limitation that it should be divisible by 4,
         *        otherwise the screen will crash when connecting to an XP desktop.*/
    width = (width + 3) & (~3);
    const HostInfo& hi = QWoSshConf::instance()->find(m_target);

    QRdpWork::TargetInfo ti;
    ti.host = hi.host;
    ti.password = hi.password;
    ti.port = hi.port;
    ti.user = hi.user;

    QVariantMap mdata = QWoUtils::qBase64ToVariant(hi.property).toMap();
    if(mdata.contains("desktopType")) {
        QString deskType = mdata.value("desktopType", "desktop").toString();
        if(deskType == "fix") {
            int w = mdata.value("desktopWidth", "1024").toInt();
            int h = mdata.value("desktopHeight", "768").toInt();
            if(w > 10) {
                width = w;
            }
            if(h > 10) {
                height = h;
            }
        }
    }
    m_rdp->start(ti, width, height);
    m_loading->show();
}

void QWoRdpWidget::onTimeout()
{
    if(m_rdp) {
        QRect rt = m_rdp->clip(size());
        if(!rt.isEmpty()){
            rt.adjust(-10, -10, 10, 10);
            if(m_loading->isVisible()) {
                m_loading->setVisible(false);
                m_mask->setVisible(false);
            }
            update(rt);
        }
    }
}

void QWoRdpWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

void QWoRdpWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QSize sz = event->size();
    m_loading->setGeometry(0, 0, sz.width(), sz.height());
    m_mask->setGeometry(0, 0, sz.width(), sz.height());
}

void QWoRdpWidget::paintEvent(QPaintEvent *ev)
{
    if(m_rdp == nullptr) {
        return;
    }
    QPainter p(this);
    p.setClipRegion(ev->region());
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QImage img = m_rdp->capture();
    QRect drawRt = rect();
    p.drawImage(drawRt, img);
}

void QWoRdpWidget::mousePressEvent(QMouseEvent *ev)
{
    if(m_rdp){
        m_rdp->mousePressEvent(ev, size());
    }
}

void QWoRdpWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if(m_rdp){
        m_rdp->mouseMoveEvent(ev, size());
    }
}

void QWoRdpWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(m_rdp){
        m_rdp->mouseReleaseEvent(ev, size());
    }
}

void QWoRdpWidget::wheelEvent(QWheelEvent *ev)
{
    if(m_rdp){
        m_rdp->wheelEvent(ev, size());
    }
}

void QWoRdpWidget::keyPressEvent(QKeyEvent *ev)
{
    if(m_rdp){
        m_rdp->keyPressEvent(ev);
    }
}

void QWoRdpWidget::keyReleaseEvent(QKeyEvent *ev)
{
    if(m_rdp){
        m_rdp->keyReleaseEvent(ev);
    }
}

void QWoRdpWidget::focusInEvent(QFocusEvent *ev)
{
    QWidget::focusInEvent(ev);
    if(m_rdp){
        m_rdp->restoreKeyboardStatus();
    }
}

void QWoRdpWidget::focusOutEvent(QFocusEvent *ev)
{
    QWidget::focusOutEvent(ev);
    if(m_rdp){
        m_rdp->restoreKeyboardStatus();
    }
}

bool QWoRdpWidget::focusNextPrevChild(bool next)
{
    return false;
}

void QWoRdpWidget::onSessionReconnect()
{
    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

void QWoRdpWidget::onForceToCloseThisSession()
{
    QWoShower::forceToCloseTopLevelWidget(this);
}

void QWoRdpWidget::onFinished()
{
    if(m_rdp) {
        QWoRdpFactory::instance()->release(m_rdp);
    }

    m_rdp = nullptr;
    m_mask->setVisible(true);
    m_loading->setVisible(false);
}
