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

#include "qwosetting.h"
#include "qkxmessagebox.h"

#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QTranslator>

#include "qwoutils.h"


QWoSetting::QWoSetting(QObject *parent)
    : QKxSetting(parent)
{

}

QString QWoSetting::ftpTaskPath()
{
    return specialFilePath("ftp");
}

QString QWoSetting::identityFilePath()
{
    return specialFilePath("identify");
}

QString QWoSetting::historyFilePath()
{
    return specialFilePath("history");
}

QString QWoSetting::examplePath()
{
    static QString path = QDir::cleanPath(QApplication::applicationDirPath()+"/../example/");
    return QDir::cleanPath(path);
}

QString QWoSetting::privatePath()
{
    static QString path = QDir::cleanPath(QApplication::applicationDirPath()+"/../private/");
    return path;
}

QString QWoSetting::playbooksPath()
{
    static QString path = QDir::cleanPath(privatePath() + "/playbooks/");
    return path;
}

QString QWoSetting::customPlaybooksPath()
{
    return QWoSetting::value("Playbooks/customPath").toString();
}

void QWoSetting::setCustomPlaybooksPath(const QString &path)
{
    QWoSetting::setValue("Playbooks/customPath", path);
}

QString QWoSetting::privateLanguagesPath()
{
    static QString path = QDir::cleanPath(privatePath() + "/languages/");
    return path;
}

QString QWoSetting::sshServerFilePath()
{
    QString path;
    path = QWoSetting::value("ssh/serverList", "").toString();
    if(!QFile::exists(path)) {
        path = QDir::cleanPath(QWoSetting::applicationDataPath() + "/servers.conf");
        path = QDir::toNativeSeparators(path);
        return path;
    }
    return path;
}

QString QWoSetting::sshServerDbPath()
{
    QString path;
    path = QWoSetting::value("ssh/serverDb", "").toString();
    if(!QFile::exists(path)) {
        path = QDir::cleanPath(QWoSetting::applicationDataPath() + "/server.db");
        path = QDir::toNativeSeparators(path);
        return path;
    }
    return path;
}

QString QWoSetting::sftpTaskDbPath()
{
    QString path;
    path = QWoSetting::value("sftp/taskDb", "").toString();
    if(!QFile::exists(path)) {
        path = QDir::cleanPath(QWoSetting::applicationDataPath() + "/sftpTask.db");
        path = QDir::toNativeSeparators(path);
        return path;
    }
    return path;
}

QString QWoSetting::sftpTaskLogPath()
{
    QString path;
    path = QWoSetting::value("sftp/taskLog", "").toString();
    if(!QFile::exists(path)) {
        path = QWoSetting::ensurePath("sftpLog");
        path = QDir::cleanPath(path);
        path = QDir::toNativeSeparators(path);
        return path;
    }
    return path;
}

QString QWoSetting::tempPath()
{
    QString path;
    path = QWoSetting::value("woterm/temp", "").toString();
    if(!QFile::exists(path)) {
        path = QWoSetting::ensurePath("temp");
        path = QDir::cleanPath(path);
        path = QDir::toNativeSeparators(path);
        return path;
    }
    return path;
}

QString QWoSetting::customFontPath()
{
    QString path = QWoSetting::ensurePath("font");
    path = QDir::cleanPath(path);
    path = QDir::toNativeSeparators(path);
    return path;
}

QString QWoSetting::customKeytabPath()
{
    QString path = QWoSetting::ensurePath("keytab");
    path = QDir::cleanPath(path);
    path = QDir::toNativeSeparators(path);
    return path;
}

QString QWoSetting::sftpCachePath()
{
    QString path = QWoSetting::ensurePath("sftpCache");
    path = QDir::cleanPath(path);
    path = QDir::toNativeSeparators(path);
    return path;
}

QString QWoSetting::sftpViewPath()
{
    QString path = QWoSetting::ensurePath("sftpView");
    path = QDir::cleanPath(path);
    path = QDir::toNativeSeparators(path);
    return path;
}

QString QWoSetting::fileIconCachePath()
{
    QString path;
    path = QWoSetting::ensurePath("fileIcon");
    if(!QFile::exists(path)) {
        return "";
    }
    return path;
}

QString QWoSetting::downloadPath()
{
    QString path;
    path = QWoSetting::value("sftp/downloadPath", "").toString();
    if(!QFile::exists(path)) {
        return "";
    }
    return path;
}

void QWoSetting::setDownloadPath(const QString &path)
{
    QWoSetting::setValue("sftp/downloadPath", path);
}

QString QWoSetting::lastJsLoadPath()
{
    QString path;
    path = QWoSetting::value("js/last", "").toString();
    if(!QFile::exists(path)) {
        return "";
    }
    return path;
}

