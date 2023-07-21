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

#include "qwotunneldialog.h"
#include "ui_qwotunneldialog.h"

#include "qwotunneladddialog.h"
#include "qwotunnelmodifydialog.h"
#include "qwotunnelmodel.h"
#include "qkxmessagebox.h"
#include "qwoutils.h"
#include "qwotunnelserver.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoapplication.h"

QWoTunnelDialog::QWoTunnelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoTunnelDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Tunnel manage"));
    setWindowIcon(QIcon(":/woterm/resource/images/tunnel.png"));

    ui->info->setVisible(QWoApplication::instance()->type() == QWoApplication::eMain);

    QObject::connect(ui->btnClose, SIGNAL(clicked()),  this, SLOT(close()));
    QObject::connect(ui->btnAdd, SIGNAL(clicked()),  this, SLOT(onAddButtonClicked()));
    QObject::connect(ui->btnModify, SIGNAL(clicked()),  this, SLOT(onModifyButtonClicked()));
    QObject::connect(ui->btnRemove, SIGNAL(clicked()),  this, SLOT(onRemoveButtonClicked()));
    QObject::connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(onStartButtonClicked()));
    QObject::connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(onStopButtonClicked()));
    QObject::connect(ui->btnSwitch, SIGNAL(clicked()), this, SLOT(onSwitchButtonClicked()));

    onSwitchButtonClicked();

    m_model = new QWoTunnelModel(ui->tunnelList);
    QObject::connect(m_model, SIGNAL(modelReset()), this, SLOT(onTunnelListModelReset()), Qt::QueuedConnection);

    ui->tunnelList->setIndentation(0);
    ui->tunnelList->setModel(m_model);
    ui->tunnelList->resizeColumnToContents(0);
    ui->tunnelList->setSelectionBehavior(QAbstractItemView::SelectRows);
    QTextDocument *doc = ui->logview->document();
    doc->setMaximumBlockCount(500);

    QObject::connect(ui->tunnelList, SIGNAL(clicked(QModelIndex)), this, SLOT(onTunnelListItemClicked(QModelIndex)), Qt::QueuedConnection);
    QObject::connect(ui->tunnelList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTunnelListItemClicked(QModelIndex)), Qt::QueuedConnection);
    QObject::connect(ui->tunnelList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onModifyButtonClicked()), Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, "resetState", Qt::QueuedConnection);

    QWoSshConf::instance()->refresh();
}

QWoTunnelDialog::~QWoTunnelDialog()
{
    delete ui;
}

void QWoTunnelDialog::onAddButtonClicked()
{
    QWoTunnelAddDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_model->add(dlg.result());
    }
}

void QWoTunnelDialog::onModifyButtonClicked()
{
    const QModelIndex& idx = ui->tunnelList->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    TunnelInfo ti = idx.data(ROLE_INDEX).value<TunnelInfo>();
    QWoTunnelModifyDialog dlg(ti, this);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_model->modify(dlg.result());
    }
}

