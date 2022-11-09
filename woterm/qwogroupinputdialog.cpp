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

#include "qwogroupinputdialog.h"
#include "ui_qwogroupinputdialog.h"

#include <QIntValidator>

QWoGroupInputDialog::QWoGroupInputDialog(const QString& name, int order, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoGroupInputDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(name.isEmpty() ? tr("New group") : tr("Modify group"));
    ui->order->setValidator(new QIntValidator(0, 65535, this));
    ui->name->setText(name);
    ui->order->setText(QString::number(order));
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApplyClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}

QWoGroupInputDialog::~QWoGroupInputDialog()
{
    delete ui;
}

void QWoGroupInputDialog::onApplyClicked()
{
    if(ui->name->text().isEmpty()) {
        return;
    }
    emit apply(ui->name->text(), ui->order->text().toInt());
}
