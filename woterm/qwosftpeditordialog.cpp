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

#include "qwosftpeditordialog.h"
#include "ui_qwosftpeditordialog.h"

#include "qkxmessagebox.h"

#include <QDir>
#include <QStringListModel>

QWoSftpEditorDialog::QWoSftpEditorDialog(const QStringList &watchs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSftpEditorDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Edit file watch list"));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnClearAll, SIGNAL(clicked()), this, SLOT(onClearAllButtonClicked()));
    QObject::connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(onStopWatchButtonClicked()));
    QObject::connect(ui->btnFtpOptions, SIGNAL(clicked()), this, SIGNAL(ftpSettingArrived()));
    QObject::connect(ui->btnFtpOptions, SIGNAL(clicked()), this, SLOT(close()));
    for(auto it = watchs.begin(); it != watchs.end(); it++) {
        QString file = *it;
        QFileInfo fi(file);
        QString fileFull = fi.fileName();
        QStringList fns = fileFull.split('-');
        if(fns.length() >= 3) {
            QString pid = fns.takeFirst();
            QString sessionName = fns.takeFirst();
            QString fileName = fns.join('-');
            QString session = QByteArray::fromHex(sessionName.toLatin1());
            m_watchs.insert(session+"-"+fileName, file);
        }else{
            QString fileName = fi.fileName();
            m_watchs.insert(fileName, file);
        }
    }
    QStringListModel *model = new QStringListModel(m_watchs.keys(), ui->tasks);
    ui->tasks->setModel(model);
    ui->tasks->setEditTriggers(QListView::NoEditTriggers);
}

QWoSftpEditorDialog::~QWoSftpEditorDialog()
{
    delete ui;
}

void QWoSftpEditorDialog::onClearAllButtonClicked()
{
    emit clearAll();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->tasks->model());
    model->setStringList(QStringList());
}

void QWoSftpEditorDialog::onStopWatchButtonClicked()
{
    QModelIndex idx = ui->tasks->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("File watch"), tr("No selected items"));
        return;
    }
    QString fileName = idx.data(Qt::DisplayRole).toString();
    QString file = m_watchs.take(fileName);
    emit stopArrived(file);
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->tasks->model());
    model->setStringList(m_watchs.keys());
}
