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

#include <QStringListModel>
#include <QMessageBox>

#include <libssh/libssh.h>

QWoIdentifyCreateDialog::QWoIdentifyCreateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyCreateDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("IdentifyCreate"));

    QStringList ways;
    ways.append("RSA");
    //ways.append("ECDSA");
    //ways.append("ED25519");
    ui->type->setModel(new QStringListModel(ways, this));
    QObject::connect(ui->type, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onTypeCurrentIndexChanged(const QString&)));
    ui->type->setCurrentIndex(0);
    QStringList bits;
    bits.append("1024");
    bits.append("2048");
    bits.append("4096");
    ui->bits->setModel(new QStringListModel(bits, this));
    onTypeCurrentIndexChanged(ui->type->currentText());

    QObject::connect(ui->btnCreate, SIGNAL(clicked()), this, SLOT(onButtonCreateClicked()));
}

QWoIdentifyCreateDialog::~QWoIdentifyCreateDialog()
{
    delete ui;
}

void QWoIdentifyCreateDialog::onTypeCurrentIndexChanged(const QString &way)
{
    QAbstractItemModel *model = ui->bits->model();
    if(way == "RSA") {
        QStringList bits;
        bits.append("1024");
        bits.append("2048");
        bits.append("4096");
        ui->bits->setModel(new QStringListModel(bits, this));
    }else if(way == "ECDSA") {
        QStringList bits;
        bits.append("256");
        bits.append("384");
        bits.append("521");
        ui->bits->setModel(new QStringListModel(bits, this));
    }else if(way == "ED25519") {
        QStringList bits;
        bits.append("256");
        ui->bits->setModel(new QStringListModel(bits, this));
    }
    ui->bits->setCurrentIndex(0);
    if(model) {
        model->deleteLater();
    }
}

void QWoIdentifyCreateDialog::onButtonCreateClicked()
{
    QString name = ui->name->text();
    if(name.isEmpty()) {
        QMessageBox::warning(this, "Warning", "The name should not be empty", QMessageBox::Ok);
        return;
    }
    ssh_keytypes_e way = SSH_KEYTYPE_RSA;
    QString type = ui->type->currentText();
    if(type == "RSA") {
        way = SSH_KEYTYPE_RSA;
    }else if(type =="ECDSA") {
        way = SSH_KEYTYPE_ECDSA;
    }else{
        way = SSH_KEYTYPE_ED25519;
    }
    int bits = ui->bits->currentText().toInt();
    ssh_key key = nullptr;
    int err = ssh_pki_generate(way, bits, &key);
    if(err != SSH_OK) {
        QMessageBox::information(this, tr("info"), QString(tr("failed to create identify:[%1]")).arg(name));
        return ;
    }
    QString file = QWoSetting::identifyFilePath() + "/" + QWoUtils::nameToPath(name);
    QByteArray path = file.toLocal8Bit();
    err = ssh_pki_export_privkey_file(key, nullptr, nullptr, nullptr, path);
    ssh_key_free(key);
    done(QDialog::Accepted);
}
