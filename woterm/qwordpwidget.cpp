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

#include "qrdpwidget.h"
#include "qwoutils.h"
#include "qwoshower.h"
#include "qwosetting.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QUuid>
#include <QPainter>


QWoRdpWidget::QWoRdpWidget(const QString &target, QWidget *parent)
    : QWidget(parent)
    , m_target(target)
    , m_smartResize(true)
{
    m_area = new QScrollArea(this);
    m_area->setFrameShape(QFrame::NoFrame);
    m_area->setContentsMargins(0, 0, 0, 0);

    m_loading = new QWoLoadingWidget(QColor(18,150,219), this);
    m_mask = new QWoTermMask(this);
    QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseThisSession()));
    QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));

    m_rdp = QRdpWidget::create(false, m_area);
    m_area->setWidget(m_rdp);
    QObject::connect(m_rdp, SIGNAL(connectingArrived()), this, SLOT(onConnectingArrived()));
    QObject::connect(m_rdp, SIGNAL(connectedArrived()), this, SLOT(onConnectedArrived()));
    QObject::connect(m_rdp, SIGNAL(disconnectedArrived()), this, SLOT(onDisconnectedArrived()));

    m_rdp->hide();
    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QWoRdpWidget::~QWoRdpWidget()
{

}

bool QWoRdpWidget::smartResize() const
{
    return m_smartResize;
}

void QWoRdpWidget::setSmartResize(bool on)
{
    m_smartResize = on;
    QMetaObject::invokeMethod(this, "resizeRdpWidget", Qt::QueuedConnection);
}

bool QWoRdpWidget::isConnected()
{
    return m_rdp->connectionStatus() == QRdpWidget::eConnected;
}

void QWoRdpWidget::stop()
{
    if(m_rdp) {
        m_rdp->stop();
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
    m_area->setGeometry(0, 0, sz.width(), sz.height());
    m_loading->setGeometry(0, 0, sz.width(), sz.height());
    m_mask->setGeometry(0, 0, sz.width(), sz.height());
    resizeRdpWidget();
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
    m_mask->setVisible(true);
    m_loading->setVisible(false);
    m_rdp->hide();
}

void QWoRdpWidget::onConnectingArrived()
{
    m_loading->show();
    m_mask->hide();
}

void QWoRdpWidget::onConnectedArrived()
{
    m_loading->hide();
    m_mask->hide();
    m_rdp->show();
}

void QWoRdpWidget::onDisconnectedArrived()
{
    m_loading->hide();
    m_mask->show();
    m_rdp->hide();
}

void QWoRdpWidget::reconnect()
{
    QDesktopWidget desk;
    QRect rt = desk.screenGeometry(this);
    int width = rt.width();
    int height = rt.height();

    /* FIXME: desktopWidth has a limitation that it should be divisible by 4,
             *        otherwise the screen will crash when connecting to an XP desktop.*/
    width = (width + 3) & (~3);
    const HostInfo& hi = QWoSshConf::instance()->find(m_target);

    QVariantMap mdata = QWoUtils::qBase64ToVariant(hi.property).toMap();
    if(mdata.isEmpty()) {
        mdata = QWoSetting::rdpDefault();
    }
    if(mdata.contains("desktopType")) {
        QString deskType = mdata.value("desktopType", "desktop").toString();
        if(deskType == "fix") {
            int w = mdata.value("desktopWidth", rt.width()).toInt();
            int h = mdata.value("desktopHeight", rt.height()).toInt();
            if(w > 10) {
                width = w;
            }
            if(h > 10) {
                height = h;
            }
        }
    }    
    int colorDepth = mdata.value("colorDepth", 16).toInt();
    m_rdp->setColorDepth(colorDepth);

    int mode = mdata.value("audioMode", 0).toInt();
    if(mode == 0) {
        m_rdp->setAudioPlayMode(QRdpWidget::ePlayOnLocal);
    }else if(mode == 1) {
        m_rdp->setAudioPlayMode(QRdpWidget::ePlayOnServer);
    }else {
        m_rdp->setAudioPlayMode(QRdpWidget::ePlayDisable);
    }

    QRdpWidget::PerformanceFlags flags;
    bool noContent = mdata.value("noContent", false).toBool();
    if(noContent) {
        flags |= QRdpWidget::eDISABLE_FULLWINDOWDRAG;
    }
    bool noWallpaper = mdata.value("noWallpaper", false).toBool();
    if(noWallpaper) {
        flags |= QRdpWidget::eDISABLE_WALLPAPER;
    }

    bool noTheme = mdata.value("noTheme", false).toBool();
    if(noTheme) {
        flags |= QRdpWidget::eDISABLE_THEMING;
    }

    bool noSmooth = mdata.value("noFontSmooth", true).toBool();
    if(!noSmooth) {
        flags |= QRdpWidget::eENABLE_FONT_SMOOTHING;
    }
    m_rdp->setPerformanceFlags(flags);
    m_rdp->setDesktopSize(width, height);
    m_rdp->start(hi.host, hi.port, hi.user, hi.password);
    resizeRdpWidget();

    m_mask->hide();
}

void QWoRdpWidget::resizeRdpWidget()
{
    if(m_smartResize) {
        QSize sz = m_area->size();
        m_rdp->resize(sz);
    }else{
        QSize ds = m_rdp->desktopSize();
        m_rdp->resize(ds);
    }
}
