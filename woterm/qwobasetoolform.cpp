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

#include "qwobasetoolform.h"
#include "ui_qwobasetoolform.h"
#include "qwotermwidget.h"
#include "qwofloatwindow.h"
#include "qwomainwindow.h"
#include "qwotermwidgetimpl.h"
#include "qwoshower.h"

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QMenu>
#include <QToolButton>
#include <QStyleOption>

QWoBaseToolForm::QWoBaseToolForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoBaseToolForm)
{
    ui->setupUi(this);
    setObjectName("deskopTool");
    setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(6);

    QObject::connect(ui->btnExitFull, SIGNAL(clicked(bool)), this, SLOT(onExitFullScreen()));
    QObject::connect(ui->btnFullScreen, SIGNAL(clicked(bool)), this, SLOT(onFullScreen()));
    QObject::connect(ui->btnMinimize, SIGNAL(clicked(bool)), this, SLOT(onMinimizeWindow()));
    QObject::connect(ui->btnReturn, SIGNAL(clicked(bool)), this, SLOT(onReturnTabWindow()));
    QObject::connect(ui->btnClose, SIGNAL(clicked(bool)), this, SLOT(onCloseWindow()));

    parent->installEventFilter(this);

    QObject::connect(QWoMainWindow::shower(), SIGNAL(floatChanged(QWoShowerWidget*,bool)), this, SLOT(onShowerFloatChanged(QWoShowerWidget*,bool)));
}

QWoBaseToolForm::~QWoBaseToolForm()
{
    delete ui;
}

QSize QWoBaseToolForm::sizeHint() const
{
    QSize sz = QWidget::sizeHint();
    return sz;
}

void QWoBaseToolForm::onExitFullScreen()
{
    QWidget *w = topLevelWidget();
    w->showNormal();
    // on mac or x11 window. the window state is not sync for some reason.
    // should follow code to try sync it.
    QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
}

void QWoBaseToolForm::onReturnTabWindow()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    wfloat->restoreToShower();
    qDebug() << "onReturnTabWindow";
}

void QWoBaseToolForm::onFullScreen()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    if(wfloat != nullptr) {
        wfloat->showFullScreen();
        QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
        return;
    }
    QWidget *parent = parentWidget();
    QWoShowerWidget *shower = qobject_cast<QWoShowerWidget*>(parent);
    while(parent != nullptr && shower == nullptr){
        parent = parent->parentWidget();
        shower = qobject_cast<QWoShowerWidget*>(parent);
    }
    if(shower != nullptr) {
        QMetaObject::invokeMethod(QWoMainWindow::shower(), "floatSession", Q_ARG(QWoShowerWidget*, shower), Q_ARG(bool, true));
    }
}

void QWoBaseToolForm::onMinimizeWindow()
{
    QWidget *w = topLevelWidget();
    w->showMinimized();
}

void QWoBaseToolForm::onCloseWindow()
{
    QWoShower::forceToCloseTopLevelWidget(this);
}

void QWoBaseToolForm::onShowerFloatChanged(QWoShowerWidget *shower, bool bFloat)
{
    QWoTermWidgetImpl *impl = qobject_cast<QWoTermWidgetImpl*>(shower);
    if(impl == nullptr) {
        return;
    }
    QWidget *topLevel = topLevelWidget();
    topLevel->installEventFilter(this);
    QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
    QWoBaseToolForm *form = impl->findChild<QWoBaseToolForm*>();
    if(form == this) {
        ui->btnReturn->setVisible(bFloat);
    }
}

void QWoBaseToolForm::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

void QWoBaseToolForm::enterEvent(QEvent *e)
{
    emit enter();
}

void QWoBaseToolForm::leaveEvent(QEvent *e)
{
    emit leave();
}

bool QWoBaseToolForm::eventFilter(QObject *w, QEvent *e)
{
    QEvent::Type t = e->type();
    if(t == QEvent::Resize) {
        QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
    }
    return QWidget::eventFilter(w, e);
}

void QWoBaseToolForm::resizeEvent(QResizeEvent *e)
{
    emit sizeChanged(e->size());
    QWidget::resizeEvent(e);
}

void QWoBaseToolForm::updateStatus()
{
    QWidget *w = topLevelWidget();
    ui->btnExitFull->setVisible(w->isFullScreen());
    ui->btnFullScreen->setVisible(!w->isFullScreen());
    resetLayout();
}

void QWoBaseToolForm::resetLayout()
{
    QSize sz = sizeHint();
    resize(sz);
}
