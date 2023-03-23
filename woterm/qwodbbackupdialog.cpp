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

#include "qwodbbackupdialog.h"
#include "ui_qwodbbackupdialog.h"
#include "qwodbsftpdetaildialog.h"
#include "qwosetting.h"
#include "qkxmessagebox.h"
#include "qwosshconf.h"
#include "qwodbsftpuploadsync.h"

#include <QStringListModel>
#include <QTimer>
#include <QFileDialog>
#include <QFileInfo>

QWoDbBackupDialog::QWoDbBackupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDbBackupDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Database backup"));
    ui->backupType->setModel(new QStringListModel(QStringList() << tr("sftp server") << tr("local file"), this));
    QObject::connect(ui->backupType, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged()));
    onCurrentIndexChanged();
    QStringList crypts;
    crypts.append("AES-CBC-256");
    crypts.append("AES-CTR-256");
    crypts.append("AES-GCM-256");
    crypts.append("DES-CBC");
    crypts.append("DES-ECB");
    crypts.append("DES-OFB64");
    crypts.append("RC4");
    crypts.append("Blowfish");
    ui->cryptType->setModel(new QStringListModel(crypts, this));
    ui->sftpServer->setReadOnly(true);
    QObject::connect(ui->btnSftpDetail, SIGNAL(clicked()), this, SLOT(onSftpDetailButtonClicked()));

    QString lastFile = QWoSetting::value("DBBackup/lastLocalFile").toString();
    ui->pathLocal->setText(lastFile);
    QString cryptType = QWoSetting::value("DBBackup/lastCryptType").toString();
    if(cryptType.isEmpty()) {
        ui->cryptType->setCurrentIndex(0);
    }else{
        ui->cryptType->setCurrentText(cryptType);
    }
    QString cryptKey = QWoSetting::value("DBBackup/lastCryptKey").toString();
    ui->cryptKey->setText(cryptKey);

    resetSftpUrl();

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onFileSaveClicked()));
    QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onFileBrowserClicked()));
    QObject::connect(ui->btnUpload, SIGNAL(clicked()), this, SLOT(onFileUploadClicked()));
}

QWoDbBackupDialog::~QWoDbBackupDialog()
{
    delete ui;
}

void QWoDbBackupDialog::onCurrentIndexChanged()
{
    int idx = ui->backupType->currentIndex();
    if(idx == 0) {
        // sftp server.
        ui->sftpArea->show();
        ui->localArea->hide();
    }else{
        ui->sftpArea->hide();
        ui->localArea->show();
    }
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
}

void QWoDbBackupDialog::onAdjustLayout()
{
    adjustSize();
}

void QWoDbBackupDialog::onSftpDetailButtonClicked()
{
    QWoDbSftpDetailDialog dlg(this);
    if(dlg.exec() == (QDialog::Accepted + 1)) {
        resetSftpUrl();
    }
}

void QWoDbBackupDialog::onFileSaveClicked()
{
    QString lastFile = ui->pathLocal->text();
    if(lastFile.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("the local file should not be empty."));
        return;
    }
    QWoSetting::setValue("DBBackup/lastLocalFile", lastFile);
    if(!QWoSshConf::instance()->backup(lastFile)) {
        QKxMessageBox::warning(this, tr("Failure"), tr("failed to backup the database."));
    }else{
        QKxMessageBox::information(this, tr("Success"), tr("success to backup the file."));
    }
}

void QWoDbBackupDialog::onFileUploadClicked()
{
    QString cryptType = ui->cryptType->currentText();
    QString cryptKey = ui->cryptKey->text();
    if(cryptKey.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("the encryption key should not be empty."));
        return;
    }
    QWoSetting::setValue("DBBackup/lastCryptType", cryptType);
    QWoSetting::setValue("DBBackup/lastCryptKey", cryptKey);

    if(m_sync == nullptr) {
        m_sync = new QWoDBSftpUploadSync(this);
        QObject::connect(m_sync, SIGNAL(infoArrived(int,int,QString)), this, SLOT(onInfoArrived(int,int,QString)));
    }
    m_sync->upload(cryptType, cryptKey);
}

void QWoDbBackupDialog::onFileBrowserClicked()
{
    QString path = ui->pathLocal->text();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Local file"), path, tr("SQLite (*.db)"));
    if(fileName.isEmpty()) {
        return;
    }
    ui->pathLocal->setText(fileName);
}

void QWoDbBackupDialog::onInfoArrived(int action, int err, const QString &errDesc)
{
    ui->info->setText(errDesc);
}

void QWoDbBackupDialog::resetSftpUrl()
{
    QVariantMap dm = QWoSetting::value("DBBackup/sftpDetail").toMap();
    QString host = dm.value("host").toString();
    QString name = dm.value("name").toString();
    QString path = dm.value("path", "~/woterm_db_backup").toString();
    QString port = dm.value("port", 22).toString();
    QString url = QString("sftp://%1@%2:%3?port=%4").arg(name, host, path, port);
    ui->sftpServer->setText(url);
}
