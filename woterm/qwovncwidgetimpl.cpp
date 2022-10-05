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

#include "qwovncwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwovncplaywidget.h"
#include "qwosshconf.h"

#include <QVBoxLayout>
#include <QMenu>

QWoVncWidgetImpl::QWoVncWidgetImpl(const QString& target, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_rdp = new QWoVncPlayWidget(target, this);
    QObject::connect(m_rdp, SIGNAL(destroyed()), this, SLOT(onRootDestroy()));
    layout->addWidget(m_rdp);
    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
}

QWoVncWidgetImpl::~QWoVncWidgetImpl()
{

}

bool QWoVncWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoVncWidgetImpl::handleTabContextMenu(QMenu *menu)
{
}

QMap<QString, QString> QWoVncWidgetImpl::collectUnsafeCloseMessage()
{
    return QMap<QString, QString>();
}

void QWoVncWidgetImpl::onRootDestroy()
{
    deleteLater();
}
