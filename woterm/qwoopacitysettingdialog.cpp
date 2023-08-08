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

#include "qwoopacitysettingdialog.h"
#include "ui_qwoopacitysettingdialog.h"

#include "qwosetting.h"
#include "qkxmessagebox.h"

QWoOpacitySettingDialog::QWoOpacitySettingDialog(bool turnOn, int opacity, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoOpacitySettingDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Main window Opacity setting"));

    QObject::connect(ui->chkOpacity, SIGNAL(clicked()), this, SLOT(onOpacityValueChanged()));
    QObject::connect(ui->opacity, SIGNAL(valueChanged(int)), this, SLOT(onOpacityValueChanged()));

    ui->chkOpacity->setChecked(turnOn);
    ui->opacity->setRange(50, 100);
    opacity = qBound(50, opacity, 100);
    ui->opacity->setValue(opacity);

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApply()));
}

QWoOpacitySettingDialog::~QWoOpacitySettingDialog()
{
    delete ui;
}

bool QWoOpacitySettingDialog::opacityTurnOn()
{
    return ui->chkOpacity->isChecked();
}

int QWoOpacitySettingDialog::opacityValue()
{
    return ui->opacity->value();
}

void QWoOpacitySettingDialog::onOpacityValueChanged()
{
    bool on = ui->chkOpacity->isChecked();
    int v = ui->opacity->value();
    ui->opacity->setEnabled(on);
    emit opacityChanged(on, v);
}

void QWoOpacitySettingDialog::onApply()
{
    if(!QWoSetting::allowToSetWindowOpacity()){
        QKxMessageBox::information(this, tr("Permission restrictions"), tr("Please enable allow to set the window opacity in the administrator dialog."));
        return;
    }
    done(QDialog::Accepted+1);
}
