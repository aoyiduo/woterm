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

#include "qwodbsftpdetaildialog.h"
#include "ui_qwodbsftpdetaildialog.h"
#include "qwoidentifydialog.h"
#include "qwosetting.h"
#include "qwossh.h"
#include "qwoglobal.h"
#include "qwoidentify.h"
#include "qkxmessagebox.h"
#include "qkxbuttonassist.h"
#include "qwoutils.h"

#include <QStringListModel>
#include <QIntValidator>
#include <QTimer>
#include <QDir>

QWoDbSftpDetailDialog::QWoDbSftpDetailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDbSftpDetailDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Sftp server"));
    ui->identity->setReadOnly(true);
    ui->password->setEchoMode(QLineEdit::Password);
    QKxButtonAssist *assist = new QKxButtonAssist(":/woterm/resource/skin/eye.png", ui->password);
    QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    ui->type->setModel(new QStringListModel(QStringList() << tr("Password") << tr("Identity file"), ui->type));
    QObject::connect(ui->type, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTextChanged()));
    QObject::connect(ui->btnIdentify, SIGNAL(clicked()), this, SLOT(onIdentifyButtonClicked()));
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->btnTest, SIGNAL(clicked()), this, SLOT(onTestButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onTestTimeout()));

    QVariantMap dm = QWoSetting::value("DBBackup/sftpDetail").toMap();
    QString host = dm.value("host").toString();
    QString name = dm.value("name").toString();
    QString password = dm.value("password").toString();
    QString identity = dm.value("identity").toString();
    QString path = dm.value("path", "~/woterm_db_backup").toString();
    QString port = dm.value("port", 22).toString();
    if(!identity.isEmpty()) {
        ui->type->setCurrentIndex(1);
        ui->identity->setText(identity);
    }else{
        ui->type->setCurrentIndex(0);
        ui->password->setText(password);
    }
    ui->host->setText(host);
    ui->name->setText(name);
    ui->path->setText(path);
    ui->port->setText(port);
    ui->port->setValidator(new QIntValidator(1, 65535));
    onCurrentTextChanged();
}

QWoDbSftpDetailDialog::~QWoDbSftpDetailDialog()
{
    release();
    delete ui;
}

void QWoDbSftpDetailDialog::onCurrentTextChanged()
{
    int idx = ui->type->currentIndex();
    if(idx == 0) {
        ui->passArea->show();
        ui->identityArea->hide();
    }else{
        ui->passArea->hide();
        ui->identityArea->show();
    }
    adjustSize();
}

void QWoDbSftpDetailDialog::onIdentifyButtonClicked()
{
    QString name = QWoIdentifyDialog::open(false, this);
    if(name.isEmpty()) {
        return;
    }
    ui->identity->setText(name);
}

void QWoDbSftpDetailDialog::onTestButtonClicked()
{
    HostInfo hi;
    hi.host = ui->host->text();
    hi.user = ui->name->text();
    hi.port = ui->port->text().toInt();
    if(hi.host.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("the host parameter should not be empty"));
        return;
    }

    if(hi.user.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("the login name parameter should not be empty"));
        return;
    }

    if(ui->type->currentIndex() == 0) {
        hi.password = ui->password->text();
        if(hi.password.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("the password parameter should not be empty"));
            return;
        }
    }else{
        hi.identityFile = ui->identity->text();
        if(hi.identityFile.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("the identity file parameter should not be empty"));
            return;
        }
        IdentifyInfo info;
        if(!QWoIdentify::infomation(hi.identityFile.toUtf8(), &info)) {
            return;
        }
        hi.identityContent = info.prvKey;
    }

    QString path = ui->path->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("the save path parameter should not be empty"));
        return;
    }

    release();

    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));

    m_sftp->start(hi, 11);
    m_sftp->fileInfo(path);
    ui->btnTest->setEnabled(false);
    m_timer->start(1000 * 10);

    saveDetail();
}

void QWoDbSftpDetailDialog::onApplyButtonClicked()
{
    saveDetail();
    done(Accepted + 1);
}

void QWoDbSftpDetailDialog::onConnectionStart()
{

}

