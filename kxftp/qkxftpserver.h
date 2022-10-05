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