void QWoTunnelDialog::onRemoveButtonClicked()
{
    const QModelIndex& idx = ui->tunnelList->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    TunnelInfo ti = idx.data(ROLE_INDEX).value<TunnelInfo>();
    QWoTunnelServer *server = QWoTunnelFactory::instance()->find(ti.id);
    if(server != nullptr) {
        server->stop();
        server->deleteLater();
    }
    if(QKxMessageBox::information(this, tr("Remove warnning"), tr("About to remove the tunnel information:")+ti.sessionName, QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_model->remove(ti.id);
    }
}

void QWoTunnelDialog::onTunnelListModelReset()
{
    ui->tunnelList->resizeColumnToContents(0);
    QMetaObject::invokeMethod(this, "resetState", Qt::QueuedConnection);
}

void QWoTunnelDialog::onTunnelListItemClicked(const QModelIndex &idx)
{
    ui->btnModify->setEnabled(idx.isValid());    
    resetState();
}

void QWoTunnelDialog::onStartButtonClicked()
{
    const QModelIndex& idx = ui->tunnelList->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    TunnelInfo ti = idx.data(ROLE_INDEX).value<TunnelInfo>();
    if(!ti.hostLocal.startsWith("127.0.0.")) {
        if(!QWoSetting::allowOtherHostConnectToTunnel()) {
            QKxMessageBox::information(this, tr("Permission restrictions"), tr("Please enable allow connections from the local area network in the administrator dialog."));
            return;
        }
    }
    if(ti.isPortmapping) {
        bool bcreated = false;
        QWoPortmappingServer *server = QWoTunnelFactory::instance()->get<QWoPortmappingServer>(ti.id, &bcreated);
        if(bcreated) {
            QObject::connect(server, SIGNAL(errorArrived(QString)), this, SLOT(onTunnelErrorArrived(QString)), Qt::UniqueConnection);
        }
        if(server->isRunning()) {
            return;
        }
        if(server->start(ti.sessionName, ti.hostLocal, ti.portLocal, ti.hostRemote, ti.portRemote)) {
            QString err = tr("Tunnel started successfully.");
            appendLog(ti.sessionName, err);
        }else{
            QString err = tr("Tunnel startup failed.");
            appendLog(ti.sessionName, err);
            QKxMessageBox::warning(this, tr("Tunnel error"), err);
        }
    }else{
        bool bcreated = false;
        QWoSocks45Server *server = QWoTunnelFactory::instance()->get<QWoSocks45Server>(ti.id, &bcreated);
        if(bcreated) {
            QObject::connect(server, SIGNAL(errorArrived(QString)), this, SLOT(onTunnelErrorArrived(QString)), Qt::UniqueConnection);
        }
        if(server->isRunning()) {
            return;
        }
        if(server->start(ti.sessionName, ti.hostLocal, ti.portLocal)) {
            QString err = tr("Tunnel started successfully.");
            appendLog(ti.sessionName, err);
        }else{
            QString err = tr("Tunnel startup failed.");
            appendLog(ti.sessionName, err);
            QKxMessageBox::warning(this, tr("Tunnel error"), err);
        }
    }
    QMetaObject::invokeMethod(this, "resetState", Qt::QueuedConnection);
}

void QWoTunnelDialog::onStopButtonClicked()
{
    const QModelIndex& idx = ui->tunnelList->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    TunnelInfo ti = idx.data(ROLE_INDEX).value<TunnelInfo>();
    QWoTunnelServer *server = QWoTunnelFactory::instance()->find(ti.id);
    if(server != nullptr) {
        server->stop();
    }
    QMetaObject::invokeMethod(this, "resetState", Qt::QueuedConnection);
}

void QWoTunnelDialog::onSwitchButtonClicked()
{
    bool isVisible = ui->buttonArea->isVisible();
    if(isVisible) {
        ui->btnSwitch->setIcon(QIcon("../private/skins/black/back2.png"));
        ui->btnSwitch->setText(tr("Back"));
        ui->buttonArea->setVisible(false);
        ui->stackedWidget->setCurrentWidget(ui->logArea);
    }else{
        ui->btnSwitch->setIcon(QIcon("../private/skins/black/history2.png"));
        ui->btnSwitch->setText(tr("Logview"));
        ui->buttonArea->setVisible(true);
        ui->stackedWidget->setCurrentWidget(ui->tunnelArea);
    }
}

void QWoTunnelDialog::onTunnelErrorArrived(const QString &err)
{
    QWoTunnelServer *server = qobject_cast<QWoTunnelServer*>(sender());
    QString name = server->sessionName();
    appendLog(name, err);
}

void QWoTunnelDialog::appendLog(const QString &sessionName, const QString &_msg)
{
    QString msg = QString("[%1]%2").arg(sessionName, _msg);

    QTextCursor tc = ui->logview->textCursor();
    tc.movePosition(QTextCursor::End);
    tc.insertBlock();
    tc.insertText(msg);
}

void QWoTunnelDialog::resetState()
{
    QModelIndex idx = ui->tunnelList->currentIndex();
    if(!idx.isValid()) {
        ui->btnModify->setVisible(false);
        ui->btnRemove->setVisible(false);
        ui->btnStart->setVisible(false);
        ui->btnStop->setVisible(false);
        return;
    }
    TunnelInfo ti = idx.data(ROLE_INDEX).value<TunnelInfo>();
    ui->btnModify->setVisible(true);
    ui->btnRemove->setVisible(true);
    bool isRun = QWoTunnelFactory::instance()->isRunning(ti.id);
    ui->btnStart->setVisible(!isRun);
    ui->btnStop->setVisible(isRun);

    ui->tunnelList->viewport()->update();
}
