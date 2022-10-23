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

#include<QSettings>
#include<QApplication>
#include<QDir>
#include<QMessageBox>
#include<QStandardPaths>
#include<QProcess>
#include <QTranslator>

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
            QMessageBox::warning(nullptr, "warning", QString("The language file has no name:%1").arg(path));
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