void QWoDbSftpDetailDialog::onConnectionFinished(bool ok)
{
    if(!ok) {
        release();
        QKxMessageBox::information(this, tr("Failure report"), tr("Failed"));
    }
}

void QWoDbSftpDetailDialog::onCommandStart(int type, const QVariantMap &userData)
{

}

void QWoDbSftpDetailDialog::onCommandFinish(int type, const QVariantMap &userData)
{
    QString reason = userData.value("reason").toString();
    if(type == 22) {
        // #define MT_FTP_MKPATH           (22)
        release();
        if(reason == "ok") {
            QKxMessageBox::information(this, tr("Success"), tr("Success to create it."));
        }else{
            QKxMessageBox::information(this, tr("Failure"), tr("Failed to create it, please do it by manual."));
        }
    }else if(type == 21) {
        // #define MT_FTP_FILE_INFO        (21)
        QVariantMap dm = userData.value("fileInfo").toMap();
        if(dm.isEmpty()) {
            if(reason != "fatal") {
                if(QKxMessageBox::information(this,
                                              tr("Failure"),
                                              tr("Failed to open the target path, try to create it?"),
                                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
                    QString path = ui->path->text();
                    m_sftp->mkPath(path, 0x1C0);
                }else{
                    release();
                }
            }else{
                release();
            }
        }else{
            release();
            QString type = dm.value("type").toString();
            if(type != "d") {
                QKxMessageBox::information(this, tr("Failure"), tr("Please check it again and make sure the target path is a directory not a file."));
            }else{
                QKxMessageBox::information(this, tr("Success"), tr("It's a valid path to backup file."));
            }
        }
    }else{
        release();
    }
}

void QWoDbSftpDetailDialog::onErrorArrived(const QString &err, const QVariantMap &userData)
{
    QKxMessageBox::information(this, tr("Error"), err);
    release();
}

void QWoDbSftpDetailDialog::onFinishArrived(int code)
{
    release();
}

void QWoDbSftpDetailDialog::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    release();
    QKxMessageBox::information(this, tr("Parmeter error"), tr("The account information error"));
}

void QWoDbSftpDetailDialog::onTestTimeout()
{
    release();
    QKxMessageBox::information(this, tr("Error"), tr("Test timeout"));
}

void QWoDbSftpDetailDialog::onAssistButtonClicked(int i)
{
    QLineEdit::EchoMode mode = ui->password->echoMode();
    if(mode == QLineEdit::Normal) {
        ui->password->setEchoMode(QLineEdit::Password);
    }else{
        QString pwdAdmin = QWoSetting::adminPassword();
        if(pwdAdmin.isEmpty()) {
            QKxMessageBox::information(this, tr("Administrator"), tr("Please create administrator's password first!"));
            return;
        }
        QString pass = QWoUtils::getPassword(this, tr("Please input the administrator's password"));
        if(pass.isEmpty()) {
            return;
        }
        if(pass != pwdAdmin) {
            QKxMessageBox::information(this, tr("Error"), tr("Password error!"));
            return;
        }
        ui->password->setEchoMode(QLineEdit::Normal);
        QTimer::singleShot(1000*5, this, SLOT(onSetEditToPasswordMode()));
    }
}

void QWoDbSftpDetailDialog::onSetEditToPasswordMode()
{
    ui->password->setEchoMode(QLineEdit::Password);
}

void QWoDbSftpDetailDialog::release()
{
    ui->btnTest->setEnabled(true);
    m_timer->stop();
    if(m_sftp) {
        m_sftp->stop();
        QWoSshFactory::instance()->release(m_sftp);
    }
    m_sftp = nullptr;
}

void QWoDbSftpDetailDialog::saveDetail()
{
    HostInfo hi;
    hi.host = ui->host->text();
    hi.user = ui->name->text();
    hi.port = ui->port->text().toInt();

    QVariantMap dm;
    dm.insert("host", ui->host->text());
    dm.insert("port", ui->port->text());
    dm.insert("name", ui->name->text());
    if(ui->type->currentIndex() == 0) {
        dm.insert("password", ui->password->text());
    }else{
        dm.insert("identity", ui->identity->text());
    }
    dm.insert("path", ui->path->text());
    QWoSetting::setValue("DBBackup/sftpDetail", dm);
}
