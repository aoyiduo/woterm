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

#include "qwodbrestoredialog.h"
#include "ui_qwodbrestoredialog.h"
#include "qkxbuttonassist.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwohostlistmodel.h"
#include "qwohosttreemodel.h"
#include "qkxmessagebox.h"

#include <QFileDialog>
#include <QTimer>
#include <QDir>


QWoDBRestoreDialog::QWoDBRestoreDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDBRestoreDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Database Restore"));

    ui->filePath->setReadOnly(true);
    QKxButtonAssist *assist = new QKxButtonAssist("../private/skins/black/folder.png", ui->filePath);
    QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    QObject::connect(ui->btnRestore, SIGNAL(clicked()), this, SLOT(onRestoreButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    adjustSize();
}

QWoDBRestoreDialog::~QWoDBRestoreDialog()
{
    delete ui;
}

void QWoDBRestoreDialog::onAssistButtonClicked(int idx)
{
    QString path = QWoSetting::lastBackupPath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Restore Session Database"), path, "SQLite3 (*.db *.bak)");
    if(fileName.isEmpty()) {
        return;
    }
    ui->filePath->setText(QDir::toNativeSeparators(fileName));
}

void QWoDBRestoreDialog::onRestoreButtonClicked()
{
    QString path = ui->filePath->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Restore information"), tr("please select a backup file to restore"));
        return;
    }
    if(!QWoSshConf::databaseValid(path)) {
        QKxMessageBox::information(this, tr("Restore information"), tr("The backup file is incorrect or corrupt"));
        return;
    }
    if(QWoSshConf::instance()->restore(path)) {
        QKxMessageBox::information(this, tr("Restore information"), tr("success to restore database."));
    }else{
        QKxMessageBox::information(this, tr("Restore information"), tr("failed to restore database."));
    }
}