void QWoSetting::setLastJsLoadPath(const QString &path)
{
    QWoSetting::setValue("js/last", path);
}

QMap<QString, QString> QWoSetting::allLanguages()
{
    static QMap<QString, QString> langs;
    if(langs.isEmpty()) {
        QDir dir(privateLanguagesPath());
        QFileInfoList lsfi = dir.entryInfoList(QStringList("*.qm"), QDir::Files, QDir::Name);
        for(int i = 0;i < lsfi.length(); i++) {
            const QFileInfo& fi = lsfi.at(i);
            QString path = fi.absoluteFilePath();
            QString type = QWoSetting::languageName(path);
            if(!type.isEmpty()){
                langs.insert(type, fi.fileName());
            }else{
                QKxMessageBox::warning(nullptr, "warning", QString("The language file has no name:%1").arg(path));
            }
        }
    }
    return langs;
}

QString QWoSetting::languageTypeAsBCP47Field()
{
    QString name = QWoSetting::value("language/fileName").toString();
    if(name.isEmpty()) {
        QLocale local;
        QStringList langs = local.uiLanguages();
        if(!langs.isEmpty()) {
            QString lang = langs.at(0);
            lang = lang.split('-').at(0);
            if(lang == "zh") {
                return lang;
            }
        }
        return "en";
    }
    QString bcp47Field = name.mid(name.length() - 5, 2);
    return bcp47Field;
}

QStringList QWoSetting::allLanguageNames()
{
    QMap<QString, QString> langs = allLanguages();
    return langs.keys();
}

QString QWoSetting::languagePath(const QString &name)
{
    QMap<QString, QString> langs = allLanguages();
    return langs.value(name);
}

QString QWoSetting::languageName(const QString &path)
{
    QTranslator translator;
    translator.load(path);
    return translator.translate("English", "English");
}

QString QWoSetting::languageFileName()
{
    QString name = QWoSetting::value("language/fileName").toString();
    int pos = name.lastIndexOf('/');
    if(pos > 0) {
        name = name.mid(pos+1);
    }
    if(name.isEmpty()) {
        QLocale local;
        QStringList langs = local.uiLanguages();
        if(!langs.isEmpty()) {
            QString lang = langs.at(0);
            lang = lang.split('-').at(0);
            QString path = QString("woterm_%1.qm").arg(lang.toLower());
            QMap<QString,QString> langs = allLanguages();
            if(langs.values().contains(path)) {
                return path;
            }
        }
        return QString("woterm_en.qm");;
    }
    return name;
}

QString QWoSetting::absoluteLanguageFilePath()
{
    return privateLanguagesPath() + "/" + languageFileName();
}

void QWoSetting::setLanguageFileName(const QString &lang)
{
    QWoSetting::setValue("language/fileName", lang);
}

bool QWoSetting::isChineseLanguageFile()
{
    QString fileName = languageFileName();
    return fileName.endsWith("_zh.qm");
}

QString QWoSetting::lastBackupPath()
{
    return value("backup/path", "").toString();
}

void QWoSetting::setLastBackupPath(const QString &path)
{
    setValue("backup/path", path);
}

bool QWoSetting::shouldReportLicense()
{
    QDate dt = value("license/report").toDate();
    if(!dt.isValid()) {
        return true;
    }
    QDate today = QDate::currentDate();
    return today >= dt;
}

void QWoSetting::setIgnoreTodayReportLicense()
{
    QDate dt = QDate::currentDate();
    dt = dt.addDays(1);
    setValue("license/report", dt);
}

bool QWoSetting::isAlwayFreeVersion()
{
    return value("upgrade/alwayFreeVersion", false).toBool();
}

void QWoSetting::setAlwayFreeVersion(bool on)
{
    setValue("upgrade/alwayFreeVersion", on);
}

bool QWoSetting::shouldPopupUpgradeUltimate()
{
    QDate dt = value("upgrade/ultimate").toDate();
    if(!dt.isValid()) {
        return true;
    }
    QDate today = QDate::currentDate();
    return today >= dt;
}

void QWoSetting::setIgnoreTodayUpgradeUltimate()
{
    QDate dt = QDate::currentDate();
    dt = dt.addDays(1);
    setValue("upgrade/ultimate", dt);
}

bool QWoSetting::shouldPopupUpgradeVersionMessage(const QString &ver)
{
    QString verHit = value("upgrade/version").toString();
    if(verHit != ver) {
        return true;
    }
    bool skip = value("upgrade/skip", false).toBool();
    if(skip) {
        return false;
    }
    QDate dt = value("upgrade/date").toDate();
    if(!dt.isValid()) {
        return true;
    }
    QDate today = QDate::currentDate();
    return today >= dt;
}

void QWoSetting::setNextUpgradeVersionDate(const QString& ver, const QDate &dt)
{
    setValue("upgrade/version", ver);
    setValue("upgrade/date", dt);
    remove("upgrade/skip");
}

