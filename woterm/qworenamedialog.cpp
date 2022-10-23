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

#include <QTimer>

QString QWoRenameDialog::open(const QString &name, QWidget *parent)
{
    QWoRenameDialog dlg(name, parent);
    dlg.exec();
    return dlg.result();
}

QString QWoRenameDialog::open(const QString &name, const QString &errMsg, QWidget *parent)
{
    QWoRenameDialog dlg(name, parent);
    dlg.setErrorMessage(errMsg);
    dlg.exec();
    return dlg.result();
}

QString QWoRenameDialog::open(const QString &name, const QString &title, const QString &errMsg, QWidget *parent)
{
    QWoRenameDialog dlg(name, parent);
    dlg.setWindowTitle(title);
    dlg.setErrorMessage(errMsg);
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
    ui->msgErr->setVisible(false);
    ui->msgErr->setText("");
    ui->name->setText(name);
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    setErrorMessage("");
}

QWoRenameDialog::~QWoRenameDialog()
{
    delete ui;
}

void QWoRenameDialog::setErrorMessage(const QString &msg)
{
    ui->msgErr->setText(msg);
    ui->msgErr->setVisible(!msg.isEmpty());
    QTimer::singleShot(0, this, SLOT(onAdjustSize()));
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

void QWoRenameDialog::onAdjustSize()
{
    adjustSize();
}
