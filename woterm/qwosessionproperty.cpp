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

#include "qwosessionproperty.h"
#include "ui_qwosessionproperty.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qwohostlistmodel.h"
#include "qwosetting.h"
#include "qwolineedit.h"
#include "qwoidentifydialog.h"
#include "qwohostsimplelist.h"
#include "qwosessionmoreproperty.h"
#include "qwosessionrdpproperty.h"
#include "qwosessionvncproperty.h"
#include "qwosessionttyproperty.h"
#include "qwogroupinputdialog.h"
#include "qkxbuttonassist.h"
#include "qkxver.h"
#include "qkxmessagebox.h"

#include <QFileInfo>
#include <QStringListModel>
#include <QIntValidator>
#include <QDebug>
#include <QDir>
#include <QTimer>

EHostType hostType(const QString& txt) {
    if(txt == "SshWithSftp") {
        return SshWithSftp;
    }else if(txt == "SftpOnly") {
        return SftpOnly;
    }else if(txt == "Telnet") {
        return Telnet;
    }else if(txt == "RLogin") {
        return RLogin;
    }else if(txt == "Rdp/Mstsc") {
        return Mstsc;
    }else if(txt == "Vnc") {
        return Vnc;
    }else if(txt == "SerialPort") {
        return SerialPort;
    }
    return SshWithSftp;
}

QString showerType(const QString& txt) {
    if(txt == "SshWithSftp") {
        return "terminal";
    }else if(txt == "SftpOnly") {
        return "terminal";
    }else if(txt == "Telnet") {
        return "terminal";
    }else if(txt == "RLogin") {
        return "terminal";
    }else if(txt == "Rdp/Mstsc") {
        return "mstsc";
    }else if(txt == "Vnc") {
        return "vnc";
    }else if(txt == "SerialPort") {
        return "terminal";
    }
    return "terminal";
}

QString hostType2ShowerType(EHostType t) {
    if(t == SshWithSftp) {
        return "terminal";
    }else if(t == SftpOnly) {
        return "terminal";
    }else if(t == Telnet) {
        return "terminal";
    }else if(t == RLogin) {
        return "terminal";
    }else if(t == Mstsc) {
        return "mstsc";
    }else if(t == Vnc) {
        return "vnc";
    }else if(t == SerialPort) {
        return "terminal";
    }
    return "terminal";
}

QWoSessionProperty::QWoSessionProperty(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSessionProperty)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Session[New]"));
    init();
    resizeWidget();
}

QWoSessionProperty::~QWoSessionProperty()
{
    delete ui;
}

bool QWoSessionProperty::setSession(const QString &name)
{
    m_name = name;
    HostInfo hi = QWoSshConf::instance()->find(name);
    setWindowTitle(tr("Session[%1]").arg(name));

    ui->groupBox->setCurrentText(hi.group);
    ui->name->setText(hi.name);
    ui->host->setText(hi.host);
    ui->port->setText(QString("%1").arg(hi.port));
    ui->loginName->setText(hi.user);
    ui->password->setText(hi.password);
    ui->identify->setText(hi.identityFile);
    if(!hi.password.isEmpty()) {
        ui->loginType->setCurrentText(tr("Password"));
    }else{
        ui->loginType->setCurrentText(tr("Identify"));
    }
    ui->memo->setPlainText(hi.memo);
    ui->command->setText(hi.script);
    ui->proxyJump->setText(hi.proxyJump);
    ui->baudRate->setCurrentText(hi.baudRate);
    ui->dataBits->setCurrentText(hi.dataBits);
    ui->flowControl->setCurrentText(hi.flowControl);
    ui->stopBits->setCurrentText(hi.stopBits);
    ui->parity->setCurrentText(hi.parity);

    if(hi.type == SshWithSftp) {
        ui->type->setCurrentText("SshWithSftp");
    }else if(hi.type == SftpOnly) {
        ui->type->setCurrentText("SftpOnly");
    }else if(hi.type == Telnet) {
        ui->type->setCurrentText("Telnet");
    }else if(hi.type == RLogin) {
        ui->type->setCurrentText("RLogin");
    }else if(hi.type == Mstsc) {
        ui->type->setCurrentText("Rdp/Mstsc");
    }else if(hi.type == Vnc) {
        ui->type->setCurrentText("Vnc");
    }else if(hi.type == SerialPort) {
        return false;
    }
    return true;
}

