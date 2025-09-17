/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwoaboutdialog.h"
#include "ui_qwoaboutdialog.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QUrl>

#include "version.h"
#include "qkxhttpclient.h"
#include "qkxmessagebox.h"
#include "qwoutils.h"
#include "qkxver.h"

QWoAboutDialog::QWoAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoAboutDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);


    ui->verNow->setText(WOTERM_VERSION);
    QKxVer *ver = QKxVer::instance();
    QKxVer::ELicenseType type = ver->licenseType();
    QString typeDesc = tr("Unknow version");
    if(type == QKxVer::EFreeVersion) {
        typeDesc = tr("Free version");
    }else if(type == QKxVer::ETrialVersion) {
        typeDesc = tr("Trial version");
    } else if(type == QKxVer::ESchoolVersion) {
        typeDesc = tr("School version");
    } else if(type == QKxVer::EUltimateVersion) {
        typeDesc = tr("Ultimate version");
    }
    if(ver->isExpired()) {
        typeDesc += QString("[%1]").arg(tr("expired"));
    }
    ui->authorizeLevel->setText(typeDesc);
    ui->website->setText("<a href=\"http://www.woterm.com\">http://www.woterm.com</a>");
    ui->website->setOpenExternalLinks(true);
    ui->website->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->website->setWordWrap(true);
    ui->website->setTextFormat(Qt::RichText);

    QObject::connect(ui->btnVersion, SIGNAL(clicked()), this, SLOT(onVersionCheckButtonClicked()));

    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,QByteArray)), this, SLOT(onVersionCheck(int,QByteArray)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get("http://down.woterm.com/.ver");
    adjustSize();

    ui->buildTime->setText(WOTERM_BUILD_TIME);
#ifdef QT_DEBUG
    ui->btnOfficeWeb->setEnabled(false);
#else
    ui->btnOfficeWeb->deleteLater();
#endif

}

QWoAboutDialog::~QWoAboutDialog()
{
    delete ui;
}

void QWoAboutDialog::onVersionCheck(int code, const QByteArray &body)
{
    QString ver = body.trimmed();
    ui->verLatest->setText(ver);
}

void QWoAboutDialog::onVersionCheckButtonClicked()
{
    QDesktopServices::openUrl(QUrl("http://www.woterm.com"));
}
