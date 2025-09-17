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

#include "qwovncftprequest.h"

#include "qkxvncwidget.h"

#include <QDebug>
#include <QDataStream>

QWoVncFtpRequest::QWoVncFtpRequest(QKxVncWidget *vnc, QObject *parent)
    : QKxFtpRequest(parent)
    , m_vnc(vnc)
{
    QObject::connect(vnc, SIGNAL(ftpArrived(QByteArray)), this, SLOT(onFtpResult(QByteArray)));
}

void QWoVncFtpRequest::writePacket(const QByteArray &data)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << data;
    m_vnc->sendFtpPacket(buf);
}

void QWoVncFtpRequest::onFtpResult(const QByteArray &buf)
{
    handlePacket(buf);
}
