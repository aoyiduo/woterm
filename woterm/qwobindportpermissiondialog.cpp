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

#include "qwobindportpermissiondialog.h"
#include "ui_qwobindportpermissiondialog.h"
#include "qwoutils.h"

QWoBindPortPermissionDialog::QWoBindPortPermissionDialog(const QString& target, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoBindPortPermissionDialog)
    , m_target(target)
{
    ui->setupUi(this);
    setWindowTitle(tr("Permission error"));
    // /sbin/sysctl -w net.ipv4.ip_unprivileged_port_start=0
    QObject::connect(ui->btnAuthorization, SIGNAL(clicked()), this, SLOT(onAuthorizationButtonClicked()));
    QObject::connect(ui->btnNewProcess, SIGNAL(clicked()), this, SLOT(onNewProcessButtonClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    adjustSize();
}

QWoBindPortPermissionDialog::~QWoBindPortPermissionDialog()
{
    delete ui;
}

void QWoBindPortPermissionDialog::onAuthorizationButtonClicked()
{
    if(QWoUtils::runAsRoot("/sbin/sysctl net.ipv4.ip_unprivileged_port_start=0")){
        done(QDialog::Accepted+1);
    }
}

void QWoBindPortPermissionDialog::onNewProcessButtonClicked()
{
    if(QWoUtils::openself("rlogin", m_target, true)) {
        done(QDialog::Accepted+2);
    }
}
