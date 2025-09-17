/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxfileassist.h"

#include <QFile>

QKxFileAssist::QKxFileAssist(QObject *parent)
    : QObject(parent)
{

}

bool QKxFileAssist::rename(const QString &fileSrc, const QString &fileDst)
{
    return QFile::rename(fileSrc, fileDst);
}

bool QKxFileAssist::exist(const QString &path) const
{
    return QFile::exists(path);
}

void QKxFileAssist::close()
{
    if(m_file == nullptr) {
        return;
    }
    m_file->close();
}

bool QKxFileAssist::open(const QString &path, bool readOnly)
{
    if(m_file == nullptr) {
        m_file = new QFile(path, this);
    }else{
        m_file->close();
        m_file->setFileName(path);
    }
    return m_file->open(readOnly ? QFile::ReadOnly : QFile::ReadWrite);
}

QByteArray QKxFileAssist::read(int size)
{
    if(m_file == nullptr) {
        return QByteArray();
    }
    return m_file->read(size);
}

bool QKxFileAssist::write(const QByteArray &buf)
{
    if(m_file == nullptr) {
        return false;
    }
    return m_file->write(buf);
}

bool QKxFileAssist::seek(qint64 pos)
{
    if(m_file == nullptr) {
        return false;
    }
    return m_file->seek(pos);
}

qint64 QKxFileAssist::fileSize() const
{
    if(m_file == nullptr) {
        return false;
    }
    return m_file->size();
}

QKxFileAssist::FileError QKxFileAssist::error() const
{
    if(m_file == nullptr) {
        return NoError;
    }
    return (FileError)m_file->error();
}

QString QKxFileAssist::errorString() const
{
    if(m_file == nullptr) {
        return QString();
    }
    return m_file->errorString();
}
