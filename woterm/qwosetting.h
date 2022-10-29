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

#ifndef QWOSETTING_H
#define QWOSETTING_H

#include "qkxsetting.h"

#include <QVariant>
#include <QDate>

class QWoSetting : public QKxSetting
{
public:
    static QString ftpTaskPath();
    static QString identityFilePath();
    static QString historyFilePath();
    static QString examplePath();
    static QString privateDataPath();
    static QString privateJsCorePath();
    static QString privateColorSchemaPath();
    static QString privateKeyboardLayoutPath();
    static QString privateTranslationPath();
    static QString sshServerFilePath();
    static QString sshServerDbPath();
    static QString lastJsLoadPath();
    static void setLastJsLoadPath(const QString& path);
    static QString languageFile();
    static QMap<QString, QString> allLanguages();
    static QString languageName(const QString& path);
    static void setLanguageFile(const QString& path);

    static QString lastBackupPath();
    static void setLastBackupPath(const QString& path);

    /* upgrade */
    static bool shouldPopupUpgradeMessage(const QString& ver);
    static void setNextUpgradeDate(const QString& ver, const QDate &dt);
    static void setIgnoreTodayUpgrade(const QString& ver);
    static void setSkipThisVersion(const QString& ver);
};

#endif
