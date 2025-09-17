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

#include "qwoidentifykeycontentdialog.h"
#include "ui_qwoidentifykeycontentdialog.h"

#include <QClipboard>

QWoIdentifyKeyContentDialog::QWoIdentifyKeyContentDialog(bool isPublicKey, const QString& key, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyKeyContentDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(isPublicKey ? tr("Public key") : tr("Private key"));
    ui->key->setReadOnly(true);
    ui->key->setText(key);
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(onCopy()));}

QWoIdentifyKeyContentDialog::~QWoIdentifyKeyContentDialog()
{
    delete ui;
}

void QWoIdentifyKeyContentDialog::onCopy()
{
    QString txt = ui->key->toPlainText();
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(txt);
}
