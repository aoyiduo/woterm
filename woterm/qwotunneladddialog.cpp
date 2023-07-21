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

#include "qwotunneladddialog.h"
#include "ui_qwotunneladddialog.h"

#include "qwohostsimplelist.h"
#include "qkxbuttonassist.h"
#include "qkxmessagebox.h"
#include "qkxver.h"
#include "qwoutils.h"
#include "qwosetting.h"

#include <QStringListModel>
#include <QDateTime>
#include <QIntValidator>


#define PORTMAP_INDEX       (0)

QWoTunnelAddDialog::QWoTunnelAddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoTunnelAddDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Add tunnel"));
    ui->sessionName->setReadOnly(true);
    m_id = 0;

    QObject::connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->btnModify, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->btnSessions, SIGNAL(clicked()), this, SLOT(onSessionsButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

    ui->btnModify->setVisible(false);

    ui->hostLocal->setClearButtonEnabled(false);
    ui->hostLocal->setText("127.0.0.1");
    ui->hostLocal->setReadOnly(true);
    ui->hostRemote->setClearButtonEnabled(false);
    ui->hostRemote->setText("127.0.0.1");
    ui->hostRemote->setReadOnly(true);

    {
        QKxButtonAssist *btnLocal = new QKxButtonAssist("../private/skins/black/edit.png", ui->hostLocal);
        QObject::connect(btnLocal, SIGNAL(pressed(QToolButton*)), this, SLOT(onHostLocalAssistClicked()));
        btnLocal->button(0)->setToolTip(tr("Edit the host address"));
    }

    {
        QKxButtonAssist *btnRemote = new QKxButtonAssist("../private/skins/black/edit.png", ui->hostRemote);
        QObject::connect(btnRemote, SIGNAL(pressed(QToolButton*)), this, SLOT(onHostRemoteAssistClicked()));
        btnRemote->button(0)->setToolTip(tr("Edit the host address"));
    }

    ui->portLocal->setText("1080");
    ui->portRemote->setText("3306");
    ui->portLocal->setValidator(new QIntValidator(1, 65535, ui->portLocal));
    ui->portRemote->setValidator(new QIntValidator(1, 65535, ui->portRemote));


    QStringList types;
    types.append(tr("Port mapping"));
    types.append(tr("Socks4/5"));
    ui->type->setModel(new QStringListModel(types));
    QObject::connect(ui->type, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeCurrentIndexChanged(int)));
    ui->type->setCurrentIndex(1);
    adjustSize();
}

QWoTunnelAddDialog::~QWoTunnelAddDialog()
{
    delete ui;
}

TunnelInfo QWoTunnelAddDialog::result() const
{
    TunnelInfo ti;
    ti.id = m_id;
    ti.isPortmapping = ui->type->currentIndex() == PORTMAP_INDEX;
    ti.hostLocal = ui->hostLocal->text();
    ti.portLocal = ui->portLocal->text().toInt();
    if(ti.isPortmapping) {
        ti.hostRemote = ui->hostRemote->text();
        ti.portRemote = ui->portRemote->text().toInt();
    }
    ti.sessionName = ui->sessionName->text();
    return ti;
}

void QWoTunnelAddDialog::onApplyButtonClicked()
{
    QString name = ui->sessionName->text();
    if(name.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The session name parameter can not be empty."));
        return;
    }
    if(ui->type->currentIndex() == PORTMAP_INDEX) {
        QString hostRemote = ui->hostRemote->text();
        if(hostRemote.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("The remote host parameter can not be empty."));
            return;
        }
        QString portRemote = ui->portRemote->text();
        if(portRemote.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("The remote port parameter can not be empty."));
            return;
        }
        bool ok;
        int iport = portRemote.toInt(&ok);
        if(iport <= 0 || !ok) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("The remote port parameter should be range between 1 and 65535."));
            return;
        }
    }
    QString hostLocal = ui->hostLocal->text();
    if(hostLocal.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The local host parameter can not be empty."));
        return;
    }
    QString portLocal = ui->portLocal->text();
    if(portLocal.isEmpty()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The local port parameter can not be empty."));
        return;
    }
    bool ok;
    int iport = portLocal.toInt(&ok);
    if(iport <= 0 || !ok) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The local port parameter should be range between 1 and 65535."));
        return;
    }
    if(!hostLocal.startsWith("127.0.0.")) {
        if(!QWoSetting::allowOtherHostConnectToTunnel()) {
            QKxMessageBox::information(this, tr("Permission restrictions"), tr("Please enable allow connections from the local network in the administrator dialog."));
            return;
        }
    }
    done(QDialog::Accepted+1);
}

void QWoTunnelAddDialog::onSessionsButtonClicked()
{
    QWoHostSimpleList dlg(SshBase, this);
    dlg.exec();
    HostInfo hi;
    if(!dlg.result(&hi)) {
        return;
    }
    ui->sessionName->setText(hi.name);
}

void QWoTunnelAddDialog::onTypeCurrentIndexChanged(int idx)
{
    ui->remoteGroup->setVisible(idx == 0);
    adjustSize();
}

void QWoTunnelAddDialog::onHostLocalAssistClicked()
{
    ui->hostLocal->setReadOnly(!ui->hostLocal->isReadOnly());
    QStyle *style = ui->hostLocal->style();
    style->polish(ui->hostLocal);
}

void QWoTunnelAddDialog::onHostRemoteAssistClicked()
{
    ui->hostRemote->setReadOnly(!ui->hostRemote->isReadOnly());
    QStyle *style = ui->hostRemote->style();
    style->polish(ui->hostRemote);
}
