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

#include "qwofloatwindow.h"
#include "qwoshower.h"
#include "qwoshowerwidget.h"
#include "qwobasetoolform.h"

#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QDebug>

#define DURATION_DEFAULT        (150)

QWoFloatWindow::QWoFloatWindow(QWoShowerWidget *child, QWoShower *shower, EToolType type)
    : QWidget(nullptr)
    , m_child(child)
    , m_shower(shower)
    , m_type(type)
{
    setAttribute(Qt::WA_DeleteOnClose);
    child->setParent(this);
    child->show();
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(child);
    m_layout = layout;

    if(type != ETT_Vnc) {
        m_tool = new QWoBaseToolForm(this);
        QObject::connect(m_tool, SIGNAL(enter()), this, SLOT(onToolEnter()));
        QObject::connect(m_tool, SIGNAL(leave()), this, SLOT(onToolLeave()));
        m_tool->raise();
    }
    QObject::connect(child, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
    QObject::connect(child, SIGNAL(destroyed(QObject*)), this, SLOT(onDeleteLater()));
}

void QWoFloatWindow::restoreToShower()
{
    m_shower->restoreSession(m_child);
#ifdef Q_OS_MAC
    // must be show normal or will be black and no response to all operation, why?why?why?
    showNormal();
#endif
    hide();
    deleteLater();
}

void QWoFloatWindow::onToolEnter()
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

void QWoFloatWindow::onToolLeave()
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

void QWoFloatWindow::onDeleteLater()
{
    QObject *obj = sender();
    qDebug() << "onDeleteLater" << obj->objectName();
}

void QWoFloatWindow::resizeEvent(QResizeEvent *e)
{
    QSize sz = e->size();
    QWidget::resizeEvent(e);
    updateToolbarPosition();
}

void QWoFloatWindow::updateToolbarPosition()
{
    if(m_tool == nullptr) {
        return;
    }
    QSize szWin = size();
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
