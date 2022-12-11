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

#include "qwoidentifycreatedialog.h"
#include "ui_qwoidentifycreatedialog.h"

#include "qwoutils.h"
#include "qwosetting.h"
#include "qwoidentify.h"
#include "qkxmessagebox.h"

#include <QStringListModel>
#include <QTimer>

#include <libssh/libssh.h>

QWoIdentifyCreateDialog::QWoIdentifyCreateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyCreateDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("IdentifyCreate"));

    QObject::connect(ui->btnCreate, SIGNAL(clicked()), this, SLOT(onButtonCreateClicked()));
    QObject::connect(ui->typRsa, SIGNAL(clicked()), this, SLOT(onButtonTypeRsaClicked()));
    QObject::connect(ui->typED25519, SIGNAL(clicked()), this, SLOT(onButtonTypeED25519Clicked()));
    ui->typED25519->setChecked(true);
    ui->boxBits->setVisible(false);
    ui->bit2048->setChecked(true);
    QTimer::singleShot(0, this, SLOT(onAdjustSize()));
}

QWoIdentifyCreateDialog::~QWoIdentifyCreateDialog()
{
    delete ui;
}

void QWoIdentifyCreateDialog::onButtonCreateClicked()
{
    QString name = ui->name->text();
    if(name.isEmpty()) {
        QKxMessageBox::warning(this, "Warning", "The name should not be empty", QMessageBox::Ok);
        return;
    }
    ssh_keytypes_e type = ui->typED25519->isChecked() ? SSH_KEYTYPE_ED25519 : SSH_KEYTYPE_RSA;
    int bits = ui->bit1024->isChecked() ? 1024 : 2048;
    ssh_key key = nullptr;
    int err = ssh_pki_generate(type, bits, &key);
    if(err != SSH_OK) {
        QKxMessageBox::information(this, tr("info"), QString(tr("failed to create identify:[%1]")).arg(name));
        return ;
    }
    char *b64 = nullptr;
    err = ssh_pki_export_privkey_base64(key, nullptr, nullptr, nullptr, &b64);
    QByteArray content(b64);
    ssh_key_free(key);
    if(err == SSH_OK) {
        ssh_string_free_char(b64);
    }
    if(!QWoIdentify::create(name, content)) {
        QKxMessageBox::information(this, tr("info"), QString(tr("failed to save identity for name already exist:[%1]")).arg(name));
        return ;
    }
    done(QDialog::Accepted);
}

void QWoIdentifyCreateDialog::onButtonTypeRsaClicked()
{
    ui->boxBits->setVisible(true);
    QTimer::singleShot(0, this, SLOT(onAdjustSize()));
}

void QWoIdentifyCreateDialog::onButtonTypeED25519Clicked()
{
    ui->boxBits->setVisible(false);
    QTimer::singleShot(0, this, SLOT(onAdjustSize()));
}

void QWoIdentifyCreateDialog::onAdjustSize()
{
    QSize sz = minimumSize();
    resize(sz);
}
