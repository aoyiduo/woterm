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

#ifndef QKXMACTIMER_H
#define QKXMACTIMER_H

#include "qkxutil_share.h"

#include <QObject>

#ifdef __OBJC__
@class NSTimer;
#else
using NSTimer = void;
#endif

/**
 * Class similar to a QTimer but allows setting a tolerance, which
 * makes timers more battery-friendly on OSX.
 */
class KXUTIL_EXPORT QKxMacTimer : public QObject
{
    Q_OBJECT

public:
    explicit QKxMacTimer(QObject *parent = 0);
    virtual ~QKxMacTimer();

    void setInterval(int msec);
    int interval() const { return m_interval; }
    inline void setSingleShot(bool singleShot);

    /**
     * Set the tolerance for the timer. See NSTimer::setTolerance.
     *
     * Tolerance is ignored on OS X < 10.9.
     */
    void setTolerance(int msec);
    int tolerance() const { return m_tolerance; }

public Q_SLOTS:
    void start();
    void stop();
    inline bool isSingleShot() const { return m_singleShot; }

Q_SIGNALS:
    void timeout();

private:
    Q_DISABLE_COPY(QKxMacTimer)

    void restart();

    int m_interval;
    int m_tolerance;
    bool m_singleShot;

    NSTimer *m_nsTimer;
};

#endif // QKXMACTIMER_H
