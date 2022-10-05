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

#include "qkxcolorschema.h"

#include <QFile>

QKxColorSchema::QKxColorSchema(QObject *parent)
    : QObject(parent)
{

}

QKxColorSchema::~QKxColorSchema()
{

}

bool QKxColorSchema::load(const QString &path)
{
    QFile f(path);
    if(!f.open(QFile::ReadOnly|QFile::Text)) {
        return false;
    }
    while(!f.atEnd()){
        QString line = f.readLine().simplified();
        if(line.startsWith("ColorForeground=")) {
            QString tmp = line.mid(16);
            m_foreground = QColor(tmp);
        }else if(line.startsWith("ColorBackground=")) {
            QString tmp = line.mid(16);
            m_background = QColor(tmp);
        }else if(line.startsWith("ColorCursor=")) {
            QString tmp = line.mid(12);
            m_cursor = QColor(tmp);
        }else if(line.startsWith("ColorPalette=")) {
            QString tmp = line.mid(13);
            QStringList clrs = tmp.split(';');
            for(int i = 0; i < clrs.length(); i++) {
                m_indexs.append(clrs[i]);
            }
        }
    }
    m_pretty.clear();
    return !m_indexs.isEmpty();
}

QColor QKxColorSchema::cursor() const
{
    return m_cursor;
}

QColor QKxColorSchema::background() const
{
    return m_background;
}

QColor QKxColorSchema::foreground() const
{
    return m_foreground;
}

QColor QKxColorSchema::bg(const TermColor &clr) const
{
    if(clr.isDefault()) {
        return m_background;
    }
    if(clr.isIndex()) {
        int idx = qBound(0, int(clr.color), m_indexs.length()-1);
        return m_indexs.at(idx);
    }
    return QColor(QRgb(clr.color));
}

QColor QKxColorSchema::fg(const TermColor &clr) const
{
    if(clr.isDefault()) {
        return m_foreground;
    }
    if(clr.isIndex()) {
        int idx = qBound(0, int(clr.color), m_indexs.length() - 1);
        return m_indexs.at(idx);
    }
    return QColor(QRgb(clr.color));
}

QColor QKxColorSchema::index(int i) const
{
    return m_indexs.value(i);
}

QColor QKxColorSchema::pretty(const QColor &clr)
{
    QColor fg = m_pretty.value(clr.rgba());
    if(fg.isValid()) {
        return fg;
    }
    int r, g, b;
    clr.getRgb(&r, &g, &b);
    int gray = (r*299 + g*587 + b*114 + 500) / 1000;
    if(255 - gray < gray) {
        fg = QColor(0, 0, 0);
    }else{
        fg = QColor(255, 255, 255);
    }
    m_pretty.insert(clr.rgba(), fg);
    return fg;
}
