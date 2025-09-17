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

#ifndef QKXFTPCLIENT_H
#define QKXFTPCLIENT_H

#include "qkxftpresponse.h"

#include <QObject>
#include <QPointer>
#include <QDataStream>
#include <QCryptographicHash>


class QIODevice;
class QLocalSocket;
class QFile;

class QKxFtpClient : public QKxFtpResponse
{
    Q_OBJECT
public:
    explicit QKxFtpClient(QIODevice *local, QObject *parent = nullptr);
    ~QKxFtpClient();

private slots:
    void onLocalReadyRead();
private:
    virtual void writeResult(const QByteArray& buf);
private:
    QPointer<QIODevice> m_local;
    QDataStream m_stream;
    QPointer<QKxLengthBodyPacket> m_lbp;
    // transfer
    QCryptographicHash m_crypt;
    QPointer<QFile> m_file;
    qint32 m_recvLeft;
    QByteArray m_recvBuffer;
};

#endif // QKXFTPCLIENT_H
