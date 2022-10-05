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

#include "qkxsharememory.h"

#include <QDataStream>
#include <QDebug>

class MemoryLocker
{
    QSharedMemory * m_shared;
public:
    MemoryLocker(QSharedMemory *shared)
        : m_shared(shared) {
        m_shared->lock();
    }
    ~MemoryLocker() {
        m_shared->unlock();
    }
};

QKxShareMemory::QKxShareMemory(const QString &name, int maxLength, QObject *parent)
    : QObject(parent)
    , m_share(name, parent)
{
    if(!m_share.create(maxLength)) {
        if(!m_share.attach(QSharedMemory::ReadWrite)) {
            qWarning() << "QKxShareMemory::construct error" << m_share.errorString();
        }
    }
}

QKxShareMemory::~QKxShareMemory()
{
    m_share.detach();
}

QString QKxShareMemory::name() const
{
    return m_share.key();
}

void QKxShareMemory::setValue(const QString &key, const QVariant &val)
{
    QMap<QString, QVariant> kvs = load();
    kvs.insert(key, val);
    save(kvs);
}

QVariant QKxShareMemory::value(const QString& key, const QVariant &defval) const
{
    QMap<QString, QVariant> kvs = load();
    return kvs.value(key, defval);
}

QMap<QString, QVariant> QKxShareMemory::all() const
{
    QMap<QString, QVariant> kvs = load();
    return kvs;
}

void QKxShareMemory::reset(const QMap<QString, QVariant> &kvs)
{
    save(kvs);
}

QMap<QString, QVariant> QKxShareMemory::load() const
{
    MemoryLocker lk((QSharedMemory*)&m_share);
    QMap<QString, QVariant> kvs;
    QByteArray buf = QByteArray::fromRawData((const char*)m_share.data(), m_share.size());
    QDataStream ds(buf);
    kvs.clear();
    ds >> kvs;
    return kvs;
}

bool QKxShareMemory::save(const QMap<QString, QVariant>& kvs)
{
    MemoryLocker lk(&m_share);

    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << kvs;
    if(buf.length() > m_share.size()) {
        return false;
    }
    memcpy(m_share.data(), buf.data(), buf.length());
    return true;
}
