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

#include "qwoaboutdialog.h"
#include "ui_qwoaboutdialog.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

#include "version.h"
#include "qkxhttpclient.h"
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
    ui->authorizeLevel->setText(QKxVer::isUltimate() ? "Ultimate beta" : "Free");

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
