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

#include "qwounioninputconfiguredialog.h"
#include "ui_qwounioninputconfiguredialog.h"

QWoUnionInputConfigureDialog::QWoUnionInputConfigureDialog(bool isRN, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoUnionInputConfigureDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    adjustSize();

    QObject::connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(onYesButtonClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    ui->radEnterAsN->setChecked(!isRN);
    ui->radEnterAsRN->setChecked(isRN);
}

QWoUnionInputConfigureDialog::~QWoUnionInputConfigureDialog()
{
    delete ui;
}

bool QWoUnionInputConfigureDialog::isEnterRN()
{
    return ui->radEnterAsRN->isChecked();
}

void QWoUnionInputConfigureDialog::onYesButtonClicked()
{
    done(QDialog::Accepted);
}
