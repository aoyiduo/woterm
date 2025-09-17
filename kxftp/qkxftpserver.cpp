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

#include "qkxftpserver.h"
#include "qkxftpclient.h"

#include "qkxftp_private.h"
#include "qkxftpserverprivate.h"

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QLocalServer>

QKxFtpServer::QKxFtpServer(const QString& envName)
    : QObject(nullptr)
{
    m_prv = new QKxFtpServerPrivate(envName);
}

QKxFtpServer::~QKxFtpServer()
{
    delete m_prv;
}
