/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwolicensetrialapplydialog.h"
#include "ui_qwolicensetrialapplydialog.h"

#include "qkxhttpclient.h"
#include "qkxver.h"
#include "qkxmessagebox.h"
#include "qkxprocesslaunch.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

QWoLicenseTrialApplyDialog::QWoLicenseTrialApplyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoLicenseTrialApplyDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    setWindowTitle(tr("Trial license"));

    QStringList msgs;
    msgs << tr("Thank you very much for your use.");
    msgs << tr("In order to help you better understand the functions and usages of this software, we provide you with a free one month trial experience.");
    msgs << tr("If you like it, please purchase a license to support us.");
    msgs << tr("We will also work harder to make it more powerfull and give back your kindness.");
    ui->statement->setPlainText(msgs.join("\r\n"));
    ui->statement->setReadOnly(true);
    setMinimumWidth(360);
    QObject::connect(ui->btnRetry, SIGNAL(clicked()), this, SLOT(onRetryButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

    ui->btnRetry->setText(tr("Get a license"));
    adjustSize();
}

QWoLicenseTrialApplyDialog::~QWoLicenseTrialApplyDialog()
{
    delete ui;
}

void QWoLicenseTrialApplyDialog::onRetryButtonClicked()
{
    tryToGetLicense(3);
}

void QWoLicenseTrialApplyDialog::onRetryToGetLicense(int code, const QByteArray& body)
{
    if(ui->btnRetry->isVisible()) {
        ui->btnRetry->setEnabled(true);
    }
    if(code == 200) {
        QJsonDocument doc = QJsonDocument::fromJson(body);
        QJsonObject obj = doc.object();
        int err = obj.value("error").toInt();
        if(err == 0) {
            QString key = obj.value("data").toString();
            qDebug() << "onRetryToGetLicense:" << key;
            QKxVer *ver = QKxVer::instance();
            QString errMsg;
            if(ver->parse(key, errMsg)) {
                if(ver->isExpired()) {
                    QDate expire = ver->expireDate();
                    QString desc = tr("I'm very sorry, you had already enjoyed the chance about %1 before ago. If the information is incorrect, you can contact the official support to solve it.").arg(expire.toString("yyyy-MM-dd"));
                    ui->status->setText(desc);
                }else{
                    QString desc = tr("You have obtained a trial license, please enjoy using it.");
                    ui->status->setText(desc);

                    if(QKxMessageBox::warning(this, tr("Activation"), tr("Success to activate and restart the application to take effect right now."), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
                        QString path = QCoreApplication::instance()->applicationFilePath();
                        if(QKxProcessLaunch::startDetached(path)) {
                            QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
                        }
                    }
                }
                ui->btnRetry->hide();
            }else{
                QStringList out;
                out << tr("Error: %1").arg(errMsg);
                out << tr("Please try again. If it still cannot be resolved, please feedback and ask for official support.");
                ui->status->setText(out.join(' '));
                ui->btnRetry->show();
                ui->btnRetry->setText(tr("Retry again"));
            }
        }else{
            QStringList out;
            out << tr("Error: unexpected error.");
            out << tr("Please try again. If it still cannot be resolved, please feedback and ask for official support.");
            ui->status->setText(out.join(' '));
            ui->btnRetry->show();
            ui->btnRetry->setText(tr("Retry again"));
        }

    }else{
        ui->status->setText(tr("Please try again. If it still cannot be resolved, please feedback and ask for official support."));
        ui->btnRetry->show();
        ui->btnRetry->setText(tr("Retry again"));
    }
}

void QWoLicenseTrialApplyDialog::tryToGetLicense(int tryLeft)
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
        QMetaObject::invokeMethod(this, "tryToGetLicense", Qt::QueuedConnection, Q_ARG(int, tryLeft));
        return;
    }

    QString info = ver->machineInformation();
    if(mid.isEmpty()) {
        QKxMessageBox::information(this, tr("Machine information"), tr("Unable to generate a unique computer ID, please contact customer service to resolve this issue."));
        return;
    }

    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onRetryToGetLicense(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    QJsonObject obj;
    obj.insert("mid", mid);
    obj.insert("info", info);
    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray json = doc.toJson(QJsonDocument::Compact);
    http->post("http://key.woterm.com/trial", json, "application/json; charset=utf-8");
    ui->btnRetry->setEnabled(false);
    ui->status->setText(tr("start to check...."));
}