void QWoSessionProperty::init()
{
    QWoLineEdit::decorator(ui->identify);
    ui->identify->setReadOnly(true);
    QWoLineEdit::decorator(ui->proxyJump);
    ui->proxyJump->setReadOnly(true);
    {
        QStringList ways;
        ways.append("SshWithSftp");
        ways.append("SftpOnly");
        ways.append("Telnet");
        ways.append("RLogin");
        ways.append("Rdp/Mstsc");
        ways.append("Vnc");
        ui->type->setModel(new QStringListModel(ways, this));
        QObject::connect(ui->type, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTypeCurrentIndexChanged(QString)));
        ui->type->setCurrentIndex(0);
        QWoUtils::setLayoutVisible(ui->localLayout, false);
        QWoUtils::setLayoutVisible(ui->remoteLayout, true);
    }
    //-----------------Remoe--------
    {
        QStringList ways;
        ways.append(tr("Password"));
        ways.append(tr("Identify"));
        ui->loginType->setModel(new QStringListModel(ways, this));
        QObject::connect(ui->loginType, SIGNAL(currentIndexChanged(QString)), this, SLOT(onAuthCurrentIndexChanged(QString)));
        ui->loginType->setCurrentIndex(0);
        onAuthCurrentIndexChanged(ui->loginType->currentText());
    }
    {
        ui->command->setPlaceholderText("ls && df");
        ui->port->setText("22");
        ui->port->setValidator(new QIntValidator(10, 65535));
        ui->portTip->setVisible(false);
        QObject::connect(ui->port, SIGNAL(textChanged(QString)), this, SLOT(onPortTextChanged(QString)));
        ui->password->setEchoMode(QLineEdit::Password);
        if(QKxVer::isUltimate()) {
            QKxButtonAssist *assist = new QKxButtonAssist(":/woterm/resource/skin/eye.png", ui->password);
            QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
        }
    }

    //---------------------Local-----------
    {
        QStringList rate;
        rate.append("1200");
        rate.append("2400");
        rate.append("4800");
        rate.append("9600");
        rate.append("19200");
        rate.append("38400");
        rate.append("57600");
        rate.append("115200");
        ui->baudRate->setModel(new QStringListModel(rate, this));
        ui->baudRate->setCurrentText("9600");
    }
    {
        QStringList bits;
        bits.append("5");
        bits.append("6");
        bits.append("7");
        bits.append("8");
        ui->dataBits->setModel(new QStringListModel(bits, this));
        ui->dataBits->setCurrentText("8");
    }
    {
        QStringList parity;
        parity.append("None");
        parity.append("Even");
        parity.append("Odd");
        parity.append("Space");
        parity.append("Mark");
        ui->parity->setModel(new QStringListModel(parity, this));
        ui->parity->setCurrentText("None");
    }
    {
        QStringList bits;
        bits.append("1");
        bits.append("1.5");
        bits.append("2");
        ui->stopBits->setModel(new QStringListModel(bits, this));
        ui->stopBits->setCurrentText("1");
    }
    {
        QStringList flow;
        flow.append("None");
        flow.append("RTS/CTS");
        flow.append("XON/XOFF");
        ui->flowControl->setModel(new QStringListModel(flow, this));
        ui->flowControl->setCurrentText("None");
    }

    QObject::connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(onTypeConnect()));
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onTypeSave()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    QObject::connect(ui->proxyBrowser, SIGNAL(clicked()), this, SLOT(onProxyJumpeBrowser()));
    QObject::connect(ui->identifyBrowser, SIGNAL(clicked()), this, SLOT(onIdentifyFileBrowser()));
    QObject::connect(ui->btnMore, SIGNAL(clicked()), this, SLOT(onMoreConfig()));

    QObject::connect(ui->btnGroupAdd, SIGNAL(clicked()), this, SLOT(onGroupAddCliecked()));

    QStringList names = QWoSshConf::instance()->groupNameList();
    QStringListModel *model = new QStringListModel(names, ui->groupBox);
    ui->groupBox->setModel(model);
}

