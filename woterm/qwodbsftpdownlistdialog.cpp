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

#include "qwodbsftpdownlistdialog.h"
#include "ui_qwodbsftpdownlistdialog.h"

#include "qwodbsftpdownsync.h"
#include "qkxmessagebox.h"

#include <QStringListModel>

QString QWoDBSftpDownListDialog::result(QWoDBSftpDownSync *sync, QWidget *parent)
{
    QWoDBSftpDownListDialog dlg(sync, parent);
    if(dlg.exec() == QDialog::Accepted) {
        return dlg.m_fileName;
    }
    return "";
}

QWoDBSftpDownListDialog::QWoDBSftpDownListDialog(QWoDBSftpDownSync *sync, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoDBSftpDownListDialog)
    , m_sync(sync)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("Backup file list"));

    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onListItemDoubleClicked(QModelIndex)));
    QObject::connect(ui->btnReflesh, SIGNAL(clicked()), this, SLOT(onButtonRefleshClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onButtonSelectClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(m_sync, SIGNAL(listArrived(QStringList)), this, SLOT(onSyncListArrived(QStringList)));
    m_sync->listAll();
    adjustSize();
}

QWoDBSftpDownListDialog::~QWoDBSftpDownListDialog()
{
    delete ui;
}

void QWoDBSftpDownListDialog::onButtonRefleshClicked()
{
    m_sync->listAll();
}

void QWoDBSftpDownListDialog::onButtonSelectClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("Please select a backup file."));
        return;
    }
    m_fileName = idx.data().toString();
    done(QDialog::Accepted);
}

void QWoDBSftpDownListDialog::onSyncListArrived(const QStringList &fileNames)
{
    QAbstractItemModel *model = ui->listView->model();
    if(model) {
        model->deleteLater();
    }
    model = new QStringListModel(fileNames, this);
    ui->listView->setModel(model);
}

void QWoDBSftpDownListDialog::onListItemDoubleClicked(const QModelIndex &idx)
{
    QString name = idx.data().toString();
    if(name.isEmpty()) {
        return;
    }
    m_fileName = name;
    done(QDialog::Accepted);
}
