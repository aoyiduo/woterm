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

#include "qkxkeepwakeup.h"

#include <QDebug>
#include <QDateTime>
#include <QTimer>

#ifdef Q_OS_MAC
#include <dispatch/dispatch.h>
#include <IOKit/IOKitLib.h>
#include <IOSurface/IOSurface.h>
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

// Determines the minimum amount of time between attempts to waken the display
// in response to an input event.
// https://source.chromium.org/chromium/chromium/src/+/main:remoting/host/input_injector_mac.cc
const int kWakeUpDisplayIntervalMs = 1000;
QKxKeepWakeUp::QKxKeepWakeUp(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onAliveTimeout()));
    m_timer->start(kWakeUpDisplayIntervalMs);
    m_tmLast = QDateTime::currentSecsSinceEpoch();
}

void QKxKeepWakeUp::onAliveTimeout()
{
#ifdef Q_OS_MAC
    IOPMAssertionID power_assertion_id = kIOPMNullAssertionID;
    IOReturn result = IOPMAssertionCreateWithName(
          CFSTR("UserIsActive"),
          kIOPMAssertionLevelOn,
          CFSTR("FeiDesk Remote Desktop connection active"),
          &power_assertion_id);
    if (result == kIOReturnSuccess) {
        IOPMAssertionRelease(power_assertion_id);
    }
#else
    m_timer->stop();
#endif

    qint64 now = QDateTime::currentSecsSinceEpoch();
    if(now - m_tmLast > 60) {
        m_tmLast = now;
        qDebug() << "QKxKeepWakeUp::onKeepAliveTimeout" << QDateTime::currentSecsSinceEpoch();
    }
}
