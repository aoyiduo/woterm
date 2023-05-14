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

#include "qwosessiontoolconfiguredialog.h"
#include "ui_qwosessiontoolconfiguredialog.h"
#include "qkxmessagebox.h"
#include "qwosetting.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QVariantMap>

QWoSessionToolConfigureDialog::QWoSessionToolConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSessionToolConfigureDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Session tool configure"));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));
    QObject::connect(ui->btnTextBrowser, SIGNAL(clicked()), this, SLOT(onBrowserButtonClicked()));

    QVariantMap dm  = QWoSetting::value("sftpTool/textEditor").toMap();
    if(!dm.isEmpty()) {
        ui->textEditor->setText(dm.value("path").toString());
        ui->textArgs->setText(dm.value("arguments").toString());
    }
    adjustSize();
}

QWoSessionToolConfigureDialog::~QWoSessionToolConfigureDialog()
{
    delete ui;
}

void QWoSessionToolConfigureDialog::onSaveButtonClicked()
{
    QString params = ui->textArgs->text();
    if(!params.contains("{file}")) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The startup parameters does not contain a %1 placeholder").arg("{file}"));
        return;
    }
    QString path = ui->textEditor->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The text editor parameter should not be empty."));
        return;
    }
    QVariantMap dm;
    dm.insert("path", path);
    dm.insert("arguments", params);
    QWoSetting::setValue("sftpTool/textEditor", dm);
    done(QDialog::Accepted);
}

void QWoSessionToolConfigureDialog::onBrowserButtonClicked()
{
    QVariantMap editor = QWoSetting::value("sftpTool/textEditor").toMap();
    QString file = QFileDialog::getOpenFileName(this, tr("Sftp file editor"), editor.value("path").toString());
    if(file.isEmpty()) {
        return;
    }
    QFileInfo fi(file);
    if(!fi.isExecutable()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The file does not have execute permission, please reselect."));
        return;
    }
    ui->textEditor->setText(file);
}
