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

#include "qwoserialinput.h"
#include "ui_qwoserialinput.h"

#include <QSerialPortInfo>
#include <QStringListModel>

QWoSerialInput::QWoSerialInput(const QString& target, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QWoSerialInput)
{
    ui->setupUi(this);
    ui->panel->setFixedWidth(250);
    onBtnRefleshClicked();
    QObject::connect(ui->btnReflesh, SIGNAL(clicked(bool)), this, SLOT(onBtnRefleshClicked()));
    QObject::connect(ui->btnConnect, SIGNAL(clicked(bool)), this, SLOT(onBtnConnectCliecked()));
    QObject::connect(ui->btnDisconnect, SIGNAL(clicked(bool)), this, SLOT(onBtnDisconnectCliecked()));
    QObject::connect(ui->btnSend, SIGNAL(clicked(bool)), this, SLOT(onBtnSendClicked()));
    QObject::connect(ui->btnMore, SIGNAL(clicked(bool)), this, SIGNAL(moreReady()));
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
}

QWoSerialInput::~QWoSerialInput()
{
    delete ui;
}

bool QWoSerialInput::isTextMode()
{
    return ui->hex->isChecked();
}

bool QWoSerialInput::isAutoNewLine()
{
    return ui->line->isChecked();
}

void QWoSerialInput::reset()
{
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
}

void QWoSerialInput::onBtnRefleshClicked()
{
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    QList<QString> coms;
    for(int i = 0; i < infos.length(); i++) {
        coms.append(infos.at(i).portName());
    }
    ui->comx->setModel(new QStringListModel(coms, this));
    ui->tip->setVisible(coms.isEmpty());
}

void QWoSerialInput::onBtnConnectCliecked()
{
    if(ui->comx->currentText().isEmpty()) {
        return;
    }
    ui->btnConnect->setEnabled(false);
    ui->btnDisconnect->setEnabled(true);
    emit connectReady(ui->comx->currentText());
}

void QWoSerialInput::onBtnDisconnectCliecked()
{
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    emit disconnect();
}

void QWoSerialInput::onBtnSendClicked()
{
    QString txt = ui->edit->toPlainText();
    if(txt.isEmpty()) {
        return;
    }
    emit sendText(txt);
}
