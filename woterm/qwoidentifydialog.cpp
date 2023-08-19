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

#include "qwoidentifydialog.h"
#include "ui_qwoidentifydialog.h"
#include "qwoutils.h"
#include "qwosetting.h"
#include "qworenamedialog.h"
#include "qwoidentifykeycontentdialog.h"
#include "qwoidentifycreatedialog.h"
#include "qwoidentify.h"
#include "qkxmessagebox.h"
#include "qkxver.h"
#include "qkxbuttonassist.h"

#include <QFileDialog>
#include <QDebug>
#include <QDirModel>
#include <QProcess>
#include <QTimer>
#include <QEventLoop>
#include <QCryptographicHash>
#include <QTreeWidget>
#include <QProcessEnvironment>

#define ROLE_IDENTIFY_PUBKEY (Qt::UserRole+1)
#define ROLE_IDENTIFY_PRVKEY (Qt::UserRole+2)
#define ROLE_IDENTIFY_TYPE (Qt::UserRole+3)
#define ROLE_IDENTIFY_FIGURE (Qt::UserRole+4)

QWoIdentifyDialog::QWoIdentifyDialog(bool noselect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);    
    ui->setupUi(this);
    setWindowTitle(tr("Identify Manage"));

    ui->btnSelect->setVisible(!noselect);
    QObject::connect(ui->btnCreate, SIGNAL(clicked()), this, SLOT(onButtonCreateClicked()));
    QObject::connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(onButtonDeleteClicked()));
    QObject::connect(ui->btnExport, SIGNAL(clicked()), this, SLOT(onButtonExportClicked()));
    QObject::connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onButtonImportClicked()));
    QObject::connect(ui->btnRename, SIGNAL(clicked()), this, SLOT(onButtonRenameClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onButtonSelectClicked()));
    QObject::connect(ui->btnView, SIGNAL(clicked()), this, SLOT(onButtonPublicViewClicked()));
    QObject::connect(ui->btnPrivate, SIGNAL(clicked()), this, SLOT(onButtonPrivateViewClicked()));
    QObject::connect(ui->identify, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    QStringList items;
    items.append(tr("name"));
    items.append(tr("type"));
    items.append(tr("fingerprint"));
    ui->identify->setHeaderLabels(items);

    {
        QKxButtonAssist *assist = new QKxButtonAssist("../private/skins/black/edit.png", ui->agentFile);
        ui->agentFile->setReadOnly(true);
        QObject::connect(assist, &QKxButtonAssist::clicked, this, [=]{
            ui->agentFile->setReadOnly(!ui->agentFile->isReadOnly());
        });
        QString tmp = QWoSetting::lastLocalSshAgentAddress();
        ui->agentFile->setText(tmp);
        QObject::connect(ui->agentFile, &QLineEdit::textChanged, this, [=](){
            QWoSetting::setLastLocalSshAgentAddress(ui->agentFile->text());
        });
    }

    QByteArray sshAuthSock = QWoSetting::sshAuthSockDefault();
    ui->agentFile->setPlaceholderText(tr("Default:")+sshAuthSock);

    if(QKxVer::instance()->isFreeVersion()) {
        ui->btnPrivate->setVisible(false);
        ui->btnExport->setVisible(false);
    }

    {
        QString path = QDir::cleanPath(QDir::homePath() + "/.ssh");
        ui->sshFilePath->setText(path);
        bool on = QWoSetting::enableUserHomeIdentityFiles();
        ui->chkLocalUser->setChecked(on);
        QObject::connect(ui->chkLocalUser, &QCheckBox::clicked, this, [=](){
            QWoSetting::setEnableUserHomeIdentityFiles(ui->chkLocalUser->isChecked());
        });
    }
    {
        bool on = QWoSetting::enableLocalSshAgent();
        ui->chkLocalAgent->setChecked(on);
        QObject::connect(ui->chkLocalAgent, &QCheckBox::clicked, this, [=](){
            QWoSetting::setEnableLocalSshAgent(ui->chkLocalAgent->isChecked());
        });
    }
    {
        bool on = QWoSetting::enableRemoteSshAgent();
        ui->chkRemoteAgent->setChecked(on);
        ui->remoteAgentArea->setEnabled(on);
        QObject::connect(ui->chkRemoteAgent, &QCheckBox::clicked, this, [=](){
            QWoSetting::setEnableRemoteSshAgent(ui->chkRemoteAgent->isChecked());
            ui->remoteAgentArea->setEnabled(ui->chkRemoteAgent->isChecked());
        });

        QString addr = QWoSetting::remoteSshAgentAddress();
        QStringList hp = addr.split(':');
        ui->host->setText(hp.at(0));
        ui->port->setText(hp.at(1));
        ui->port->setValidator(new QIntValidator(80, 65535));
        QObject::connect(ui->host, &QLineEdit::textChanged, this, [=](){
            QString host = ui->host->text();
            QString port = ui->port->text();
            QWoSetting::setRemoteSshAgentAddress(host+":"+port);
        });
        QObject::connect(ui->port, &QLineEdit::textChanged, this, [=](){
            QString host = ui->host->text();
            QString port = ui->port->text();
            QWoSetting::setRemoteSshAgentAddress(host+":"+port);
        });
    }

    ui->tabWidget->setCurrentIndex(0);
    bool canUse = QWoSetting::allowToUseExternalIdentityFiles();
    ui->remote->setEnabled(canUse);
    ui->dirs->setEnabled(canUse);
    QWoUtils::setLayoutVisible(ui->externalTip1, !canUse);
    QWoUtils::setLayoutVisible(ui->externalTip2, !canUse);
    reload();
}

