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

#include "qwodbmigratedialog.h"
#include "ui_qwodbmigratedialog.h"

#include "qkxmessagebox.h"
#include "qwosetting.h"


#include <QStringListModel>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QTextCodec>
#include <QStandardPaths>
#include <QDir>
#include <QDomDocument>

QWoDBMigrateDialog::QWoDBMigrateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDBMigrateDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Migrate from other tools"));

    QStringList typs = {tr("XShell Sessions directory"), tr("SecureCRT sessions directory"), tr("SecureCRT XML export file")};
    ui->type->setModel(new QStringListModel(typs, ui->type));
    ui->dbPath->setReadOnly(true);

    QString lastPath = QWoSetting::value("sessionImport/lastPath").toString();
    ui->dbPath->setText(lastPath);

    int lastSource = QWoSetting::value("sessionImport/lastSource", 0).toInt();
    ui->type->setCurrentIndex(lastSource);

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onBrowserButtonClicked()));
    QObject::connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onImportSessionButtonClicked()));
    QObject::connect(ui->btnSearch, SIGNAL(clicked()), this, SLOT(onSearchSessionPathButtonClicked()));
    QObject::connect(ui->type, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypsCurrentIndexChanged(int)));

#ifdef Q_OS_WIN
    ui->btnSearch->setVisible(true);
#else
    ui->btnSearch->setVisible(false);
#endif
    adjustSize();
}

QWoDBMigrateDialog::~QWoDBMigrateDialog()
{
    delete ui;
}

void QWoDBMigrateDialog::onBrowserButtonClicked()
{
    QString path;
    int idx = ui->type->currentIndex();
    if(idx == 0) {
        // xshell
        path = QFileDialog::getExistingDirectory(this, tr("Xshell session path"), QDir::homePath());
    }else if(idx == 1) {
        // SecureCRT
        path = QFileDialog::getExistingDirectory(this, tr("SecureCRT session path"), QDir::homePath());
    }else if(idx == 2) {
        // SecureCRT file
        path = QFileDialog::getOpenFileName(this, tr("SecureCRT xml backup file"), QDir::homePath(), "XML files(*.xml)");
    }

    if(path.isEmpty()) {
        return;
    }

    ui->dbPath->setText(path);
    QWoSetting::setValue("sessionImport/lastPath", path);
}

void QWoDBMigrateDialog::onImportSessionButtonClicked()
{
    QString path = ui->dbPath->text();
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Session path"), tr("Please specify the file path for session import."));
        return;
    }

    QMap<QString, QList<HostInfo>> all;
    int idx = ui->type->currentIndex();
    if(idx == 0) {
        // xshell
        QFileInfo fi(ui->dbPath->text());
        if(!fi.isDir()) {
            QKxMessageBox::information(this, tr("Session path"), tr("Please specify the file path for session import."));
            return;
        }
        handleXShellSessionImport(ui->dbPath->text(), all);
    }else if(idx == 1) {
        // SecureCRT
        QFileInfo fi(ui->dbPath->text());
        if(!fi.isDir()) {
            QKxMessageBox::information(this, tr("Session path"), tr("Please specify the file path for session import."));
            return;
        }
        handleSecureCRTSessionImport(ui->dbPath->text(), all);
    }else if(idx == 2) {
        // SecureCRT file
        QFileInfo fi(ui->dbPath->text());
        if(!fi.isFile()) {
            QKxMessageBox::information(this, tr("Session xml file"), tr("Please specify the xml file for session import."));
            return;
        }
        handleSecureCRTSessionXMLImport(ui->dbPath->text(), all);
    }
    int total = 0;
    for(auto it = all.begin(); it != all.end(); it++) {
        QString group = it.key();
        QList<HostInfo> his = it.value();
        QWoSshConf::instance()->updateGroup(group);
        for(int i = 0; i < his.length(); i++) {
            const HostInfo& hi = his.at(i);
            QWoSshConf::instance()->modifyOrAppend(hi);
            total++;
        }
    }

    QKxMessageBox::information(this, tr("Import information"), tr("Import a total of %1 session records.").arg(total));
}

void QWoDBMigrateDialog::onSearchSessionPathButtonClicked()
{
    QString path = QDir::homePath();
    int idx = ui->type->currentIndex();
    if(idx == 0) {
        // xshell
        path += "/Documents/NetSarang Computer";
        QFileInfo fi(path);
        if(!fi.exists()) {
            QKxMessageBox::information(this, tr("Sessions directory"), tr("The session save path for the specified program was not found, but you can specify one."));
            return;
        }
        QDir d(path);
        QFileInfoList lsfi = d.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
            QFileInfo fi = *it;
            QString path = fi.absoluteFilePath();
            path += "/Xshell/Sessions";
            QFileInfo hit(path);
            if(!hit.exists()) {
                continue;
            }
            ui->dbPath->setText(hit.absoluteFilePath());
        }
    }else if(idx == 1) {
        // SecureCRT
        path += "/AppData/Roaming/VanDyke/Config/Sessions";
        QFileInfo fi(path);
        if(!fi.exists()) {
            QKxMessageBox::information(this, tr("Sessions directory"), tr("The session save path for the specified program was not found, but you can specify one."));
            return;
        }
        ui->dbPath->setText(fi.absoluteFilePath());
    }else{
        ui->dbPath->setText("");
    }
}

