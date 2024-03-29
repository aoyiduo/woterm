﻿/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwolicenseactivatedialog.h"
#include "ui_qwolicenseactivatedialog.h"

#include "qkxbuttonassist.h"
#include "qkxver.h"
#include "qkxmessagebox.h"
#include "qkxhttpclient.h"
#include "qwosetting.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <QClipboard>
#include <QGuiApplication>
#include <QButtonGroup>
#include <QDateTime>
#include <QStyle>

QWoLicenseActivateDialog::QWoLicenseActivateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoLicenseActivateDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    setWindowTitle(tr("License Activation"));

    setMinimumWidth(360);

    QKxButtonAssist *btn = new QKxButtonAssist("../private/skins/black/ftp.png", false, ui->mid);
    QObject::connect(btn, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnClose2, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnActivate, SIGNAL(clicked()), this, SLOT(onActivateButtonClicked()));
    QObject::connect(ui->btnRedeem, SIGNAL(clicked()), this, SLOT(onRedeemButtonClicked()));
    QObject::connect(ui->radioKey, SIGNAL(clicked()), this, SLOT(onTypeButtonClicked()));
    QObject::connect(ui->radioCode, SIGNAL(clicked()), this, SLOT(onTypeButtonClicked()));
    QObject::connect(ui->chkKeyModify, SIGNAL(clicked()), this, SLOT(onKeyModifyButtonClicked()));
    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(ui->radioKey);
    group->addButton(ui->radioCode);
    ui->radioCode->setChecked(false);
    ui->radioKey->setChecked(true);
    ui->code->setReadOnly(false);
    QKxVer *ver = QKxVer::instance();
    ui->mid->setText(ver->machineID());
    ui->key->setText(ver->licenseKey());
    onTypeButtonClicked();
}

QWoLicenseActivateDialog::~QWoLicenseActivateDialog()
{
    delete ui;
}

void QWoLicenseActivateDialog::onAssistButtonClicked(int idx)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString mid = ui->mid->text();
    if(mid.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("Failed to get machine code, please check the network is workable."));
        return;
    }
    clipboard->setText(mid);
}

void QWoLicenseActivateDialog::onActivateButtonClicked()
{
    QString key = ui->key->text();
    if(key.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("Please input a valid License key first."));
        return;
    }

    QKxVer *ver = QKxVer::instance();

    if(key== "WoTerm.202309") {
        QString info = ver->macAddressesAsString();
        QKxMessageBox::information(this, tr("Device information"), info);
        return;
    }

    if(ver->machineID().isEmpty()) {
        QKxMessageBox::information(this, tr("Device information"), tr("Unable to activate version without machine code information."));
        return;
    }

    QString keyOld = ver->licenseKey();
    if(key == keyOld) {
        return;
    }

    QString errMsg;
    int licenseType;
    if(!ver->valid(key, licenseType, errMsg)) {
        QKxMessageBox::information(this, tr("Parameter error"), errMsg);
        return;
    }
    if(licenseType == QKxLicense::ETrialVersion) {
        if(!ver->parse(key, errMsg)) {
            QKxMessageBox::information(this, tr("Parameter error"), errMsg);
            return;
        }
        done(QDialog::Accepted+1);
        return;
    }

    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onValidResult(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    QJsonObject obj;
    obj.insert("mid", ver->machineID());
    obj.insert("info", ver->encryptKey(ver->machineID(), key));
    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray json = doc.toJson(QJsonDocument::Compact);
    http->post("http://key.woterm.com/valid", json, "application/json; charset=utf-8");
}

void QWoLicenseActivateDialog::onRedeemButtonClicked()
{
    QKxVer *ver = QKxVer::instance();
    QString code = ui->code->text();
    if(code.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("Please input a valid redeem code first."));
        return;
    }
    bool ok;
    if(code.toLongLong(&ok) == 0 || !ok) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("Please input a valid redeem code first."));
        return;
    }
    QStringList codes = QWoSetting::value("redeem/codes").toStringList();
    if(codes.contains(code)) {
        int retval = QKxMessageBox::information(this,
                                tr("Parameter error"),
                                tr("According to local access history, the code has already been used. Do you want to continue with this operation?"),
                                QMessageBox::Yes|QMessageBox::No);
        if(retval != QMessageBox::Yes) {
            return;
        }
    }
    bool noTip = ver->isFreeVersion() || ver->isExpired();
    if(!noTip) {
        int daysLeft = ver->expireDaysLeft();
        if(daysLeft > 366 * 3) {
            int retval = QKxMessageBox::information(this,
                        tr("Ultimate redeem information"),
                        tr("The validity period exceeds 3 years and cannot be further redeemed."));
            return;
        }
        if(ver->isUltimateVersion()) {
            int retval = QKxMessageBox::information(this,
                                                    tr("Ultimate redeem information"),
                                                    tr("There is currently %1 day left to expire. A valid redemption code will be extended for at least one year beyond the original remaining days. Do you want to continue.").arg(daysLeft),
                                                    QMessageBox::Yes|QMessageBox::No);
            if(retval != QMessageBox::Yes) {
                return;
            }
        }
    }


    tryToRedeemLicense(code, 3);
}

void QWoLicenseActivateDialog::onTypeButtonClicked()
{
    if(ui->radioCode->isChecked()) {
        ui->codeArea->setVisible(true);
        ui->keyArea->setVisible(false);
    }else{
        ui->codeArea->setVisible(false);
        ui->keyArea->setVisible(true);
    }
    QString key = ui->key->text();
    ui->key->setReadOnly(!key.isEmpty());
    ui->chkKeyModify->setVisible(!key.isEmpty());
    ui->chkKeyModify->setChecked(false);

    QStyle *style = ui->key->style();
    style->polish(ui->key);

    adjustSize();
}

