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

#include "qwosessionfileassociationadddialog.h"
#include "ui_qwosessionfileassociationadddialog.h"

#include "qkxmessagebox.h"
#include <QFileDialog>
#include <QIntValidator>

QWoSessionFileAssociationAddDialog::QWoSessionFileAssociationAddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSessionFileAssociationAddDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("File association"));

    ui->priority->setText("0");
    ui->priority->setValidator(new QIntValidator(0, 65535));

    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));
    QObject::connect(ui->btnAppFind, SIGNAL(clicked()), this, SLOT(onAppFindButtonClicked()));
    adjustSize();
}

QWoSessionFileAssociationAddDialog::~QWoSessionFileAssociationAddDialog()
{
    delete ui;
}

void QWoSessionFileAssociationAddDialog::init(const QString &fileTyps, const QString &app, const QString &params, int priority)
{
    ui->fileTypes->setText(fileTyps);
    ui->application->setText(app);
    ui->parameter->setText(params);
    ui->priority->setText(QString::number(priority));
}

QString QWoSessionFileAssociationAddDialog::fileTypes() const
{
    return ui->fileTypes->text();
}

QString QWoSessionFileAssociationAddDialog::application() const
{
    return ui->application->text();
}

QString QWoSessionFileAssociationAddDialog::parameter() const
{
    return ui->parameter->text();
}

int QWoSessionFileAssociationAddDialog::priority() const
{
    return ui->priority->text().toInt();
}

void QWoSessionFileAssociationAddDialog::onSaveButtonClicked()
{
    QString params = ui->parameter->text();
    if(!params.contains("{file}")) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The start parameter does not contain a %1 placeholder").arg("{file}"));
        return;
    }
    QString priority = ui->priority->text();
    if(priority.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The priority parameter should be a number bigger than zero"));
        return;
    }
    QString path = ui->application->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The application parameter should not be empty."));
        return;
    }

    QFileInfo fi(path);
    if(!fi.isExecutable()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The application is no permission to execute it"));
        return;
    }

    QString suffix = ui->fileTypes->text();
    if(suffix.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The file types parameter should not be empty."));
        return;
    }
    suffix = suffix.replace(" ","");
    if(suffix != "*") {
        if(suffix.contains("*")) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("Cannot be mixed with the star[*] symbol."));
            return;
        }
        QStringList all = suffix.split(',');
        for(auto it = all.begin(); it != all.end(); it++) {
            QString typ = *it;
            typ = typ.simplified();
            if(!typ.startsWith('.')) {
                QKxMessageBox::information(this, tr("Parameter error"), tr("The format of the suffix name[%1] is incorrect and must start with a [.] character like [.%1].").arg(typ));
                return;
            }
        }
    }
    done(QDialog::Accepted+1);
}

void QWoSessionFileAssociationAddDialog::onAppFindButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Execute file"), ui->application->text());
    if(path.isEmpty()) {
        return;
    }
    QFileInfo fi(path);
    if(!fi.isExecutable()) {
        QKxMessageBox::information(this, tr("Execute program"), tr("No permission to execute it"));
        return;
    }
    ui->application->setText(fi.absoluteFilePath());
}