void QWoDBMigrateDialog::onTypsCurrentIndexChanged(int idx)
{
    QWoSetting::setValue("sessionImport/lastSource", idx);
}

// support SSH/Telnet/Localshell/Serial/FTP/RLOGIN
void QWoDBMigrateDialog::handleXShellSessionImport(const QString &path, QMap<QString, QList<HostInfo>>& all)
{
    QFileInfo fi(path);
    if(!fi.isDir()) {
        return;
    }
    QDir d(path);
    QList<QFileInfo> infos = d.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
    for(auto it = infos.begin(); it != infos.end(); it++) {
        QFileInfo info = *it;
        QString fileName = info.absoluteFilePath();
        if(info.isDir()) {
            handleXShellSessionImport(fileName, all);
        }else if(fileName.endsWith(".xsh")) {
            QFile f(fileName);
            if(!f.open(QFile::ReadOnly)) {
                continue;
            }
            QByteArray bytesAll = f.readAll();
            QTextCodec *codec = QTextCodec::codecForUtfText(bytesAll);
            QString content = codec->toUnicode(bytesAll);
            QStringList lines = content.split('\n');
            QMap<QString,QString> kvs;
            for(int i = 0; i < lines.length(); i++) {
                QString line = lines.at(i).simplified();
                int pos = line.indexOf('=');
                if(pos < 0) {
                    continue;
                }
                QString key = line.left(pos).simplified();
                QString val = line.mid(pos+1).simplified();
                kvs.insert(key, val);
            }
            QString key = kvs.value("Protocol");
            HostInfo hi;
            hi.name = info.baseName();
            hi.host = kvs.value("Host");
            hi.user = kvs.value("UserName");
            if(key == "SSH") {
                // SSH
                hi.port = kvs.value("Port", "22").toInt();
                hi.type = SshWithSftp;
            }else if(key == "SFTP") {
                hi.port = kvs.value("Port", "22").toInt();
                hi.type = SftpOnly;
            }else if(key == "TELNET") {
                hi.port = kvs.value("Port", "23").toInt();
                hi.type = Telnet;
            }else if(key == "RLOGIN") {
                hi.port = kvs.value("Port", "513").toInt();
                hi.type = RLogin;
            }else {
                continue;
            }
            if(!hi.isValid()) {
                continue;
            }
            QDir group = info.absoluteDir();
            hi.group = group.dirName();
            QList<HostInfo> his = all.take(hi.group);
            his.append(hi);
            all.insert(hi.group, his);
        }
    }
}

// support SSH1/SSH2/Telnet/Localshell/RDP/Serial/TAPI/RAW/RLOGIN
void QWoDBMigrateDialog::handleSecureCRTSessionImport(const QString &path, QMap<QString, QList<HostInfo> > &all)
{
    QFileInfo fi(path);
    if(!fi.isDir()) {
        return;
    }
    QDir d(path);
    QList<QFileInfo> infos = d.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
    for(auto it = infos.begin(); it != infos.end(); it++) {
        QFileInfo info = *it;
        QString fileName = info.absoluteFilePath();
        if(info.isDir()) {
            handleSecureCRTSessionImport(fileName, all);
        }else if(fileName.endsWith(".ini")) {
            QFile f(fileName);
            if(!f.open(QFile::ReadOnly)) {
                continue;
            }
            QByteArray bytesAll = f.readAll();
            QTextCodec *codec = QTextCodec::codecForUtfText(bytesAll);
            QString content = codec->toUnicode(bytesAll);
            QStringList lines = content.split('\n');
            QMap<QString,QString> kvs;
            for(int i = 0; i < lines.length(); i++) {
                QString line = lines.at(i).simplified();
                int pos = line.indexOf('=');
                if(pos < 0) {
                    continue;
                }
                QString key = line.left(pos).simplified();
                QString val = line.mid(pos+1).simplified();
                kvs.insert(key, val);
            }
            QString key = kvs.value("S:\"Protocol Name\"").toLower();
            HostInfo hi;
            hi.name = info.baseName();
            hi.host = kvs.value("S:\"Hostname\"");
            hi.user = kvs.value("S:\"Username\"");
            if(key.startsWith("ssh")) {
                // SSH
                hi.port = kvs.value("D:\"Port\"", "16").toInt(nullptr, 16);
                hi.type = SshWithSftp;
            }else if(key.startsWith("telnet")) {
                hi.port = kvs.value("D:\"Port\"", "17").toInt(nullptr, 16);
                hi.type = Telnet;
            }else if(key.startsWith("rlogin")) {
                hi.port = kvs.value("D:\"Port\"", "0201").toInt(nullptr, 16);
                hi.type = RLogin;
            }else if(key == "rdp") {
                hi.port = kvs.value("D:\"Port\"", "0d3d").toInt(nullptr, 16);
                hi.type = Mstsc;
            }else {
                continue;
            }
            if(!hi.isValid()) {
                continue;
            }
            QDir group = info.absoluteDir();
            hi.group = group.dirName();
            QList<HostInfo> his = all.take(hi.group);
            his.append(hi);
            all.insert(hi.group, his);
        }
    }
}