QWoIdentifyDialog::~QWoIdentifyDialog()
{
    delete ui;
}

QString QWoIdentifyDialog::open(bool noselect, QWidget *parent)
{
    QWoIdentifyDialog dlg(noselect, parent);
    dlg.exec();
    return dlg.result();
}


QString QWoIdentifyDialog::result() const
{
    return m_result;
}

void QWoIdentifyDialog::onButtonCreateClicked()
{
    QWoIdentifyCreateDialog dlg(this);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted) {
        reload();
    }
}

void QWoIdentifyDialog::onButtonImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    qDebug() << "fileName" << fileName;
    if(fileName.isEmpty()) {
        return;
    }
    fileName = QDir::toNativeSeparators(fileName);
    if(!QWoIdentify::isPrivateKey(fileName)) {
        QKxMessageBox::information(this, tr("info"), tr("Invalid private key file. Please select a valid private key file."));
        return;
    }
    IdentifyInfo info;
    if(!QWoIdentify::import(fileName, &info)) {
        QKxMessageBox::information(this, tr("info"), tr("the identify's file is bad"));
        return;
    }

    QAbstractItemModel *model = ui->identify->model();
    for(int i = 0; i < model->rowCount(); i++) {
        QModelIndex idx = model->index(i, 0);
        QString name = idx.data().toString();
        QString figure = idx.data(ROLE_IDENTIFY_FIGURE).toString();
        if(figure == info.fingureprint) {
            ui->identify->setCurrentIndex(idx);
            QKxMessageBox::information(this, tr("info"), tr("the same identify had been exist by name: %1").arg(name));
            return;
        }
    }
    QFileInfo fi(fileName);
    info.name = fi.fileName();
    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray buf = f.readAll();
    QByteArray data = QWoUtils::fromWotermStream(buf);
    f.close();
    QString name = info.name;
    QString errMsg;
    for(int i = 0; i < 10; i++) {
        name = QWoRenameDialog::open(name, errMsg, this);
        if(name.isEmpty()) {
            return;
        }
        if(QWoIdentify::create(name, info.prvKey)) {
            break;
        }
        errMsg = tr("The name already exists. Please enter a new name.");
    }
    reload();
}

void QWoIdentifyDialog::onButtonExportClicked()
{
    if(!isAdministrator()) {
        return;
    }

    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    QByteArray prvKey = idx2.data(ROLE_IDENTIFY_PRVKEY).toByteArray();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    if(fileName.isEmpty()) {
        return;
    }
    QFile prv(fileName);
    if(prv.open(QFile::WriteOnly)) {
        prv.write(prvKey);
        prv.close();

        QString type = idx2.data(ROLE_IDENTIFY_TYPE).toString();
        QString key = idx2.data(ROLE_IDENTIFY_PUBKEY).toString();
        QString content = type + " " + key + " " + name;
        QFile pub(fileName + ".pub");
        if(pub.open(QFile::WriteOnly)){
            pub.write(content.toUtf8());
            pub.close();
        }
    }
}

