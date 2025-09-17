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

#include "qwoversionupgradetipdialog.h"
#include "ui_qwoversionupgradetipdialog.h"
#include "qkxhttpclient.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "version.h"
#include "qkxmessagebox.h"

#include <QLocale>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDesktopServices>

QWoVersionUpgradeTipDialog::QWoVersionUpgradeTipDialog(bool isAuto, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoVersionUpgradeTipDialog)
    , m_parent(parent)
    , m_isAuto(isAuto)

{
    ui->setupUi(this);

    setWindowTitle(tr("Version upgrade"));

    QObject::connect(ui->btnIgnore, SIGNAL(clicked()), this, SLOT(onIgnoreButtonClicked()));
    QObject::connect(ui->btn7Days, SIGNAL(clicked()), this, SLOT(on7DaysButtonClicked()));
    QObject::connect(ui->btnTomorrow, SIGNAL(clicked()), this, SLOT(onTomorrowButtonClicked()));
    QObject::connect(ui->btnUpgrade, SIGNAL(clicked()), this, SLOT(onUpgradeButtonClicked()));

    checkVersion();
}

QWoVersionUpgradeTipDialog::~QWoVersionUpgradeTipDialog()
{
    delete ui;
}


void QWoVersionUpgradeTipDialog::check(QWidget *parent, bool isAuto)
{
    QWoVersionUpgradeTipDialog *dlg = new QWoVersionUpgradeTipDialog(isAuto, parent);
    Q_UNUSED(dlg)
}

void QWoVersionUpgradeTipDialog::checkVersion()
{
    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionCheck(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get("http://down.woterm.com/.ver");
}

void QWoVersionUpgradeTipDialog::getVersionDescription()
{
    QString lang = QWoSetting::languageTypeAsBCP47Field();
    QString verDesc = QString("http://down.woterm.com/.desc/latest-%1").arg(lang);
    qDebug() << "QWoVersionUpgradeTipDialog" << verDesc;
    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionDescriptionGet(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get(verDesc);
}

void QWoVersionUpgradeTipDialog::onVersionCheck(int code, const QByteArray &body)
{
    handleVersionCheck(code, body);
    deleteLater();
}

void QWoVersionUpgradeTipDialog::onVersionDescriptionGet(int code, const QByteArray& body)
{
    if(code == 200) {
        ui->desc->setPlainText(QString::fromUtf8(body));
    }else{
        ui->desc->setPlainText(tr("Failed to get upgrade content."));
    }
}

void QWoVersionUpgradeTipDialog::onIgnoreButtonClicked()
{
    QWoSetting::setSkipThisVersion(m_verBody);
    done(QDialog::Accepted);
}

void QWoVersionUpgradeTipDialog::on7DaysButtonClicked()
{
    QDate dt = QDate::currentDate();
    dt = dt.addDays(7);
    QWoSetting::setNextUpgradeVersionDate(m_verBody, dt);
    done(QDialog::Accepted);
}

void QWoVersionUpgradeTipDialog::onTomorrowButtonClicked()
{
    QWoSetting::setIgnoreTodayUpgradeVersion(m_verBody);
    done(QDialog::Accepted);
}

void QWoVersionUpgradeTipDialog::onUpgradeButtonClicked()
{
    QDesktopServices::openUrl(QUrl("http://woterm.com"));
    done(QDialog::Accepted);
}

bool QWoVersionUpgradeTipDialog::handleVersionCheck(int code, const QByteArray &body)
{
    if(body.isEmpty()) {
        return false;
    }
    if(code == 200) {
        qDebug() << code << body;
        QString verBody = body.trimmed();
        if(body[0] == 'v') {
            verBody = verBody.mid(1);
        }
        m_verBody = verBody;
        int verLatest = QWoUtils::parseVersion(verBody);
        int verCurrent = QWoUtils::parseVersion(WOTERM_VERSION);
        if(verCurrent < verLatest) {
            if(m_isAuto) {
                bool pop = QWoSetting::shouldPopupUpgradeVersionMessage(verBody);
                if(!pop) {
                    return false;
                }

            }
            getVersionDescription();
            exec();
        }else if(!m_isAuto){
            QKxMessageBox::information(m_parent, tr("Upgrade information"), tr("The current version is already the latest version."));
        }
    }else if(!m_isAuto) {
        QKxMessageBox::information(m_parent, tr("Upgrade information"), tr("Unexpected error, please try again later."));
    }
    return true;
}
