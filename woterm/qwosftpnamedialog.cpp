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

#include "qwosftpnamedialog.h"
#include "ui_qwosftpnamedialog.h"


QWoSftpNameDialog::QWoSftpNameDialog(const QString &name, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSftpNameDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    setWindowTitle(tr("Rename"));

    ui->name->setText(name);
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}

QWoSftpNameDialog::~QWoSftpNameDialog()
{
    delete ui;
}

void QWoSftpNameDialog::setPrivateButtonVisible(bool on)
{
    ui->prv->setVisible(on);
}

QString QWoSftpNameDialog::result() const
{
    return m_result;
}

bool QWoSftpNameDialog::isPrivate() const
{
    return ui->prv->isChecked();
}

void QWoSftpNameDialog::onButtonSaveClicked()
{
    m_result = ui->name->text();
    if(m_result.isEmpty()) {
        return;
    }
    close();
}
