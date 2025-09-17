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

#ifndef QWOVNCFTPREQUEST_H
#define QWOVNCFTPREQUEST_H

#include <QPointer>

#include "qkxftprequest.h"

class QKxVncWidget;
class QWoVncFtpRequest : public QKxFtpRequest
{
    Q_OBJECT
public:
    explicit QWoVncFtpRequest(QKxVncWidget *vnc, QObject *parent = nullptr);
protected:
    virtual void writePacket(const QByteArray& data);
protected slots:
   void onFtpResult(const QByteArray& buf);
private:
    QPointer<QKxVncWidget> m_vnc;
};

#endif // QWOVNCFTPREQUEST_H
