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

#include "qwoadmindialog.h"
#include "ui_qwoadmindialog.h"

#include "qwosetting.h"
#include "qkxbuttonassist.h"
#include "qkxmessagebox.h"

QWoAdminDialog::QWoAdminDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoAdminDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Administator"));
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    QKxButtonAssist *assist = new QKxButtonAssist("../private/skins/black/eye.png", ui->pass);
    QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    QObject::connect(ui->pass, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    QObject::connect(ui->btnPassReset, SIGNAL(clicked()), this, SLOT(onPasswordResetClicked()));
    QObject::connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->chkStartup, SIGNAL(clicked()), this, SLOT(onStartupClicked()));
    QObject::connect(ui->chkTunnelOpen, SIGNAL(clicked()), this, SLOT(onAllowConnectTunnelClicked()));
    QObject::connect(ui->chkInitWindow, SIGNAL(clicked()), this, SLOT(onTerminalOnAppStartClicked()));
    QObject::connect(ui->chkTunnelDaemon, SIGNAL(clicked()), this, SLOT(onTunnelDaemonClicked()));

    ui->pass->setEchoMode(QLineEdit::Password);
    QString pass = QWoSetting::adminPassword();
    ui->pass->setText(pass);

    ui->chkStartup->setChecked(QWoSetting::startupByAdmin());
    ui->chkPassword->setChecked(QWoSetting::lookupPasswordByAdmin());
    ui->chkTunnelOpen->setChecked(QWoSetting::allowOtherHostConnectToTunnel());
    ui->chkTunnelDaemon->setChecked(QWoSetting::tunnelRunAsDaemon());
    ui->chkInitWindow->setChecked(QWoSetting::localTerminalOnAppStart());

    adjustSize();
}

QWoAdminDialog::~QWoAdminDialog()
{
    delete ui;
}

void QWoAdminDialog::onAssistButtonClicked(int idx)
{
    QLineEdit::EchoMode mode =  ui->pass->echoMode();
    if(mode == QLineEdit::Password) {
        ui->pass->setEchoMode(QLineEdit::Normal);
    }else{
        ui->pass->setEchoMode(QLineEdit::Password);
    }
}

void QWoAdminDialog::onPasswordResetClicked()
{
    QString pass = ui->pass->text();
    if(pass.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The password should not empty."));
        return;
    }
    QWoSetting::setAdminPassword(pass);
    onTextChanged(pass);
}

void QWoAdminDialog::onTextChanged(const QString &txt)
{
    QString pass = QWoSetting::adminPassword();
    ui->btnPassReset->setEnabled(pass != txt);
}

void QWoAdminDialog::onStartupClicked()
{
    QWoSetting::setStartupByAdmin(ui->chkStartup->isChecked());
}

void QWoAdminDialog::onLookupPasswordClicked()
{
    QWoSetting::setLookupPasswordByAdmin(ui->chkPassword->isChecked());
}

void QWoAdminDialog::onAllowConnectTunnelClicked()
{
    QWoSetting::setAllowOtherHostConnectToTunnel(ui->chkTunnelOpen->isChecked());
}

void QWoAdminDialog::onTerminalOnAppStartClicked()
{
    QWoSetting::setLocalTerminalOnAppStart(ui->chkInitWindow->isChecked());
}

void QWoAdminDialog::onTunnelDaemonClicked()
{
    QWoSetting::setTunnelRunAsDaemon(ui->chkTunnelDaemon->isChecked());
}
