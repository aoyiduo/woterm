/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwolicensedialog.h"
#include "ui_qwolicensedialog.h"
#include "qkxver.h"
#include "qkxmessagebox.h"
#include "qkxprocesslaunch.h"
#include "qwolicenseactivatedialog.h"
#include "qwolicensetrialapplydialog.h"
#include "qkxbuttonassist.h"
#include "qwosetting.h"

#include <QDesktopServices>
#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>
#include <QDebug>
#include <QUrl>

QWoLicenseDialog::QWoLicenseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoLicenseDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("License information"));
    ui->key->setReadOnly(true);
    ui->mid->setReadOnly(true);
    QKxButtonAssist *btn = new QKxButtonAssist(":/woterm/resource/skin/ftp.png", false, ui->key);
    QObject::connect(btn, SIGNAL(clicked(int)), this, SLOT(onSNAssistButtonClicked(int)));
    QKxButtonAssist *btn2 = new QKxButtonAssist(":/woterm/resource/skin/ftp.png", false, ui->mid);
    QObject::connect(btn2, SIGNAL(clicked(int)), this, SLOT(onMIDAssistButtonClicked(int)));

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnActivate, SIGNAL(clicked()), this, SLOT(onActivateButtonClicked()));
    QObject::connect(ui->btnRemove, SIGNAL(clicked()), this, SLOT(onLicenseRemoveButtonClicked()));
    QObject::connect(ui->btnVersions, SIGNAL(clicked()), this, SLOT(onVersionsDifferenceButtonClicked()));
    QObject::connect(ui->wantTrial, SIGNAL(linkActivated(QString)), this, SLOT(onTrialLinkActivated(QString)));
    ui->wantTrial->setOpenExternalLinks(false);
    reset();
}

QWoLicenseDialog::~QWoLicenseDialog()
{
    delete ui;
}

void QWoLicenseDialog::onActivateButtonClicked()
{
    QWoLicenseActivateDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        reset();
        QKxVer *ver = QKxVer::instance();
        int days = ver->expireDaysLeft();
        if(days < 0) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("license is expired"));
            return;
        }
        if(QKxMessageBox::warning(this, tr("Activation"), tr("Success to activate and restart the application to take effect right now."), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
            QString path = QCoreApplication::instance()->applicationFilePath();
            if(QKxProcessLaunch::startDetached(path)) {
                QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
            }
        }
    }
}

void QWoLicenseDialog::onLicenseRemoveButtonClicked()
{
    if(QKxMessageBox::warning(this,
                              tr("License remove"),
                              tr("After removing the relevant license, it will be downgraded to the free version. Do you still want to continue."),
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        QKxVer *ver = QKxVer::instance();
        if(ver->remove()) {
            QString path = QCoreApplication::instance()->applicationFilePath();
            if(QKxProcessLaunch::startDetached(path)) {
                QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
            }
        }
    }
}

void QWoLicenseDialog::onVersionsDifferenceButtonClicked()
{
    QDesktopServices::openUrl(QWoSetting::isChineseLanguageFile() ? QUrl("http://cn.woterm.com/prices") : QUrl("http://en.woterm.com/prices"));
}

void QWoLicenseDialog::onTrialLinkActivated(const QString &link)
{
    if(link == "evaluateUltimate") {
        QWoLicenseTrialApplyDialog dlg(this);
        dlg.exec();
    }
}

void QWoLicenseDialog::onSNAssistButtonClicked(int idx)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString mid = ui->key->text();
    if(mid.isEmpty()) {
        return;
    }
    clipboard->setText(mid);
}

void QWoLicenseDialog::onMIDAssistButtonClicked(int idx)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString mid = ui->mid->text();
    if(mid.isEmpty()) {
        return;
    }
    clipboard->setText(mid);
}

void QWoLicenseDialog::reset()
{
    QKxVer *ver = QKxVer::instance();
    QString mid = ver->machineID();
    ui->mid->setText(mid);
    QKxVer::ELicenseType type = ver->licenseType();
    if(type == QKxVer::EFreeVersion) {
        ui->licenseType->setText(tr("Free version"));
        ui->dateStart->setText("1972-01-01");
        ui->dateExpire->setText("9999-12-30");
        ui->daysLeft->setText(tr("Forever"));
        ui->key->setText("00000-00000-00000-00000");
        ui->btnRemove->setVisible(false);
        ui->wantTrial->setVisible(true);
    } else{        
        if(type == QKxVer::ETrialVersion) {
            ui->licenseType->setText(tr("Trial version"));
        } else if(type == QKxVer::ESchoolVersion) {
            ui->licenseType->setText(tr("School version"));
        } else if(type == QKxVer::EUltimateVersion) {
            ui->licenseType->setText(tr("Ultimate version"));
        } else {
            ui->licenseType->setText(tr("Unknow version"));
        }
        QString since = ver->sinceDate().toString("yyyy-MM-dd");
        QString expire = ver->expireDate().toString("yyyy-MM-dd");
        int days = ver->expireDaysLeft();
        QString key = ver->licenseKey();
        ui->dateStart->setText(since);
        ui->dateExpire->setText(expire);
        ui->daysLeft->setText(days >= 0 ? QString(" %1 ").arg(days)+tr("days"): tr("expired"));
        ui->key->setText(key);
        ui->btnRemove->setVisible(true);
        ui->wantTrial->setVisible(false);
    }

    adjustSize();
}
