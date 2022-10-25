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

QWoAboutDialog::QWoAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoAboutDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    ui->verNow->setText(WOTERM_VERSION);

    ui->website->setText("<a href=\"http://www.woterm.com\">http://www.woterm.com</a>");
    ui->website->setOpenExternalLinks(true);
    ui->website->setWordWrap(true);
    ui->website->setTextFormat(Qt::RichText);

    QObject::connect(ui->btnVersion, SIGNAL(clicked()), this, SLOT(onVersionCheckButtonClicked()));

    QKxHttpClient *http = new QKxHttpClient(this);
    QObject::connect(http, SIGNAL(result(int,const QByteArray&)), this, SLOT(onVersionCheck(int,const QByteArray&)));
    QObject::connect(http, SIGNAL(finished()), http, SLOT(deleteLater()));
    http->get("http://down.woterm.com/.ver");
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

void QWoAboutDialog::onResult(int code, const QByteArray &body)
{
    qDebug() << code << body;
    if(code < 0) {
        QMessageBox::warning(this, tr("version check"), QString(tr("Failed For %1")).arg(body.data()));
    }else{
        if(code == 200) {
            QJsonDocument json = QJsonDocument::fromJson(body);
            if(json.isObject()) {
                QJsonObject obj = json.object();
                QJsonValue ver = obj.value("version");
                QString version = ver.toString();
                qDebug() << "version" << version;
                int ver_latest = QWoUtils::versionToLong(version);
                int ver_now = QWoUtils::versionToLong(WOTERM_VERSION);
                if(ver_latest > ver_now) {
                    int ok = QMessageBox::information(this, tr("version check"), QString(tr("Found New Version: %1, Try To Download?")).arg(version), QMessageBox::Ok|QMessageBox::Cancel);
                    if(ok == QMessageBox::Ok) {
                        QDesktopServices::openUrl(QUrl("http://www.woterm.com"));
                    }
                }else{
                    QMessageBox::information(this, tr("version check"), QString(tr("No New Version")));
                }
            }
        }
    }
    m_httpClient->deleteLater();
}
