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