void QWoSessionProperty::updatePortTipState()
{
    QString txt = ui->portTip->text();
    for(int i = 0; i < txt.length(); i++) {
        if(txt.at(i).isDigit()) {
            QString port = txt.mid(i);
            QString type = ui->type->currentText();
            if(type == "Vnc") {
                ui->portTip->setVisible(!port.startsWith("590"));
            }else{
                ui->portTip->setVisible(port != ui->port->text());
            }
            return;
        }
    }
}

void QWoSessionProperty::resizeWidget()
{
    QSize sz = minimumSizeHint();
    resize(width(), sz.height());
}

void QWoSessionProperty::onTypeCurrentIndexChanged(const QString &type)
{
    ui->btnMore->setVisible(true);
    QWoUtils::setLayoutVisible(ui->localLayout, type == "SerialPort");
    QWoUtils::setLayoutVisible(ui->remoteLayout, type != "SerialPort");
    if(type == "SshWithSftp") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, true);
        QWoUtils::setLayoutVisible(ui->commandLayout, true);
        QWoUtils::setLayoutVisible(ui->proxyLayout, true);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, true);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, true);
        onAuthCurrentIndexChanged(ui->loginType->currentText());
        if(m_name.isEmpty() &&
                (ui->port->text() == "23" ||
                 ui->port->text() == "513" ||
                 ui->port->text() == "3389" ||
                 ui->port->text().startsWith("590"))) {
            ui->port->setText("22");
        }
        ui->portTip->setText(tr("Default:22"));
    }else if(type == "SftpOnly") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, true);
        QWoUtils::setLayoutVisible(ui->commandLayout, false);
        QWoUtils::setLayoutVisible(ui->proxyLayout, true);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, true);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, false);
        onAuthCurrentIndexChanged(ui->loginType->currentText());
        if(m_name.isEmpty() &&
                (ui->port->text() == "23" ||
                 ui->port->text() == "513" ||
                 ui->port->text() == "3389"||
                 ui->port->text().startsWith("590"))) {
            ui->port->setText("22");
        }
        ui->portTip->setText(tr("Default:22"));
        ui->btnMore->setVisible(false);
    }else if(type == "Telnet") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, false);
        QWoUtils::setLayoutVisible(ui->commandLayout, false);
        QWoUtils::setLayoutVisible(ui->proxyLayout, false);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, true);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, false);
        if(m_name.isEmpty() &&
                (ui->port->text() == "22" ||
                 ui->port->text() == "513" ||
                 ui->port->text() == "3389" ||
                 ui->port->text().startsWith("590"))) {
            ui->port->setText("23");
        }
        ui->portTip->setText(tr("Default:23"));
    }else if(type == "RLogin") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, false);
        QWoUtils::setLayoutVisible(ui->commandLayout, false);
        QWoUtils::setLayoutVisible(ui->proxyLayout, false);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, true);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, false);
        if(m_name.isEmpty() &&
                (ui->port->text() == "22"||
                 ui->port->text() == "23"||
                 ui->port->text() == "3389"||
                 ui->port->text().startsWith("590"))) {
            ui->port->setText("513");
        }
        ui->portTip->setText(tr("Default:513"));
    }else if(type == "Rdp/Mstsc") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, false);
        QWoUtils::setLayoutVisible(ui->commandLayout, false);
        QWoUtils::setLayoutVisible(ui->proxyLayout, false);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, true);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, false);
        if(m_name.isEmpty() &&
                (ui->port->text() == "22" ||
                 ui->port->text() == "23" ||
                 ui->port->text() == "513"||
                 ui->port->text().startsWith("590"))) {
            ui->port->setText("3389");
        }
        ui->portTip->setText(tr("Default:3389"));
    }else if(type == "Vnc") {
        QWoUtils::setLayoutVisible(ui->loginTypeLayout, false);
        QWoUtils::setLayoutVisible(ui->commandLayout, false);
        QWoUtils::setLayoutVisible(ui->proxyLayout, false);
        QWoUtils::setLayoutVisible(ui->loginNameLayout, false);
        QWoUtils::setLayoutVisible(ui->passwordLayout, true);
        QWoUtils::setLayoutVisible(ui->identifyLayout, false);
        if(m_name.isEmpty() &&
                (ui->port->text() == "22" ||
                 ui->port->text() == "23" ||
                 ui->port->text() == "513" ||
                 ui->port->text() == "3389")) {
            ui->port->setText("5901");
        }
        ui->portTip->setText(tr("Default:590x"));
    }
    if(type == "SerialPort"){
        ui->portTip->setVisible(false);
    }else{
        updatePortTipState();
    }
    resizeWidget();
}

