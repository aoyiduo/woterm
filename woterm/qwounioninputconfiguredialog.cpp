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
