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

#include "qwosetting.h"
#include "qkxmessagebox.h"

#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QTranslator>

#include "qwoutils.h"

#define WOTERM_DATA_PATH ("WOTERM_DATA_PATH")

QSettings *qSettings()
{
    static QString path = QDir::cleanPath(QWoSetting::applicationDataPath()+"/woterm.ini");
    static QSettings setting(path, QSettings::IniFormat);
    return &setting;
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

QString QWoSetting::privateDataPath()
{
    static QString path = QDir::cleanPath(QApplication::applicationDirPath()+"/../private/");
    return path;
}

QString QWoSetting::privateJsCorePath()
{
    static QString path = QDir::cleanPath(privateDataPath() + "/jscore/");
    return path;
}

QString QWoSetting::privateColorSchemaPath()
{
    static QString path = QDir::cleanPath(privateDataPath() + "/color-schemes/");
    return path;
}

QString QWoSetting::privateKeyboardLayoutPath()
{
    static QString path = QDir::cleanPath(privateDataPath() + "/kb-layouts/");
    return path;
}

QString QWoSetting::privateTranslationPath()
{
    static QString path = QDir::cleanPath(privateDataPath() + "/translations/");
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

QString QWoSetting::fontBackupPath()
{
    QString path = QWoSetting::ensurePath("font");
    path = QDir::cleanPath(path);
    path = QDir::toNativeSeparators(path);
    return path;
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

QString QWoSetting::languageFile()
{
    QString name = QWoSetting::value("language/path", "").toString();
    if(name.isEmpty()) {
        QLocale local;
        QStringList langs = local.uiLanguages();
        if(!langs.isEmpty()) {
            QString lang = langs.at(0);
            lang = lang.split('-').at(0);
            QString path = QString(":/woterm/language/woterm_%1.qm").arg(lang.toLower());
            QMap<QString,QString> langs = allLanguages();
            if(langs.values().contains(path)) {
                return path;
            }
        }
        return QString(":/woterm/language/woterm_en.qm");;
    }
    return name;
}

QMap<QString, QString> QWoSetting::allLanguages()
{
    QMap<QString, QString> langs;
    QDir dir(":/woterm/language");
    QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
    for(int i = 0;i < fileNames.length(); i++) {
        QString path = dir.filePath(fileNames.at(i));
        QString type = QWoSetting::languageName(path);
        if(!type.isEmpty()){
            langs.insert(type, path);
        }else{
            QKxMessageBox::warning(nullptr, "warning", QString("The language file has no name:%1").arg(path));
        }
    }
    return langs;
}

QString QWoSetting::languageName(const QString &path)
{
    QTranslator translator;
    translator.load(path);
    return translator.translate("English", "English");
}

void QWoSetting::setLanguageFile(const QString &lang)
{
    QWoSetting::setValue("language/path", lang);
}

QString QWoSetting::lastBackupPath()
{
    return value("backup/path", "").toString();
}

void QWoSetting::setLastBackupPath(const QString &path)
{
    setValue("backup/path", path);
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

bool QWoSetting::startupByAdmin()
{
    return value("admin/startup", false).toBool();
}

void QWoSetting::setStartupByAdmin(bool on)
{
    setValue("admin/startup", on);
}

bool QWoSetting::lookupPasswordByAdmin()
{
    return value("admin/lookupPassword", false).toBool();
}

void QWoSetting::setLookupPasswordByAdmin(bool on)
{
    setValue("admin/lookupPassword", on);
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