void QWoSetting::setIgnoreTodayUpgradeVersion(const QString &ver)
{
    QDate dt = QDate::currentDate();
    dt = dt.addDays(1);
    setNextUpgradeVersionDate(ver, dt);
}

void QWoSetting::setSkipThisVersion(const QString &ver)
{
    setValue("upgrade/version", ver);
    setValue("upgrade/skip", true);
    remove("upgrade/date");
}

bool QWoSetting::isListModel(const QString &where)
{
    return value("sessionList/"+where, false).toBool();
}

void QWoSetting::setListModel(const QString &where, bool isList)
{
    setValue("sessionList/"+where, isList);
}


QString QWoSetting::adminPassword()
{
    QByteArray enkey = value("admin/password", QByteArray()).toByteArray();
    if(enkey.isEmpty()) {
        return QByteArray();
    }
    QByteArray pass = QWoUtils::aesDecrypt(enkey, "WoTerm@2022-11-6");
    return QString::fromUtf8(pass);
}

void QWoSetting::setAdminPassword(const QString &pass)
{
    if(pass.isEmpty()) {
        return;
    }
    QByteArray enkey = QWoUtils::aesEncrypt(pass.toUtf8(), "WoTerm@2022-11-6");
    setValue("admin/password", enkey);
}

QStringList QWoSetting::sessionsGroupExpand(const QString& where)
{
    return value("admin/sessionGroupExpand"+where).toStringList();
}

void QWoSetting::setSessionsGroupExpand(const QString& where, const QStringList &expands)
{
    setValue("admin/sessionGroupExpand"+where, expands);
}

bool QWoSetting::startupByAdmin()
{
    return value("admin/startup", false).toBool();
}

void QWoSetting::setStartupByAdmin(bool on)
{
    setValue("admin/startup", on);
}

bool QWoSetting::allowOtherHostConnectToTunnel()
{
    return value("admin/connectToTunnel", false).toBool();
}

void QWoSetting::setAllowOtherHostConnectToTunnel(bool on)
{
    setValue("admin/connectToTunnel", on);
}

bool QWoSetting::tunnelRunAsDaemon()
{
    return value("admin/tunnelAsDaemon", false).toBool();
}

void QWoSetting::setTunnelRunAsDaemon(bool on)
{
    setValue("admin/tunnelAsDaemon", on);
}

bool QWoSetting::lookupPasswordByAdmin()
{
    return value("admin/lookupPassword", false).toBool();
}

void QWoSetting::setLookupPasswordByAdmin(bool on)
{
    setValue("admin/lookupPassword", on);
}

bool QWoSetting::localTerminalOnAppStart()
{
    return value("admin/terminalAutoStart", false).toBool();
}

void QWoSetting::setLocalTerminalOnAppStart(bool on)
{
    setValue("admin/terminalAutoStart", on);
}

int QWoSetting::windownOpacity()
{
    return value("window/opacity", 80).toInt();
}

void QWoSetting::setWindowOpacity(int level)
{
    setValue("window/opacity", level);
}

bool QWoSetting::windownOpacityEnable()
{
    return value("window/opacityEnable", false).toBool();
}

void QWoSetting::setWindowOpacityEnable(bool on)
{
    setValue("window/opacityEnable", on);
}

bool QWoSetting::allowToSetWindowOpacity()
{
    return value("admin/allowOpacity", false).toBool();
}

void QWoSetting::setAllowToSetWindowOpacity(bool on)
{
    setValue("admin/allowOpacity", on);
}

bool QWoSetting::allowToUseExternalIdentityFiles()
{
    return value("admin/allowExternalKeys", true).toBool();
}

void QWoSetting::setAllowToUseExternalIdentityFiles(bool on)
{
    setValue("admin/allowExternalKeys", on);
}

bool QWoSetting::enableUserHomeIdentityFiles()
{
    return value("admin/userHomeIdentityFiles", true).toBool();
}

void QWoSetting::setEnableUserHomeIdentityFiles(bool on)
{
    setValue("admin/userHomeIdentityFiles", on);
}

bool QWoSetting::enableLocalSshAgent()
{
    return value("admin/localSshAgent", true).toBool();
}

void QWoSetting::setEnableLocalSshAgent(bool on)
{
    setValue("admin/localSshAgent", on);
}

bool QWoSetting::enableRemoteSshAgent()
{
    return value("admin/remoteSshAgent", true).toBool();
}

void QWoSetting::setEnableRemoteSshAgent(bool on)
{
    setValue("admin/remoteSshAgent", on);
}

QString QWoSetting::remoteSshAgentAddress()
{
    return value("admin/remoteSshAgentAddress", "127.0.0.1:1200").toString();
}

