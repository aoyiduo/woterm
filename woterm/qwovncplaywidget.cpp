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

#include "qwovncplaywidget.h"
#include "qwovnctoolform.h"
#include "qwovncwidget.h"
#include "qwopasswordinput.h"

#include "qwovncftpwidget.h"
#include "qwoshower.h"

#include <QScrollArea>
#include <QResizeEvent>
#include <QDebug>

#define DURATION_DEFAULT        (150)

QWoVncPlayWidget::QWoVncPlayWidget(const QString &target, QWidget *parent)
    : QWidget(parent)
{
    m_area = new QScrollArea(this);
    m_area->setFrameShape(QFrame::NoFrame);
    m_area->setContentsMargins(0, 0, 0, 0);

    m_player = new QWoVncWidget(target, m_area);
    QObject::connect(m_player, SIGNAL(forceToClose()), this, SLOT(onForceToClose()));
    m_area->setWidget(m_player);
    m_player->resize(1024, 768);


    m_tool = new QWoVncToolForm(m_player, this);
    QObject::connect(m_tool, SIGNAL(enter()), this, SLOT(onToolEnter()));
    QObject::connect(m_tool, SIGNAL(leave()), this, SLOT(onToolLeave()));
    QObject::connect(m_tool, SIGNAL(ftpClicked()), this, SLOT(onFtpClicked()));
    m_tool->raise();

    QObject::connect(m_player, SIGNAL(screenCountChanged(int)), m_tool, SLOT(onScreenCountChanged(int)));
    QObject::connect(m_player, SIGNAL(messageSupport(int)), m_tool, SLOT(onMessageSupport(int)));

    QSize sz = m_tool->sizeHint();
    m_tool->setGeometry(QRect((sz.width() - sz.width()) / 2, -1, sz.width(), sz.height()));
    QObject::connect(m_tool, SIGNAL(qualityLevel(int)), m_player, SLOT(onSetQualityLevel(int)));
    QObject::connect(m_tool, SIGNAL(sizeChanged(QSize)), this, SLOT(onToolSizeChanged(QSize)), Qt::QueuedConnection);
    QObject::connect(m_tool, SIGNAL(nextScreenRatio()), m_player, SLOT(onNetScreenRatioRequest()));
    QObject::connect(m_tool, SIGNAL(nextScreenMode()), m_player, SLOT(onNextScreenModeRequest()));
    QObject::connect(m_tool, SIGNAL(blackScreen()), m_player, SLOT(onBlackScreen()));
    QObject::connect(m_tool, SIGNAL(lockScreen()), m_player, SLOT(onLockScreen()));
}

QWoVncPlayWidget::~QWoVncPlayWidget()
{

}

bool QWoVncPlayWidget::smartResize()
{
    return m_player->smartResize();
}

void QWoVncPlayWidget::setSmartResize(bool on)
{
    m_player->setSmartResize(on);
}

bool QWoVncPlayWidget::isConnected()
{
    return m_player->isConnected();
}

void QWoVncPlayWidget::stop()
{
    if(m_player) {
        m_player->stop();
    }
}

void QWoVncPlayWidget::reconnect()
{
    if(m_player) {
        m_player->reconnect();
    }
}

void QWoVncPlayWidget::onToolEnter()
{
    QRect rt = m_tool->geometry();
    if(rt.top() >= 0) {
        return;
    }
    if(m_ani != nullptr) {
        m_ani->stop();
        m_ani->deleteLater();
    }
    QPropertyAnimation *animation = new QPropertyAnimation(m_tool, "pos");
    animation->setObjectName("Enter");
    QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    QObject::connect(animation, SIGNAL(finished()), this, SLOT(onDeleteLater()));
    m_ani = animation;
    QPoint pt = rt.topLeft();
    animation->setDuration(DURATION_DEFAULT);
    animation->setStartValue(pt);
    QSize sz = size();
    pt.setX((sz.width() - rt.width()) / 2);
    pt.setY(-1);
    animation->setEndValue(pt);
    animation->start();
}

void QWoVncPlayWidget::onToolLeave()
{
    QRect rt = m_tool->geometry();
    int y = -rt.height() + 5;
    if(rt.top() <= y) {
        return;
    }
    if(m_ani != nullptr) {
        m_ani->stop();
        m_ani->deleteLater();
    }
    QPropertyAnimation *animation = new QPropertyAnimation(m_tool, "pos");
    animation->setObjectName("Leave");
    QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    QObject::connect(animation, SIGNAL(finished()), this, SLOT(onDeleteLater()));
    m_ani = animation;
    QPoint pt = m_tool->geometry().topLeft();
    animation->setDuration(DURATION_DEFAULT);
    animation->setStartValue(pt);
    QSize sz = size();
    pt.setX((sz.width() - rt.width()) / 2);
    pt.setY(y);
    animation->setEndValue(pt);
    animation->start();
}

void QWoVncPlayWidget::onFtpClicked()
{
    if(m_ftp == nullptr) {
        m_ftp = new QWoVncFtpWidget(m_player, this);
        m_ftp->resize(800, 600);
        m_ftp->hide();
    }
    QSize szFtp = m_ftp->size();
    QSize sz = size();
    int x = (sz.width() - szFtp.width()) / 2;
    int y = (sz.height() - szFtp.height()) / 2;
    m_ftp->setGeometry(x, y, szFtp.width(), szFtp.height());
    m_ftp->show();
}

void QWoVncPlayWidget::onDeleteLater()
{
    QObject *obj = sender();
    qDebug() << "onDeleteLater" << obj->objectName();
}

void QWoVncPlayWidget::onToolSizeChanged(const QSize &sz)
{

}

void QWoVncPlayWidget::onForceToClose()
{
    QWoShower::forceToCloseTopLevelWidget(this);
}

void QWoVncPlayWidget::resizeEvent(QResizeEvent *e)
{
    QSize sz = e->size();
    QWidget::resizeEvent(e);
    updateToolbarPosition();
}

void QWoVncPlayWidget::updateToolbarPosition()
{
    QSize szWin = size();
    m_area->setGeometry(QRect(QPoint(0, 0), szWin));
    QRect rt = m_tool->geometry();
    int y = rt.top();
    if(m_ani != nullptr) {
        QPoint pt = m_ani->endValue().toPoint();
        pt.setX((szWin.width() - rt.width()) / 2);
        m_ani->setEndValue(pt);
    }else{
        m_tool->move((szWin.width() - rt.width()) / 2, y);
    }
}
