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

#ifndef QKXSOCKET_H
#define QKXSOCKET_H

#include <QObject>
#include <QPointer>
#include <QByteArray>

#include "qkxreader.h"


class QTLSAuth : public QObject
{
public:
    explicit QTLSAuth(QObject *parent=nullptr);
    ~QTLSAuth();

    virtual bool handleAnonTLSAuth() = 0;
    virtual bool handleVeNCryptAuth() = 0;
    virtual int readFromTLS(char *out, unsigned int n) = 0;
    virtual int writeToTLS(const char *buf, unsigned int n) = 0;
    virtual void freeTLS() = 0;
};


class QKxTcpSocket : public QKxReader
{
public:
    QKxTcpSocket();
    virtual ~QKxTcpSocket();
    bool connect(const char* host, int port);
    int handle();
    bool makeTLS();

    bool waitRead(char *buf, int len, int timeout = 30000);
    bool waitWrite(char *buf, int len, int timeout = 30000);
    bool waitWrite(const QByteArray& data, int timeout = 30000);

    bool isEmpty();

    //Reader Interface
    QByteArray readArray(int timeout = 30000);
    qint32 readInt32(int timeout = 30000);
    quint32 readUint32(int timeout = 30000);
    qint16 readInt16(int timeout = 30000);
    quint16 readUint16(int timeout = 30000);
    qint8 readInt8(int timeout = 30000);
    quint8 readUint8(int timeout = 30000);
    quint32 readRgb32(int timeout = 30000);
    quint32 readRgb24(int timeout = 30000);
    quint16 readRgb16(int timeout = 30000);
    quint8 readRgb8(int timeout = 30000);
    void readSkip(int len, int timeout = 30000);
    char* current(int len, int timeout = 30000);

protected:
    int _waitRead(char *buf, int len, int timeout = 30000);
    int _waitWrite(char *buf, int len, int timeout = 30000);
    int _waitBuffer(int len, int timeout = 30000);
    int _readAll();

private:
    int m_socket;
    QByteArray m_buf;
    int m_ipos;
    //QPointer<QTLSAuth> m_auth;
};

#endif // QKXSOCKET_H
