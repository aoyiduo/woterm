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

#ifndef QKXKEEPWAKEUP_H
#define QKXKEEPWAKEUP_H

#include "qkxutil_share.h"

#include <QObject>
#include <QPointer>

class QTimer;
class KXUTIL_EXPORT QKxKeepWakeUp : public QObject
{
    Q_OBJECT
public:
    explicit QKxKeepWakeUp(QObject *parent = nullptr);

private slots:
    void onAliveTimeout();
private:
    QPointer<QTimer> m_timer;
    qint64 m_tmLast;
};

#endif // QKXKEEPWAKEUP_H
