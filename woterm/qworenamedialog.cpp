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

#include "qworenamedialog.h"
#include "ui_qworenamedialog.h"

QString QWoRenameDialog::open(const QString &name, QWidget *parent)
{
    QWoRenameDialog dlg(name, parent);
    dlg.exec();
    return dlg.result();
}

QString QWoRenameDialog::open(const QString &name, const QString &title, QWidget *parent)
{
    QWoRenameDialog dlg(name, parent);
    dlg.setWindowTitle(title);
    dlg.exec();
    return dlg.result();
}

QWoRenameDialog::QWoRenameDialog(const QString &name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoRenameDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    setWindowTitle(tr("Rename"));

    ui->name->setText(name);
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}

QWoRenameDialog::~QWoRenameDialog()
{
    delete ui;
}

QString QWoRenameDialog::result() const
{
    return m_result;
}

void QWoRenameDialog::onButtonSaveClicked()
{
    m_result = ui->name->text();
    close();
}
