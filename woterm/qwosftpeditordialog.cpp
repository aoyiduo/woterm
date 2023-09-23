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
        QString fileName = fi.fileName();
        int pos = fileName.indexOf('-');
        QString sessionName = fileName.left(pos);
        QString session = QByteArray::fromHex(sessionName.toLatin1());
        fileName = fileName.replace(0, pos, session);
        m_watchs.insert(fileName, file);
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
