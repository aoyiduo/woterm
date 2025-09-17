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

#include "qwoserialinputhistorydialog.h"
#include "ui_qwoserialinputhistorydialog.h"

#include "qwosetting.h"

#include <QStringListModel>

QWoSerialInputHistoryDialog::QWoSerialInputHistoryDialog(const QString &msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSerialInputHistoryDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("History library"));

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(onButtonAddClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onButtonSelectClicked()));

    QStringList all = QWoSetting::value("serialHistory").toStringList();
    QStringListModel *model = new QStringListModel(all, ui->history);
    ui->history->setModel(model);
    ui->history->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QObject::connect(ui->btnAdd, &QPushButton::clicked, this, [=](){
        QStringList all = model->stringList();
        if(all.contains(msg)) {
            return ;
        }
        all.append(msg);
        model->setStringList(all);
        QWoSetting::setValue("serialHistory", all);
    });
    QObject::connect(ui->btnRemove, &QPushButton::clicked, this, [=](){
        QStringList all = model->stringList();
        QModelIndex idx = ui->history->currentIndex();
        if(!idx.isValid()) {
            return ;
        }
        all.removeAt(idx.row());
        model->setStringList(all);
        QWoSetting::setValue("serialHistory", all);
    });

    QObject::connect(ui->btnSelect, &QPushButton::clicked, this, [=](){
        QStringList all = model->stringList();
        QModelIndex idx = ui->history->currentIndex();
        if(!idx.isValid()) {
            return ;
        }
        QString item = all.at(idx.row());
        emit messageArrived(item);
    });
}

QWoSerialInputHistoryDialog::~QWoSerialInputHistoryDialog()
{
    delete ui;
}
