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

#ifndef QKXSHAREMEMORY_H
#define QKXSHAREMEMORY_H

#include "qkxutil_share.h"

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QSharedMemory>

class QThread;
class KXUTIL_EXPORT QKxShareMemory : public QObject
{
    Q_OBJECT
public:
    explicit QKxShareMemory(const QString& name, int maxLength=1024, QObject *parent = nullptr);
    virtual ~QKxShareMemory();
    QString name() const;
    void setValue(const QString& key, const QVariant& val);
    QVariant value(const QString& key, const QVariant& defval) const;
    QMap<QString, QVariant> all() const;
    void reset(const QMap<QString, QVariant>& kvs);
private:
    QMap<QString, QVariant> load() const;
    bool save(const QMap<QString, QVariant>& kvs);
private:
    QSharedMemory m_share;
};

#endif // QKXSHAREMEMORY_H
