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

#include "qkxmactimer.h"

#include <Cocoa/Cocoa.h>
#include <QPointer>

QKxMacTimer::QKxMacTimer(QObject *parent) :
    QObject(parent),
    m_interval(0),
    m_tolerance(0),
    m_singleShot(false),
    m_nsTimer(0)
{
}

QKxMacTimer::~QKxMacTimer()
{
    stop();
}

void QKxMacTimer::stop()
{
    if (m_nsTimer) {
        CFRunLoopTimerInvalidate((CFRunLoopTimerRef) m_nsTimer);
    }
    m_nsTimer = 0;
}

void QKxMacTimer::restart()
{
    if (m_nsTimer) {
        stop();
        start();
    }
}

void QKxMacTimer::setInterval(int msec)
{
    if (msec != m_interval) {
        m_interval = msec;
        restart();
    }
}

void QKxMacTimer::setTolerance(int msec)
{
    if (msec != m_tolerance) {
        m_tolerance = msec;
        restart();
    }
}

void QKxMacTimer::setSingleShot(bool singleShot)
{
    if (singleShot != m_singleShot) {
        m_singleShot = singleShot;
        restart();
    }
}

void QKxMacTimer::start() {
    stop();

    // Use QPointer to make it auto-null, and use a 'weak reference' in the
    // block
    QPointer<QKxMacTimer> weakThis(this);
    double intervalSeconds = (float)m_interval / 1000.0;

    // Create a timer which kicks into a block
    CFRunLoopTimerRef timer = CFRunLoopTimerCreateWithHandler(NULL,
        CFAbsoluteTimeGetCurrent() + intervalSeconds,
        m_singleShot ? 0 : intervalSeconds,
        0, 0, // These are both ignored
        ^(CFRunLoopTimerRef) {
            if (weakThis) {
                emit weakThis->timeout();
            }
        }
    );

    // Set the tolerance using an NSTimer as there is no CF way of doing so
    if ([m_nsTimer respondsToSelector:@selector(setTolerance:)]) {
        double toleranceSeconds = (double)m_tolerance / 1000.0;
        [m_nsTimer setTolerance:toleranceSeconds];
    }

    // add timer to the hidden run loop
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
}