void QWoSessionProperty::onTypeConnect()
{
    if(!saveConfig()) {
        return;
    }
    QString name = ui->name->text();
    QString type = ui->type->currentText();
    if(type == "SshWithSftp") {
        readyToConnect(name, EOT_SSH);
    }else if(type == "SftpOnly") {
        readyToConnect(name, EOT_SFTP);
    }else if(type == "Telnet") {
        readyToConnect(name, EOT_TELNET);
    }else if(type == "RLogin") {
        readyToConnect(name, EOT_RLOGIN);
    }else if(type == "Rdp/Mstsc") {
        readyToConnect(name, EOT_MSTSC);
    }else if(type == "Vnc") {
        readyToConnect(name, EOT_VNC);
    }else if(type == "SerialPort") {
        return;
    }
    done(Connect);
}

void QWoSessionProperty::onTypeSave()
{
    if(!saveConfig()) {
        return;
    }
    done(Save);
}

void QWoSessionProperty::onAuthCurrentIndexChanged(const QString &type)
{
    bool isPassword = type == tr("Password");
    QWoUtils::setLayoutVisible(ui->identifyLayout, !isPassword);
    QWoUtils::setLayoutVisible(ui->passwordLayout, isPassword);
}

void QWoSessionProperty::onPortTextChanged(const QString &txt)
{
    updatePortTipState();
}

void QWoSessionProperty::onAssistButtonClicked(int idx)
{
    QLineEdit::EchoMode mode = ui->password->echoMode();
    if(mode == QLineEdit::Normal) {
        ui->password->setEchoMode(QLineEdit::Password);
    }else{
        QString pwdAdmin = QWoSetting::adminPassword();
        if(pwdAdmin.isEmpty()) {
            QKxMessageBox::information(this, tr("Administrator"), tr("Please create administrator's password first!"));
            return;
        }
        QString pass = QWoUtils::getPassword(this, tr("Please input the administrator password"));
        if(pass.isEmpty()) {
            return;
        }
        if(pass != QWoSetting::adminPassword()) {
            QKxMessageBox::information(this, tr("Error"), tr("Password error!"));
            return;
        }
        ui->password->setEchoMode(QLineEdit::Normal);
        QTimer::singleShot(1000*5, this, SLOT(onSetEditToPasswordMode()));
    }
}

void QWoSessionProperty::onProxyJumpeBrowser()
{
    QWoHostSimpleList dlg(SshBase, this);
    dlg.exec();
    HostInfo hi;
    if(!dlg.result(&hi)) {
        return;
    }
    ui->proxyJump->setText(hi.name);
}

