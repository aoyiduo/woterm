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

#include "qwosftppermissiondialog.h"
#include "ui_qwosftppermissiondialog.h"
#include "qkxmessagebox.h"

QWoSftpPermissionDialog::QWoSftpPermissionDialog(const QString &path, const QString permission, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSftpPermissionDialog)
    , m_permissionOld(permission)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    ui->path->setReadOnly(true);
    ui->path->setText(path);
    ui->chkSubdirs->setVisible(permission.at(0) == QChar('d'));
    ui->chkSubdirs->setChecked(false);

    ui->chkURead->setChecked(permission.at(1) == 'r');
    ui->chkUWrite->setChecked(permission.at(2) == 'w');
    ui->chkUExecute->setChecked(permission.at(3) == 'x');

    ui->chkGRead->setChecked(permission.at(4) == 'r');
    ui->chkGWrite->setChecked(permission.at(5) == 'w');
    ui->chkGExecute->setChecked(permission.at(6) == 'x');

    ui->chkORead->setChecked(permission.at(7) == 'r');
    ui->chkOWrite->setChecked(permission.at(8) == 'w');
    ui->chkOExecute->setChecked(permission.at(9) == 'x');

    QObject::connect(ui->chkURead, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkUWrite, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkUExecute, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkGRead, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkGWrite, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkGExecute, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkORead, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkOWrite, SIGNAL(clicked()), this, SLOT(onPermissionSet()));
    QObject::connect(ui->chkOExecute, SIGNAL(clicked()), this, SLOT(onPermissionSet()));

    QString perm = QString::number(permissionToNumber(), 8);
    ui->permOld->setText(perm);
    ui->permNew->setText(perm);

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnCommit, SIGNAL(clicked()), this, SLOT(onSubmitButtonClicked()));
    adjustSize();
}

QWoSftpPermissionDialog::~QWoSftpPermissionDialog()
{
    delete ui;
}

int QWoSftpPermissionDialog::permssionResult(bool *subdirs) const
{
    if(subdirs != nullptr) {
        *subdirs = ui->chkSubdirs->isChecked();
    }
    return m_permissionResult;
}

void QWoSftpPermissionDialog::onSubmitButtonClicked()
{
    m_permissionResult = permissionToNumber();
    done(QDialog::Accepted + 1);
}

void QWoSftpPermissionDialog::onPermissionSet()
{
    QString perm = QString::number(permissionToNumber(), 8);
    ui->permNew->setText(perm);

    QCheckBox *btn = qobject_cast<QCheckBox*>(sender());
    if(m_permissionOld.at(0) == 'd') {
        bool warning = false;
        if(btn == ui->chkUExecute) {
            warning = !ui->chkUExecute->isChecked();
        }else if(btn == ui->chkGExecute) {
            warning = !ui->chkGExecute->isChecked();
        }else if(btn == ui->chkOExecute) {
            warning = !ui->chkOExecute->isChecked();
        }
        if(warning) {
            QKxMessageBox::information(this, tr("Permission information"), tr("If you cancel the execution permission, maybe you will not have permission to open this directory."));
        }
    }
}

int QWoSftpPermissionDialog::permissionToNumber()
{
    int permission = 0;
    permission |= ui->chkURead->isChecked() ? 1 << 8 : 0;
    permission |= ui->chkUWrite->isChecked() ? 1 << 7 : 0;
    permission |= ui->chkUExecute->isChecked() ? 1 << 6 : 0;
    permission |= ui->chkGRead->isChecked() ? 1 << 5 : 0;
    permission |= ui->chkGWrite->isChecked() ? 1 << 4 : 0;
    permission |= ui->chkGExecute->isChecked() ? 1 << 3 : 0;
    permission |= ui->chkORead->isChecked() ? 1 << 2 : 0;
    permission |= ui->chkOWrite->isChecked() ? 1 << 1 : 0;
    permission |= ui->chkOExecute->isChecked() ? 1 << 0 : 0;
    return permission;
}
