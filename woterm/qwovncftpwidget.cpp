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

#include "qwovncftpwidget.h"
#include "ui_qwovncftpwidget.h"

#include "qkxvncwidget.h"
#include "qkxftptransferwidget.h"
#include "qkxftprequest.h"
#include "qwovncftprequest.h"
#include "qwosetting.h"

#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QBoxLayout>
#include <QDebug>


QWoVncFtpWidget::QWoVncFtpWidget(QKxVncWidget *vnc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoVncFtpWidget)
    , m_vnc(vnc)
{
    hide();
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->btnClose->setObjectName("btnClose");
    QObject::connect(ui->btnClose, SIGNAL(clicked(bool)), this, SLOT(onAboutToClose()));

    QObject::connect(vnc, SIGNAL(finished()), this, SLOT(onFinished()));

    QString path = QWoSetting::ftpTaskPath();
    QString hp = m_vnc->hostPort();
    m_transfer = new QKxFtpTransferWidget(path, hp, this);
    QBoxLayout *lo = qobject_cast<QBoxLayout*>(layout());
    lo->addWidget(m_transfer);

    QWoVncFtpRequest *ftp = new QWoVncFtpRequest(vnc, this);
    m_transfer->setRequest(ftp);
    m_ftpRequest = ftp;
}

QWoVncFtpWidget::~QWoVncFtpWidget()
{
    delete ui;
}

void QWoVncFtpWidget::onAboutToClose()
{
    close();
}

void QWoVncFtpWidget::onFinished()
{
    close();
}

void QWoVncFtpWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void QWoVncFtpWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void QWoVncFtpWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