void QWoSetting::setRemoteSshAgentAddress(const QString &addr)
{
    setValue("admin/remoteSshAgentAddress", addr);
}

QString QWoSetting::lastLocalSshAgentAddress()
{
    return value("admin/localSshAgentAddress", "").toString();
}

void QWoSetting::setLastLocalSshAgentAddress(const QString& addr)
{
    setValue("admin/localSshAgentAddress", addr);
}

QByteArray QWoSetting::sshAuthSockDefault()
{
#ifdef Q_OS_WIN
    return "\\\\.\\pipe\\openssh-ssh-agent";
    // windows like pipe: \\.\\pipe\\openssh-ssh-agent
#else
    return qgetenv("SSH_AUTH_SOCK");
    // unix socket: **.sock.
#endif
}

bool QWoSetting::allowSftpToOpenFile()
{
    return value("admin/sftpOpenFile", true).toBool();
}

void QWoSetting::setAllowSftpToOpenFile(bool on)
{
    setValue("admin/sftpOpenFile", on);
}

bool QWoSetting::serialportOnAppStart()
{
    return value("admin/serialportAutoStart", false).toBool();
}

void QWoSetting::setSerialportOnAppStart(bool on)
{
    setValue("admin/serialportAutoStart", on);
}

QVariantMap QWoSetting::localShell()
{
    QString val = QWoSetting::value("property/localShell").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    return mdata;
}

void QWoSetting::setLocalShell(const QVariantMap &dm)
{
    QString v = QWoUtils::qVariantToBase64(dm);
    QWoSetting::setValue("property/localShell", v);
}

QVariantMap QWoSetting::serialPort()
{
    QString val = QWoSetting::value("property/serialPort").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    return mdata;
}

void QWoSetting::setSerialPort(const QVariantMap &dm)
{
    QString v = QWoUtils::qVariantToBase64(dm);
    QWoSetting::setValue("property/serialPort", v);
}

QVariantMap QWoSetting::rdpDefault()
{
    QString val = QWoSetting::value("property/rdpDefault").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    return mdata;
}

void QWoSetting::setRdpDefault(const QVariantMap &dm)
{
    QString v = QWoUtils::qVariantToBase64(dm);
    QWoSetting::setValue("property/rdpDefault", v);
}

QVariantMap QWoSetting::vncDefault()
{
    QString val = QWoSetting::value("property/vncDefault").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    return mdata;
}

void QWoSetting::setVncDefault(const QVariantMap &dm)
{
    QString v = QWoUtils::qVariantToBase64(dm);
    QWoSetting::setValue("property/vncDefault", v);
}

QVariantMap QWoSetting::ttyDefault()
{
    QString val = QWoSetting::value("property/ttyDefault").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    return mdata;
}

void QWoSetting::setTtyDefault(const QVariantMap &dm)
{
    QString v = QWoUtils::qVariantToBase64(dm);
    QWoSetting::setValue("property/ttyDefault", v);
}

bool QWoSetting::tunnelMultiplex()
{
    return QWoSetting::value("tunnel/multiplexEnabled", true).toBool();
}

void QWoSetting::setTunnelMultiplex(bool on)
{
    QWoSetting::setValue("tunnel/multiplexEnabled", on);
}

int QWoSetting::tunnelMultiplexMaxCount()
{
    return QWoSetting::value("tunnel/multiplexCount", 1).toInt();
}

void QWoSetting::setTunnelMultipleMaxCount(int cnt)
{
    QWoSetting::setValue("tunnel/multiplexCount", cnt);
}

QString QWoSetting::terminalBackgroundImage()
{
    return QWoSetting::value("terminal/backgroundImage").toString();
}

void QWoSetting::setTerminalBackgroundImage(const QString &path)
{
    QWoSetting::setValue("terminal/backgroundImage", path);
}

int QWoSetting::terminalBackgroundImageAlpha()
{
    return QWoSetting::value("terminal/backgroundImageAlpha", 128).toInt();
}

void QWoSetting::setTerminalBackgroundImageAlpha(int v)
{
    QWoSetting::setValue("terminal/backgroundImageAlpha", v);
}

bool QWoSetting::terminalBackgroundImageEdgeSmooth()
{
    return QWoSetting::value("terminal/backgroundImageEdgeSmooth", true).toBool();
}

void QWoSetting::setTerminalBackgroundImageEdgeSmooth(bool v)
{
    QWoSetting::setValue("terminal/backgroundImageEdgeSmooth", v);
}

QString QWoSetting::terminalBackgroundImagePosition()
{
    return QWoSetting::value("terminal/backgroundImagePosition", "000010000").toString();
}

void QWoSetting::setTerminalBackgroundImagePosition(const QString &pos)
{
    QWoSetting::setValue("terminal/backgroundImagePosition", pos);
}
