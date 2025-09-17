/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwotunnelmodifydialog.h"
#include "ui_qwotunneladddialog.h"

QWoTunnelModifyDialog::QWoTunnelModifyDialog(const TunnelInfo &ti, QWidget *parent)
    : QWoTunnelAddDialog(parent)
{
    setWindowTitle(tr("Modify tunnel"));
    ui->btnAdd->setVisible(false);
    ui->btnModify->setVisible(true);

    m_id = ti.id;
    ui->type->setCurrentIndex(ti.isPortmapping ? 0 : 1);
    ui->hostLocal->setText(ti.hostLocal);
    ui->portLocal->setText(QString::number(ti.portLocal));
    ui->hostRemote->setText(ti.hostRemote);
    ui->portRemote->setText(QString::number(ti.portRemote));
    ui->sessionName->setText(ti.sessionName);
}
