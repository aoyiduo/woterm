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

#pragma once

#include "qkxutil_share.h"

#include <QVariant>
#include <QMap>

class KXUTIL_EXPORT QKxSetting
{
public:
    static void setValue(const QString& key, const QVariant& v);
    static QVariant value(const QString& key, const QVariant& defval=QVariant());
    static void remove(const QString& key);
    static void sync();

    // application
    static QString applicationName();
    static QString applicationDirPath();
    static QString applicationFilePath();

    static void updateStartTime();

    static QByteArray uuid();

    static QString ensurePath(const QString& name);
    static QString applicationConfigPath();
    static QString applicationDataPath();
    static QString logFilePath();
protected:
    static QString specialFilePath(const QString& name);
};