void QWoIdentifyDialog::onButtonDeleteClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    if(!QWoIdentify::remove(name)) {
        QKxMessageBox::warning(this, tr("Warning"), tr("failed to delete %1").arg(name));
        return;
    }
    model->removeRow(idx.row());
}

void QWoIdentifyDialog::onButtonSelectClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    m_result = idx2.data().toString();
    close();
}

void QWoIdentifyDialog::onButtonRenameClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    QString nameNew = QWoRenameDialog::open(name, this);
    if(nameNew.isEmpty() || name == nameNew) {
        return;
    }
    if(!QWoIdentify::rename(name, nameNew)) {
        QKxMessageBox::warning(this, tr("Warning"), tr("failed to rename '%1' to '%2'").arg(name).arg(nameNew));
        return;
    }
    reload();
}

void QWoIdentifyDialog::onButtonPublicViewClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString key = idx2.data(ROLE_IDENTIFY_PUBKEY).toString();
    if(key.isEmpty()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QString name = idx2.data().toString();
    QString type = idx2.data(ROLE_IDENTIFY_TYPE).toString();
    QString content = type + " " + key + " " + name;
    QWoIdentifyKeyContentDialog dlg(true, content, this);
    dlg.exec();
}

void QWoIdentifyDialog::onButtonPrivateViewClicked()
{
    if(!isAdministrator()) {
        return;
    }
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString key = idx2.data(ROLE_IDENTIFY_PRVKEY).toString();
    if(key.isEmpty()) {
        QKxMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QWoIdentifyKeyContentDialog dlg(false, key, this);
    dlg.exec();
}

void QWoIdentifyDialog::onItemDoubleClicked(QTreeWidgetItem *row, int col)
{
    QString name = row->data(0, Qt::DisplayRole).toString();
    if(name.isEmpty()) {
        return;
    }
    m_result = name;
    if(ui->btnSelect->isVisible()) {
        close();
    }
}

void QWoIdentifyDialog::reload()
{
    QMap<QString, IdentifyInfo> all = QWoIdentify::loadFromSqlite();
    ui->identify->clear();
    for(QMap<QString, IdentifyInfo>::iterator iter = all.begin(); iter != all.end(); iter++) {
        IdentifyInfo info = iter.value();
        QStringList cols;
        cols.append(info.name);
        cols.append(info.type);
        cols.append(info.fingureprint);
        QTreeWidgetItem *item = new QTreeWidgetItem(cols);
        item->setData(0, ROLE_IDENTIFY_PUBKEY, info.pubKey);
        item->setData(0, ROLE_IDENTIFY_PRVKEY, info.prvKey);
        item->setData(0, ROLE_IDENTIFY_TYPE, info.type);
        item->setData(0, ROLE_IDENTIFY_FIGURE, info.fingureprint);
        QFontMetrics fm(ui->identify->font());
        QSize sz = fm.size(Qt::TextSingleLine, cols.at(0)) + QSize(50, 0);
        int csz = ui->identify->columnWidth(0);
        if(sz.width() > csz) {
            ui->identify->setColumnWidth(0, sz.width());
        }
        ui->identify->addTopLevelItem(item);
    }
}

bool QWoIdentifyDialog::isAdministrator()
{
    QString pwdAdmin = QWoSetting::adminPassword();
    if(pwdAdmin.isEmpty()) {
        QKxMessageBox::information(this, tr("Administrator"), tr("Please create administrator's password first!"));
        return false;
    }
    QString pass = QWoUtils::getPassword(this, tr("Please input the administrator password"));
    if(pass.isEmpty()) {
        return false;
    }
    if(pass != QWoSetting::adminPassword()) {
        QKxMessageBox::information(this, tr("Error"), tr("Password error!"));
        return false;
    }
    return true;
}
