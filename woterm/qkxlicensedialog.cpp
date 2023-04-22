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

#include "qkxlicensedialog.h"
#include "ui_qkxlicensedialog.h"
#include "qkxlicense.h"

QKxLicenseDialog::QKxLicenseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKxLicenseDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("License"));

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnActivate, SIGNAL(clicked()), this, SLOT(onActivateButtonClicked()));

    adjustSize();
}

QKxLicenseDialog::~QKxLicenseDialog()
{
    delete ui;
}

void QKxLicenseDialog::onActivateButtonClicked()
{

}
