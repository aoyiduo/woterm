/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qmoquickassist.h"
#include "version.h"

#include <QQuickWidget>
#include <QDesktopServices>

QMoQuickAssist::QMoQuickAssist(QQuickWidget *parent)
    : QObject(parent)
    , m_quick(parent)
{
    parent->installEventFilter(this);
    m_themeColor = QColor(0xD1, 0xEE, 0xFF); //"#D1EEFF"
    m_backgroundColor = QColor(151, 222, 255);
    m_seperatorColor = alphaLv6Color();
    m_highlightColor = alphaLv4Color();
}

int QMoQuickAssist::width() const
{
    return m_quick->width();
}

int QMoQuickAssist::height() const
{
    return m_quick->height();
}

QColor QMoQuickAssist::themeColor() const
{
    return m_themeColor;
}

void QMoQuickAssist::setThemeColor(const QColor &clr)
{
    if(m_themeColor != clr) {
        m_themeColor = clr;
        emit themeColorChanged();
    }
}

QColor QMoQuickAssist::backgroundColor() const
{
    return m_backgroundColor;
}

void QMoQuickAssist::setBackgroundColor(const QColor &clr)
{
    if(m_backgroundColor == clr) {
        m_backgroundColor = clr;
        emit backgroundColorChanged();
    }
}

QColor QMoQuickAssist::seperatorColor() const
{
    return m_seperatorColor;
}

void QMoQuickAssist::setSeperatorColor(const QColor &clr)
{
    if(m_seperatorColor == clr) {
        m_seperatorColor = clr;
        emit seperatorColorChanged();
    }
}

QColor QMoQuickAssist::highlightColor() const
{
    return m_highlightColor;
}

void QMoQuickAssist::setHighlightColor(const QColor &clr)
{
    if(m_highlightColor != clr) {
        m_highlightColor = clr;
        emit highlightColorChanged();
    }
}

QColor QMoQuickAssist::alphaLv1Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0x1F);//"#1FFFFFFF";
}

QColor QMoQuickAssist::alphaLv2Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0x3F);
}

QColor QMoQuickAssist::alphaLv3Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0x5F);
}

QColor QMoQuickAssist::alphaLv4Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0x7F);
}

QColor QMoQuickAssist::alphaLv5Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0x9F);
}

QColor QMoQuickAssist::alphaLv6Color() const
{
    return QColor(0xFF, 0xFF, 0xFF, 0xAF);
}

bool QMoQuickAssist::debugVersion() const
{
#ifdef QT_DEBUG
    return true;
#else
    return false;
#endif
}

void QMoQuickAssist::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

bool QMoQuickAssist::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == m_quick) {
        QEvent::Type type = ev->type();
        if(type == QEvent::Resize) {
            emit sizeChanged();
        }
    }

    return QObject::eventFilter(obj, ev);
}