void QWoLicenseActivateDialog::onKeyModifyButtonClicked()
{
    QString key = ui->key->text();
    ui->key->setReadOnly(!ui->chkKeyModify->isChecked());
    QStyle *style = ui->key->style();
    style->polish(ui->key);
}

void QWoLicenseActivateDialog::onRetryToRedeemLicense(int code, const QByteArray &body)
{
    ui->btnRedeem->setEnabled(true);
    if(code == 200) {
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if(doc.isEmpty()) {
            QKxMessageBox::information(this, tr("Redeem error"), tr("Try again later"));
            return;
        }
        QJsonObject obj = doc.object();
        QString err;
        int ierr = obj.value("error").toInt();
        if(ierr == 0 || ierr == 2) {
            QJsonObject keyInfo = obj.value("data").toObject();
            if(!keyInfo.isEmpty()) {
                QString key = keyInfo.value("key").toString();
                QDateTime used = QDateTime::fromSecsSinceEpoch(keyInfo.value("timeUsed").toInt());
                QString code = keyInfo.value("code").toString();
                QKxVer *ver = QKxVer::instance();
                QString keyUsed = ver->licenseKey();
                if(key != keyUsed) {
                    if(!ver->parse(key, err)) {
                        QKxMessageBox::information(this, tr("Redeem error"), tr("The code[%1] has been used on %2.").arg(code).arg(used.toString()));
                        return;
                    }
                    QStringList codes = QWoSetting::value("redeem/codes").toStringList();
                    if(!codes.contains(code)) {
                        codes.append(code);
                        QWoSetting::setValue("redeem/codes", codes);
                    }
                    QKxMessageBox::information(this, tr("Redeem sucess"), tr("Success to redeem license, please store the license key safely for reinstallation etc."));
                    done(QDialog::Accepted+1);
                    return;
                }
                QKxMessageBox::information(this, tr("Redeem error"), tr("The code[%1] has been used on %2.").arg(code).arg(used.toString()));
                return;
            }
        }
        switch (ierr) {
        case -1:
            err = tr("parameters is not complete.");
            break;
        case -2:
        case -3:
        case -4:
            err = tr("decode failure.");
            break;
        case -5:
            err = tr("it not a valid code.");
            break;
        case -6:
        case -7:
        case -8:
        case -9:
            err = tr("server database error");
            break;
        case 1:
            err = tr("This code has already been used, please input another one.");
            break;
        default:
            err = tr("unknow error");
            break;
        }
        QKxMessageBox::information(this, tr("Redeem error"), tr("Try again later for reason:")+err);
    }else{
        QKxHttpClient *http = qobject_cast<QKxHttpClient*>(sender());
        QString errMsg = QString::number(code);
        if(http) {
            QString err = http->lastErrorString();
            if(!err.isEmpty()) {
                errMsg = err;
            }
        }
        QKxMessageBox::information(this, tr("Redeem error"), tr("Try again later for error:%1").arg(errMsg));
    }
}

void QWoLicenseActivateDialog::onValidResult(int code, const QByteArray &body)
{
    if(code == 200) {
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if(doc.isEmpty()) {
            QKxMessageBox::information(this, tr("License error"), tr("Try again later"));
            return;
        }
        QJsonObject obj = doc.object();
        int ierr = obj.value("error").toInt();
        QString desc = obj.value("desc").toString();
        if(ierr == 0) {
            QString data = obj.value("data").toString();
            QKxVer *ver = QKxVer::instance();
            QString key = ver->decryptKey(ver->machineID(), data);
            QString errMsg;
            if(!ver->parse(key, errMsg)) {
                QKxMessageBox::information(this, tr("Parameter error"), errMsg);
                return;
            }
            done(QDialog::Accepted+1);
            return;
        }
        QKxMessageBox::information(this, tr("Server error"), tr("Server error:%1, %2!").arg(ierr).arg(desc));
        return;
    }
    QKxMessageBox::information(this, tr("Network error"), tr("Network error:%1, try it again!").arg(code));
}

void QWoLicenseActivateDialog::tryToRedeemLicense(const QString &code, int tryLeft)
{
    tryLeft--;
    if(tryLeft < 0) {
        return;
    }

    QKxVer *ver = QKxVer::instance();
    QString mid = ver->machineID();
    QString probeMid = ver->machineProbeID();
    if(probeMid.isEmpty()) {
        ver->probeBestMid();
        QKxMessageBox::information(this, tr("Machine information"), tr("Failed to generate the right machine id, please restart application or try it later."));
        return;
    }

    if(mid != probeMid) {
        if(!ver->forceProbeMidToMachineMid()) {
            QKxMessageBox::information(this, tr("Machine information"), tr("Failed to generate the right machine id, please restart application or try it later."));
            return;
        }
        QMetaObject::invokeMethod(this, "tryToRedeemLicense", Qt::QueuedConnection, Q_ARG(QString, code), Q_ARG(int, tryLeft));
        return;
    }

    QString info = ver->buildRedeemInformation(code);

    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onRetryToRedeemLicense(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    QJsonObject obj;
    obj.insert("mid", mid);
    obj.insert("info", info);
    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray json = doc.toJson(QJsonDocument::Compact);
    http->post("http://key.woterm.com/redeem/license", json, "application/json; charset=utf-8");
    ui->btnRedeem->setEnabled(false);
}