void QWoSessionProperty::onIdentifyFileBrowser()
{
    QString fileName = QWoIdentifyDialog::open(false, this);
    if(fileName.isEmpty()) {
        return;
    }
    qDebug() << "fileName" << fileName;
    fileName = QDir::toNativeSeparators(fileName);
    ui->identify->setText(fileName);
}

void QWoSessionProperty::onMoreConfig()
{
    QString txt = ui->type->currentText();
    QString shower = showerType(txt);
    EHostType type = hostType(txt);
    QVariantMap prop = m_props.value(shower);
    if(prop.isEmpty() && !m_name.isEmpty()) {
        HostInfo hi = QWoSshConf::instance()->find(m_name);
        QString shower2 = hostType2ShowerType(hi.type);
        if(shower2 == shower && !hi.property.isEmpty()) {
            prop = QWoUtils::qBase64ToVariant(hi.property).toMap();
        }
    }
    if(type == Mstsc) {
        QWoSessionRDPProperty dlg(this);
        dlg.setCustom(prop);
        dlg.exec();
        QVariantMap result = dlg.result();
        if(!result.isEmpty()) {
            m_props.insert(shower, result);
        }
    }else if(type == Vnc) {
        QWoSessionVNCProperty dlg(this);
        dlg.setCustom(prop);
        dlg.exec();
        QVariantMap result = dlg.result();
        if(!result.isEmpty()) {
            m_props.insert(shower, result);
        }
    }else{
        QWoSessionTTYProperty dlg(QWoSessionTTYProperty::ETTY_RemoteTarget, this);
        dlg.setCustom(prop);
        dlg.exec();
        QVariantMap result = dlg.result();
        if(!result.isEmpty()) {
            m_props.insert(shower, result);
        }
    }
}

void QWoSessionProperty::onGroupAddCliecked()
{
    QWoGroupInputDialog dlg(tr(""), 0, this);
    QStringList names = QWoSshConf::instance()->groupNameList();
    QPointer<QWoGroupInputDialog> dlgPtr(&dlg);
    QObject::connect(&dlg, &QWoGroupInputDialog::apply, this, [=](const QString& name, int order){
        if(names.contains(name)) {
            QKxMessageBox::information(dlgPtr, tr("Parameter error"), tr("The group name is already exist."));
            return;
        }
        QStringList mynames = names;
        mynames.append(name);
        QStringListModel *model = new QStringListModel(mynames, ui->groupBox);
        ui->groupBox->setModel(model);
        QWoSshConf::instance()->updateGroup(name, order);
        dlgPtr->close();
    });
    dlg.exec();
}

void QWoSessionProperty::onSetEditToPasswordMode()
{
    ui->password->setEchoMode(QLineEdit::Password);
}

