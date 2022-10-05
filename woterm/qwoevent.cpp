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

#include "qwoevent.h"

#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QMenu>
#include <QClipboard>

int QWoEvent::EventType = QEvent::registerEventType();

QWoEvent::QWoEvent(WoEventType t, const QVariant& data)
 : QEvent(QEvent::Type(EventType))
 , m_type(t)
 , m_data(data)
{
    setAccepted(false);
}

QWoEvent::WoEventType QWoEvent::eventType() const
{
    return m_type;
}

QVariant QWoEvent::data() const
{
    return m_data;
}

void QWoEvent::setResult(const QVariant &result)
{
    m_result = result;
}

QVariant QWoEvent::result() const
{
    return m_result;
}

bool QWoEvent::hasResult()
{
    return m_result.isValid();
}
