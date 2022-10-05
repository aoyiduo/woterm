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

#ifndef QCOLORSCHEMA_H
#define QCOLORSCHEMA_H

#include "qvtedef.h"

#include <QObject>
#include <QColor>
#include <QMap>

class QKxColorSchema : public QObject
{
public:
    explicit QKxColorSchema(QObject *parent = nullptr);
    virtual ~QKxColorSchema();
    bool load(const QString& path);
    QColor cursor() const;
    QColor background() const;
    QColor foreground() const;
    QColor bg(const TermColor& clr) const;
    QColor fg(const TermColor& clr) const;
    QColor index(int i) const;
    QColor pretty(const QColor& clr);
private:
    QColor m_foreground;
    QColor m_background;
    QColor m_cursor;
    QVector<QColor> m_indexs;
    QMap<QRgb, QColor> m_pretty;
};

#endif // QTHEME_H