void QWoDBMigrateDialog::handleSecureCRTSessionXMLImport(const QString &path, QMap<QString, QList<HostInfo> > &all)
{
    QFileInfo fi(path);
    if(!fi.isFile()) {
        return;
    }

    QFile xmlFile(fi.absoluteFilePath());
    if(!xmlFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        return;
    }

    QDomDocument doc;
    if(!doc.setContent(xmlFile.readAll())) {
        return;
    }
    QDomElement vanDyke = doc.firstChildElement("VanDyke");
    if(!vanDyke.isElement()) {
        return;
    }

    QMap<QString, QString> props;
    props.insert("name", "Sessions");
    QList<QDomElement> sessionsAll = findChilds(vanDyke, "key", props);
    if(sessionsAll.isEmpty()) {
        return;
    }
    QDomElement sessions = sessionsAll.first();
    handleSecureCRTSessionXMLImport(sessions, all);
}

void QWoDBMigrateDialog::handleSecureCRTSessionXMLImport(const QDomNode &node, QMap<QString, QList<HostInfo> > &all)
{
    QDomElement el = node.firstChildElement("key");
    while(el.isElement()) {
        QDomElement group = el.firstChildElement("key");
        if(!group.isNull()){
            // group;
            handleSecureCRTSessionXMLImport(el, all);
            el = el.nextSiblingElement("key");
        }else{
            // session;
            HostInfo hi;
            QDomElement group = el.parentNode().toElement();
            if(!group.isElement()) {
                el = el.nextSiblingElement("key");
                continue;
            }
            hi.group = group.attribute("name");
            hi.name = el.attribute("name");
            if(hi.name.isEmpty()) {
                el = el.nextSiblingElement("key");
                continue;
            }
            hi.host = hostName(el);
            if(hi.host.isEmpty()) {
                el = el.nextSiblingElement("key");
                continue;
            }
            hi.user = userName(el);
            QString proto = protocol(el).toLower();
            if(proto.startsWith("ssh")) {
                // SSH
                hi.port = port(el, "22").toInt();
                hi.type = SshWithSftp;
            }else if(proto.startsWith("telnet")) {
                hi.port = port(el, "23").toInt();
                hi.type = Telnet;
            }else if(proto.startsWith("rlogin")) {
                hi.port = port(el, "513").toInt();
                hi.type = RLogin;
            }else if(proto == "rdp") {
                hi.port = port(el, "3389").toInt();
                hi.type = Mstsc;
            }else {
                el = el.nextSiblingElement("key");
                continue;
            }
            QList<HostInfo> his = all.take(hi.group);
            his.append(hi);
            all.insert(hi.group, his);
            el = el.nextSiblingElement("key");
        }
    }
}

QString QWoDBMigrateDialog::hostName(const QDomElement &node, const QString &vdef)
{
    QMap<QString,QString> props;
    props.insert("name", "Hostname");
    QList<QDomElement> els = findChilds(node, "string", props);
    if(els.isEmpty()) {
        return vdef;
    }
    QString v = els.first().text();
    return v.isEmpty() ? vdef : v;
}

QString QWoDBMigrateDialog::userName(const QDomElement &node, const QString &vdef)
{
    QMap<QString,QString> props;
    props.insert("name", "Username");
    QList<QDomElement> els = findChilds(node, "string", props);
    if(els.isEmpty()) {
        return vdef;
    }
    QString v = els.first().text();
    return v.isEmpty() ? vdef : v;
}

QString QWoDBMigrateDialog::protocol(const QDomElement &node, const QString &vdef)
{
    QMap<QString,QString> props;
    props.insert("name", "Protocol Name");
    QList<QDomElement> els = findChilds(node, "string", props);
    if(els.isEmpty()) {
        return vdef;
    }
    QString v = els.first().text();
    return v.isEmpty() ? vdef : v;
}

QString QWoDBMigrateDialog::port(const QDomElement &node, const QString &vdef)
{
    QMap<QString,QString> props;
    props.insert("name", "Port");
    QList<QDomElement> els = findChilds(node, "dword", props);
    if(els.isEmpty()) {
        return vdef;
    }
    QString v = els.first().text();
    return v.isEmpty() ? vdef : v;
}

QList<QDomElement> QWoDBMigrateDialog::findChilds(const QDomElement& node, const QString &name, const QMap<QString, QString> &properties)
{
    QList<QDomElement> els;
    QDomElement el = node.firstChildElement(name);
    while(el.isElement()) {
        bool hit = true;
        for(auto it = properties.begin(); it != properties.end(); it++) {
            if(el.attribute(it.key()) != it.value()) {
                hit = false;
                break;
            }
        }
        if(hit) {
            els.append(el);
        }
        el = el.nextSiblingElement(name);
    }
    return els;
}