bool QWoSessionProperty::saveConfig()
{
    HostInfo hi;
    QString type = ui->type->currentText();
    if(type == "SshWithSftp") {
        hi.type = SshWithSftp;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.user = ui->loginName->text();
        if(ui->loginType->currentText() == tr("Password")) {
            hi.password = ui->password->text();
        }else{
            hi.identityFile = ui->identify->text();
            if(hi.identityFile.isEmpty()) {
                QKxMessageBox::warning(this, tr("Info"), tr("The identity file can't be empty"));
                return false;
            }
        }
        hi.proxyJump = ui->proxyJump->text();
        hi.memo = ui->memo->toPlainText();
        hi.script = ui->command->text();
        if(!hi.name.isEmpty() && hi.name == hi.proxyJump) {
            QKxMessageBox::warning(this, tr("Info"), tr("ProxyJump can't be same with name, change to another one."));
            return false;
        }
        if(hi.user.isEmpty()) {
            QKxMessageBox::warning(this, tr("Info"), tr("The userName can't be empty"));
            return false;
        }
    }else if(type == "SftpOnly") {
        hi.type = SftpOnly;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.user = ui->loginName->text();
        if(ui->loginType->currentText() == tr("Password")) {
            hi.password = ui->password->text();
        }else{
            hi.identityFile = QWoUtils::nameToPath(ui->identify->text());
            if(hi.identityFile.isEmpty()) {
                QKxMessageBox::warning(this, tr("Info"), tr("The identify file can't be empty"));
                return false;
            }
            QString identify = QWoSetting::identityFilePath() + "/" + hi.identityFile;
            if(!QFileInfo::exists(identify)) {
                QKxMessageBox::warning(this, tr("Info"), tr("failed to find the identify file"));
                return false;
            }
            hi.identityFile.insert(0, "woterm:");
        }
        hi.proxyJump = ui->proxyJump->text();
        hi.memo = ui->memo->toPlainText();
        if(!hi.name.isEmpty() && hi.name == hi.proxyJump) {
            QKxMessageBox::warning(this, tr("Info"), tr("ProxyJump can't be same with name, change to another one."));
            return false;
        }
        if(hi.user.isEmpty()) {
            QKxMessageBox::warning(this, tr("Info"), tr("The userName can't be empty"));
            return false;
        }
    }else if(type == "Telnet") {
        hi.type = Telnet;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.user = ui->loginName->text();
        hi.password = ui->password->text();
        hi.memo = ui->memo->toPlainText();
    }else if(type == "RLogin") {
        hi.type = RLogin;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.user = ui->loginName->text();
        hi.password = ui->password->text();
        hi.memo = ui->memo->toPlainText();
    }else if(type == "Rdp/Mstsc") {
        hi.type = Mstsc;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.user = ui->loginName->text();
        hi.password = ui->password->text();
        hi.memo = ui->memo->toPlainText();
    }else if(type == "SerialPort") {
        hi.type = SerialPort;
        hi.name = ui->name->text();
        hi.baudRate = ui->baudRate->currentText();
        hi.dataBits = ui->dataBits->currentText();
        hi.stopBits = ui->stopBits->currentText();
        hi.flowControl = ui->flowControl->currentText();
        hi.parity = ui->parity->currentText();
        hi.memo = ui->memo->toPlainText();
    }else if(type == "Vnc") {
        hi.type = Vnc;
        hi.name = ui->name->text();
        hi.host = ui->host->text();
        hi.port = ui->port->text().toInt();
        hi.password = ui->password->text();
        hi.memo = ui->memo->toPlainText();
    }
    if(!m_name.isEmpty()) {
        HostInfo hit = QWoSshConf::instance()->find(m_name);
        if((hi.type != Mstsc && hit.type != Mstsc)
                || (hi.type == Mstsc && hit.type == Mstsc)) {
            hi.property = hit.property;
        }
    }
    if(hi.name.isEmpty()) {
        QKxMessageBox::warning(this, tr("Info"), tr("The name can't be empty"));
        return false;
    }
    if(hi.type != SerialPort) {
        if(hi.host.isEmpty()) {
            QKxMessageBox::warning(this, tr("Info"), tr("The host can't be empty"));
            return false;
        }

        if(hi.port < 10 || hi.port > 65535) {
            QKxMessageBox::warning(this, tr("Info"), tr("The port should be at [10,65535]"));
            return false;
        }
    }
    if(m_name != hi.name) {
        if(!m_name.isEmpty() && QWoHostListModel::instance()->exists(hi.name)) {
            QKxMessageBox::warning(this, tr("Info"), tr("The session name had been used, Change to another name."));
            return false;
        }
    }
    QString key = hostType2ShowerType(hi.type);
    QVariantMap prop = m_props.value(key);
    if(!prop.isEmpty()) {
        hi.property = QWoUtils::qVariantToBase64(prop);
    }
    hi.group = ui->groupBox->currentText();
    QWoHostListModel::instance()->modifyOrAppend(hi);
    return true;
}
