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

    QString txt = QString("Current Version: %1").arg(WOTERM_VERSION);
    txt.append("<br>Check New Version:");
    txt.append("<br>(CN): <a href=\"http://www.aoyiduo.cn/woterm\">http://www.aoyiduo.cn/woterm</a>");
    txt.append("<br>(EN): <a href=\"http://www.aoyiduo.com/woterm\">http://www.aoyiduo.com/woterm</a>");

    ui->desc->setText(txt);
    ui->desc->setOpenExternalLinks(true);
    ui->desc->setWordWrap(true);
    ui->desc->setTextFormat(Qt::RichText);

    QObject::connect(ui->btnVersion, SIGNAL(clicked()), this, SLOT(onVersionCheck()));


}

QWoAboutDialog::~QWoAboutDialog()
{
    delete ui;
}

void QWoAboutDialog::onVersionCheck()
{
//    m_httpClient = new QHttpClient(this);
//    QObject::connect(m_httpClient, SIGNAL(result(int,const QByteArray&)), this, SLOT(onResult(int,const QByteArray&)));
//    m_httpClient->get("http://www.woterm.com/version/latest");
    QDesktopServices::openUrl(QUrl("http://www.aoyiduo.com/woterm"));
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
