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

#ifndef QWOIDENTIFYINFOMATION_H
#define QWOIDENTIFYINFOMATION_H

#include <QObject>
#include <QPointer>
#include <QMap>

#include "qwoglobal.h"

class QWoIdentify : public QObject
{
public:
    static bool infomationByPrivateKey(const QString&file, IdentifyInfo *pinfo);
    static bool infomationByPrivateKey(const QByteArray& rsa, IdentifyInfo *pinfo);
    static bool isPublicKey(const QString &fileName);
    static bool isPrivateKey(const QString &fileName);
    static bool import(const QString& file, IdentifyInfo *pinfo);
    static bool create(const QString& file);
    static bool create(const QString& name, const QByteArray& prvKey);
    static bool rename(const QString& nameOld, const QString& nameNew);
    static bool remove(const QString& name);
    static QMap<QString, IdentifyInfo> loadFromSqlite();
    static QMap<QString, IdentifyInfo> loadFromFile();
};

#endif // QWOIDENTIFYINFOMATION_H
