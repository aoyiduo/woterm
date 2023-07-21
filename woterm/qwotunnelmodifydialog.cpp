/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
