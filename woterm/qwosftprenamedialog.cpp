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

#include "qwosftprenamedialog.h"
#include "ui_qwosftprenamedialog.h"

#include <QDir>
#include <QDebug>
#include <QStyle>

QWoSftpRenameDialog::QWoSftpRenameDialog(const QString &path, const QString &name, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSftpRenameDialog)
    , m_path(path)
    , m_nameOld(name)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("File rename"));
    ui->pathOld->setText(QDir::cleanPath(path+"/"+name));
    ui->pathOld->setReadOnly(true);
    ui->chkModify->setChecked(false);
    ui->pathSave->setText(path);
    ui->pathSave->setReadOnly(true);
    ui->nameNew->setText(name);
    ui->pathNew->setVisible(false);

    QObject::connect(ui->pathSave, SIGNAL(textChanged(QString)), this, SLOT(onSavePathChanged(QString)));
    QObject::connect(ui->nameNew, SIGNAL(textChanged(QString)), this, SLOT(onSavePathChanged(QString)));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnCommit, SIGNAL(clicked()), this, SLOT(onButtonCommitClicked()));
    QObject::connect(ui->chkModify, SIGNAL(clicked()), this, SLOT(onButtonPathModifyClicked()));
    adjustSize();
}

QWoSftpRenameDialog::~QWoSftpRenameDialog()
{
    delete ui;
}

QString QWoSftpRenameDialog::pathResult()
{
    return m_pathResult;
}

void QWoSftpRenameDialog::onButtonCommitClicked()
{
    QString path = ui->pathSave->text();
    QString name = ui->nameNew->text();
    m_pathResult = QDir::cleanPath(path + "/" + name);
    done(QDialog::Accepted+1);
}

void QWoSftpRenameDialog::onButtonPathModifyClicked()
{
    bool modify = !ui->chkModify->isChecked();
    qDebug() << "onButtonPathModifyClicked" << modify;
    ui->pathSave->setReadOnly(modify);
    QStyle *style = ui->pathSave->style();
    style->polish(ui->pathSave);
}

void QWoSftpRenameDialog::onSavePathChanged(const QString& _path)
{
    QString path = ui->pathSave->text();
    QString name = ui->nameNew->text();
    QString pathAbs = QDir::cleanPath(path + "/" + name);
    ui->pathNew->setText(pathAbs);
    ui->pathNew->setVisible(pathAbs != ui->pathOld->text());
    adjustSize();
}
