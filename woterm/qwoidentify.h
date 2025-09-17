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

#ifndef QWOIDENTIFYINFOMATION_H
#define QWOIDENTIFYINFOMATION_H

#include <QObject>
#include <QPointer>
#include <QMap>

#include "qwoglobal.h"

class QWoIdentify : public QObject
{
    Q_OBJECT
public:
    explicit QWoIdentify(QObject *parent = nullptr);
    static bool infomationByPrivateKey(const QString&file, IdentifyInfo *pinfo);
    static bool infomationByPrivateKey(const QByteArray& rsa, IdentifyInfo *pinfo);
    static bool isPublicKey(const QString &fileName);
    static bool isPrivateKey(const QString &fileName);
    static bool import(const QString& file, IdentifyInfo *pinfo);
    static bool create(const QString& file);
    static bool create(const QString& name, const QByteArray& prvKey);
    static bool infomation(const QByteArray& name, IdentifyInfo *pinfo);
    static bool rename(const QString& nameOld, const QString& nameNew);
    Q_INVOKABLE static bool remove(const QString& name);
    static QMap<QString, IdentifyInfo> loadFromSqlite();
    static QMap<QString, IdentifyInfo> loadFromFile();    
    Q_INVOKABLE static QStringList qmlFileNames();
    Q_INVOKABLE static QVariantList qmlLoadFromSqlite();
    Q_INVOKABLE static QVariant qmlImport(const QString& file, const QString& nameSave);
};

#endif // QWOIDENTIFYINFOMATION_H
