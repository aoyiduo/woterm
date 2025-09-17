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

#ifndef QKXFTPSERVER_H
#define QKXFTPSERVER_H

#include "qkxftp_share.h"

#include <QPointer>
#include <QDataStream>
#include <QCryptographicHash>
#include <QMap>

class QRemoteObjectHost;
class QTcpServer;
class QLocalSocket;
class QTcpSocket;
class QKxFtpIconProvider;
class QThread;
class QFile;
class QKxFtpClient;
class QIODevice;
class QLocalServer;
class QKxFtpServerPrivate;

class KXFTP_EXPORT QKxFtpServer : public QObject
{
    Q_OBJECT
public:
    explicit QKxFtpServer(const QString& envName);
    ~QKxFtpServer();
private:
    QPointer<QKxFtpServerPrivate> m_prv;
};

#endif // QKXFTPSERVER_H
