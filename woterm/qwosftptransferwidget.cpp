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

#include "qwosftptransferwidget.h"
#include "ui_qwosftptransferwidget.h"

QWoSftpTransferWidget::QWoSftpTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QWoSftpTransferWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setVisible(false);

    ui->showArea->setContentsMargins(5, 5, 5, 5);
    ui->showArea->setStyleSheet("QWidget#showArea{border-radius:5px;background:lightGray;}");
    QObject::connect(ui->btnAbort, SIGNAL(clicked()), this, SLOT(onAbort()));
    ui->progress->setRange(0, 100);
}

QWoSftpTransferWidget::~QWoSftpTransferWidget()
{
    delete ui;
}

void QWoSftpTransferWidget::setTip(const QString &tip)
{
    ui->tip->setText(tip);
}

void QWoSftpTransferWidget::onAbort()
{
    setVisible(false);
    emit abort();
}

void QWoSftpTransferWidget::progress(int v)
{
    ui->progress->setValue(v);
}

void QWoSftpTransferWidget::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);
    ui->progress->setValue(0);
}
