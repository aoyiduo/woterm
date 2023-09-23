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


QWoDBMigrateDialog::QWoDBMigrateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoDBMigrateDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Migrate from other tools"));

    QStringList typs = {"XShell", "SecureCRT"};
    ui->type->setModel(new QStringListModel(typs, ui->type));
    ui->dbPath->setReadOnly(true);

    QString lastPath = QWoSetting::value("sessionImport/lastPath").toString();
    ui->dbPath->setText(lastPath);

    int lastSource = QWoSetting::value("sessionImport/lastSource", 0).toInt();
    ui->type->setCurrentIndex(lastSource);

    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onBrowserButtonClicked()));
    QObject::connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onImportSessionButtonClicked()));
    QObject::connect(ui->type, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypsCurrentIndexChanged(int)));

    adjustSize();
}

QWoDBMigrateDialog::~QWoDBMigrateDialog()
{
    delete ui;
}

void QWoDBMigrateDialog::onBrowserButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Xshell session path"), QDir::homePath());
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
        handleXShellSessionImport(ui->dbPath->text(), all);
    }else if(idx == 1) {
        // SecureCRT
        handleSecureCRTSessionImport(ui->dbPath->text(), all);
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

void QWoDBMigrateDialog::onTypsCurrentIndexChanged(int idx)
{
    QWoSetting::setValue("sessionImport/lastSource", idx);
}

// support SSH/Telnet/Localshell/Serial/FTP/RLOGIN
void QWoDBMigrateDialog::handleXShellSessionImport(const QString &path, QMap<QString, QList<HostInfo>>& all)
{
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
