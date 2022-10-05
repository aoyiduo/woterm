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

#pragma once

#include <QEvent>
#include <QVariant>


class QWoEvent : public QEvent
{
public:
    enum WoEventType{
        PropertyChanged,
    };
public:
    QWoEvent(WoEventType t, const QVariant& data = QVariant());

    WoEventType eventType() const;
    QVariant data() const;

    void setResult(const QVariant& result);
    QVariant result() const;
    bool hasResult();

    static int EventType;
private:
    WoEventType m_type;
    QVariant m_data;
    QVariant m_result;

};
