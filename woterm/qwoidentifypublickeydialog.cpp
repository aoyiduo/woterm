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

#include "qwoidentifypublickeydialog.h"
#include "ui_qwoidentifypublickeydialog.h"

#include <QClipboard>

QWoIdentifyPublicKeyDialog::QWoIdentifyPublicKeyDialog(const QString& key, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyPublicKeyDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Public Key"));
    ui->key->setReadOnly(true);
    ui->key->setText(key);
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(onCopy()));}

QWoIdentifyPublicKeyDialog::~QWoIdentifyPublicKeyDialog()
{
    delete ui;
}

void QWoIdentifyPublicKeyDialog::onCopy()
{
    QString txt = ui->key->toPlainText();
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(txt);
}
