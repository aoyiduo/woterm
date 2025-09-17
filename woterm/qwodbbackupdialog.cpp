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

#include "qwodbbackupdialog.h"
#include "ui_qwodbbackupdialog.h"

#include "qkxbuttonassist.h"

#include "qwosetting.h"
#include "qwosshconf.h"
#include "qkxmessagebox.h"

#include <QFileDialog>
#include <QDebug>

QWoDbBackupDialog::QWoDbBackupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDbBackupDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Database backup"));
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Database Restore"));

    ui->filePath->setReadOnly(true);
    QKxButtonAssist *assist = new QKxButtonAssist("../private/skins/black/folder.png", ui->filePath);
    QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    QObject::connect(ui->btnBackup, SIGNAL(clicked()), this, SLOT(onBackupButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    adjustSize();
}

QWoDbBackupDialog::~QWoDbBackupDialog()
{
    delete ui;
}

void QWoDbBackupDialog::onBackupButtonClicked()
{
    QString fileName = ui->filePath->text();
    if(fileName.isEmpty()) {
        QKxMessageBox::warning(this, tr("Information"), tr("Input a file path to backup."));
        return;
    }
    if(!fileName.endsWith(".db")) {
        fileName += ".db";
    }
    QFileInfo fi(fileName);
    QString last = fi.absolutePath();
    QWoSetting::setLastBackupPath(last);
    if(!QWoSshConf::instance()->backup(fileName)) {
        QKxMessageBox::warning(this, tr("Failure"), tr("failed to backup the session list."));
        return;
    }
    QKxMessageBox::warning(this, tr("Success"), tr("backup success."));
}

void QWoDbBackupDialog::onAssistButtonClicked(int idx)
{
    QString path = QWoSetting::lastBackupPath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Session Database"), path, "SQLite3 (*.db)");
    if(fileName.isEmpty()) {
        return;
    }
    if(!fileName.endsWith(".db")) {
        fileName += ".db";
    }
    ui->filePath->setText(QDir::toNativeSeparators(fileName));
}
