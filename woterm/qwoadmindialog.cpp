/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
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
    QObject::connect(ui->chkLocalshell, SIGNAL(clicked()), this, SLOT(onLocalTerminalOnAppStartClicked()));
    QObject::connect(ui->chkSerialport, SIGNAL(clicked()), this, SLOT(onSerialportOnAppStartClicked()));
    QObject::connect(ui->chkTunnelDaemon, SIGNAL(clicked()), this, SLOT(onTunnelDaemonClicked()));
    QObject::connect(ui->chkOpacity, SIGNAL(clicked()), this, SLOT(onAllowMainOpacityClicked()));

    ui->pass->setEchoMode(QLineEdit::Password);
    QString pass = QWoSetting::adminPassword();
    ui->pass->setText(pass);

    ui->chkStartup->setChecked(QWoSetting::startupByAdmin());
    ui->chkPassword->setChecked(QWoSetting::lookupPasswordByAdmin());
    ui->chkTunnelOpen->setChecked(QWoSetting::allowOtherHostConnectToTunnel());
    ui->chkTunnelDaemon->setChecked(QWoSetting::tunnelRunAsDaemon());
    ui->chkLocalshell->setChecked(QWoSetting::localTerminalOnAppStart());
    ui->chkSerialport->setChecked(QWoSetting::serialportOnAppStart());
    ui->chkOpacity->setChecked(QWoSetting::allowToSetWindowOpacity());
    ui->chkSshAgent->setChecked(QWoSetting::allowToUseExternalIdentityFiles());
    QObject::connect(ui->chkSshAgent, &QCheckBox::clicked, this, [=](){
        QWoSetting::setAllowToUseExternalIdentityFiles(ui->chkSshAgent->isChecked());
    });

    ui->chkSftpOpenFile->setChecked(QWoSetting::allowSftpToOpenFile());
    QObject::connect(ui->chkSftpOpenFile, &QCheckBox::clicked, this, [=](){
        QWoSetting::setAllowSftpToOpenFile(ui->chkSftpOpenFile->isChecked());
    });

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

void QWoAdminDialog::onLocalTerminalOnAppStartClicked()
{
    QWoSetting::setLocalTerminalOnAppStart(ui->chkLocalshell->isChecked());
}

void QWoAdminDialog::onSerialportOnAppStartClicked()
{
    QWoSetting::setSerialportOnAppStart(ui->chkSerialport->isChecked());
}

void QWoAdminDialog::onTunnelDaemonClicked()
{
    QWoSetting::setTunnelRunAsDaemon(ui->chkTunnelDaemon->isChecked());
}

void QWoAdminDialog::onAllowMainOpacityClicked()
{
    QWoSetting::setAllowToSetWindowOpacity(ui->chkOpacity->isChecked());
}
