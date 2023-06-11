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

#include "qwoplaybookconfiguredialog.h"
#include "ui_qwoplaybookconfiguredialog.h"

#include "qwosetting.h"
#include "qkxmessagebox.h"

#include <QFileDialog>

QWoPlaybookConfigureDialog::QWoPlaybookConfigureDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoPlaybookConfigureDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->path->setReadOnly(true);
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApply()));
    QObject::connect(ui->btnChoose, SIGNAL(clicked()), this, SLOT(onBrowserButtonClicked()));

    setMinimumWidth(450);

    setWindowTitle(tr("Playbook configure"));

    QString customPath = QWoSetting::customPlaybooksPath();
    ui->path->setText(customPath);

    adjustSize();
}

QWoPlaybookConfigureDialog::~QWoPlaybookConfigureDialog()
{
    delete ui;
}

QString QWoPlaybookConfigureDialog::path() const
{
    return ui->path->text();
}

void QWoPlaybookConfigureDialog::onApply()
{
    QString path = ui->path->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The custom path should not empty."));
        return;
    }
    QWoSetting::setCustomPlaybooksPath(path);
    done(QDialog::Accepted+1);
}

void QWoPlaybookConfigureDialog::onBrowserButtonClicked()
{
    QString customPath = QWoSetting::customPlaybooksPath();
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Choose directory"), customPath);
    if(filePath.isEmpty()) {
        return;
    }
    ui->path->setText(filePath);
}
