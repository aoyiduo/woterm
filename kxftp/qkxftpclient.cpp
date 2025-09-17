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

#include "qkxftpclient.h"
#include "qkxftp_private.h"

#include "qkxlengthbodypacket.h"

#include <QLocalSocket>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>


QKxFtpClient::QKxFtpClient(QIODevice *local, QObject *parent)
    : QKxFtpResponse(parent)
    , m_local(local)
    , m_crypt(QCryptographicHash::Md5)
    , m_recvLeft(0)
{
    m_stream.setDevice(local);
    QObject::connect(local, SIGNAL(readyRead()), this, SLOT(onLocalReadyRead()));
    m_recvBuffer.reserve(1024);    
}

QKxFtpClient::~QKxFtpClient()
{
}

void QKxFtpClient::onLocalReadyRead()
{
    QByteArray all = m_local->readAll();
    handlePacket(all);
}

void QKxFtpClient::writeResult(const QByteArray &buf)
{
    m_stream << buf;
}
