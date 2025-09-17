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

#include "qwohostinfoedit.h"
#include "ui_qwohostinfo.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qwohostsimplelist.h"
#include "qwosetting.h"
#include "qkxmessagebox.h"


#include <QPlainTextEdit>
#include <QFileDialog>
#include <QDebug>
#include <QStringListModel>

QWoHostInfoEdit::QWoHostInfoEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoHostInfo)
{
    init();
    setWindowTitle(tr("Add"));
}

QWoHostInfoEdit::QWoHostInfoEdit(const QString& name, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoHostInfo)
    , m_name(name)
{
    init();
    setWindowTitle(tr("Modify"));
    HostInfo hi = QWoSshConf::instance()->find(name);
    ui->hostName->setText(hi.name);
    ui->host->setText(hi.host);
    ui->port->setText(QString("%1").arg(hi.port));
    ui->memo->setPlainText(hi.memo);
    ui->userName->setEditText(hi.user);
    ui->password->setText(hi.password);
    ui->identify->setEditText(hi.identityFile);
    if(!hi.password.isEmpty()) {
        ui->authType->setCurrentText("Password");
    }else{
        ui->authType->setCurrentText("IdentifyFile");
    }
    ui->jump->setEditText(hi.proxyJump);
}

QWoHostInfoEdit::~QWoHostInfoEdit()
{
    delete ui;
}

void QWoHostInfoEdit::onAuthCurrentIndexChanged(const QString & txt)
{
    bool isPass = txt == tr("Password");

    QWoUtils::setLayoutVisible(ui->passLayout, isPass);
    QWoUtils::setLayoutVisible(ui->identifyLayout, !isPass);
}

void QWoHostInfoEdit::onButtonSaveClicked()
{
    HostInfo hi;
    hi.name = ui->hostName->text();
    hi.host = ui->host->text();
    hi.port = ui->port->text().toInt();
    hi.memo = ui->memo->toPlainText();
    hi.user = ui->userName->currentText();
    hi.password = ui->password->text();
    hi.identityFile = QDir::toNativeSeparators(ui->identify->currentText());
    hi.proxyJump = ui->jump->currentText();

    if(hi.name.isEmpty()) {
        QKxMessageBox::warning(this, tr("Info"), tr("The name can't be empty"));
        return;
    }
    if(hi.host.isEmpty()) {
        QKxMessageBox::warning(this, tr("Info"), tr("The host can't be empty"));
        return;
    }
    if(hi.port < 10 || hi.port > 65535) {
        QKxMessageBox::warning(this, tr("Info"), tr("The port should be at [10,65535]"));
        return;
    }
    close();
    if(m_name.isEmpty()) {
        QWoSshConf::instance()->append(hi);
        return;
    }
    QWoSshConf::instance()->modify(hi);
    if(!hi.identityFile.isEmpty())
    {
        QVariant v = QWoSetting::value("history/identifyList");
        QStringList el = v.toStringList();
        el.removeAll(hi.identityFile);
        el.insert(0, hi.identityFile);
        if(el.length() > 5) {
            el.removeLast();
        }
        QWoSetting::setValue("history/identifyList", el);
    }
    if(!hi.user.isEmpty())
    {
        QVariant v = QWoSetting::value("history/userNameList");
        QStringList el = v.toStringList();
        el.removeAll(hi.user);
        el.insert(0, hi.user);
        if(el.length() > 5) {
            el.removeLast();
        }
        QWoSetting::setValue("history/userNameList", el);
    }
    if(!hi.proxyJump.isEmpty())
    {
        QVariant v = QWoSetting::value("history/proxyJumpList");
        QStringList el = v.toStringList();
        el.removeAll(hi.proxyJump);
        el.insert(0, hi.proxyJump);
        if(el.length() > 5) {
            el.removeLast();
        }
        QWoSetting::setValue("history/proxyJumpList", el);
    }
}

void QWoHostInfoEdit::onButtonJumpBrowserClicked()
{
    QWoHostSimpleList dlg(SshBase, this);
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        ui->jump->setEditText(hi.name);
    }
}

void QWoHostInfoEdit::onButtonIdentifyBrowserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    qDebug() << "fileName" << fileName;
    fileName = QDir::toNativeSeparators(fileName);
    ui->identify->setCurrentText(fileName);
}

void QWoHostInfoEdit::init()
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Add"));
    onAuthCurrentIndexChanged(ui->authType->currentText());
    QIntValidator *validator = new QIntValidator(10, 65535, ui->port);
    ui->port->setValidator(validator);
    ui->port->setText("22");
    QObject::connect(ui->authType, SIGNAL(currentIndexChanged(const QString &)),  this, SLOT(onAuthCurrentIndexChanged(const QString &)));
    QObject::connect(ui->save, SIGNAL(clicked()),  this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->identifyBrowser, SIGNAL(clicked()),  this, SLOT(onButtonIdentifyBrowserClicked()));
    QObject::connect(ui->jumpBrowser, SIGNAL(clicked()),  this, SLOT(onButtonJumpBrowserClicked()), Qt::QueuedConnection);
    {
        QVariant v = QWoSetting::value("history/identifyList");
        QStringList el = v.toStringList();
        QStringListModel *model = new QStringListModel(this);
        model->setStringList(el);
        ui->identify->setModel(model);
    }
    {
        QVariant v = QWoSetting::value("history/userNameList");
        QStringList el = v.toStringList();
        QStringListModel *model = new QStringListModel(this);
        model->setStringList(el);
        ui->userName->setModel(model);
    }
    {
        QVariant v = QWoSetting::value("history/proxyJumpList");
        QStringList el = v.toStringList();
        QStringListModel *model = new QStringListModel(this);
        model->setStringList(el);
        ui->jump->setModel(model);
    }
}
