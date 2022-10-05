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

#include "qwocommandlineinput.h"
#include "ui_qwocommandlineinput.h"

QWoCommandLineInput::QWoCommandLineInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QWoCommandLineInput)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::gray);
    pal.setColor(QPalette::Window, Qt::gray);
    setPalette(pal);

    QObject::connect(ui->input, SIGNAL(returnPressed()), this, SLOT(onInputReturnPressed()));
    QObject::connect(ui->close, SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));
}

QWoCommandLineInput::~QWoCommandLineInput()
{
    delete ui;
}

void QWoCommandLineInput::onInputReturnPressed()
{
    QString txt = ui->input->text();
    if(txt.isEmpty()) {
        emit returnPressed("\r");
        return;
    }
    ui->input->setText("");
    emit returnPressed(txt + "\r");
}

void QWoCommandLineInput::onCloseButtonClicked()
{
    hide();
}
