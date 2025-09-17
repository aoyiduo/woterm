/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QMOQUICKASSIST_H
#define QMOQUICKASSIST_H

#include <QPointer>
#include <QObject>
#include <QVariant>
#include <QColor>

class QQuickWidget;

class QMoQuickAssist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int width READ width NOTIFY sizeChanged)
    Q_PROPERTY(int height READ height NOTIFY sizeChanged)
    Q_PROPERTY(QColor themeColor READ themeColor WRITE setThemeColor NOTIFY themeColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor seperatorColor READ seperatorColor WRITE setSeperatorColor NOTIFY seperatorColorChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)
    Q_PROPERTY(QColor alphaLv1Color READ alphaLv1Color)
    Q_PROPERTY(QColor alphaLv2Color READ alphaLv2Color)
    Q_PROPERTY(QColor alphaLv3Color READ alphaLv3Color)
    Q_PROPERTY(QColor alphaLv4Color READ alphaLv4Color)
    Q_PROPERTY(QColor alphaLv5Color READ alphaLv5Color)
    Q_PROPERTY(bool debugVersion READ debugVersion)
public:
    explicit QMoQuickAssist(QQuickWidget *parent = nullptr);

    int width() const;
    int height() const;

    QColor themeColor() const;
    void setThemeColor(const QColor& clr);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& clr);

    QColor seperatorColor() const;
    void setSeperatorColor(const QColor& clr);

    QColor highlightColor() const;
    void setHighlightColor(const QColor& clr);

    QColor alphaLv1Color() const;
    QColor alphaLv2Color() const;
    QColor alphaLv3Color() const;
    QColor alphaLv4Color() const;
    QColor alphaLv5Color() const;
    QColor alphaLv6Color() const;

    bool debugVersion() const;

    Q_INVOKABLE void openUrl(const QString& url);

signals:
    void sizeChanged();
    void themeColorChanged();
    void backgroundColorChanged();
    void seperatorColorChanged();
    void highlightColorChanged();
private:
    virtual bool eventFilter(QObject *watched, QEvent *event);
private:
    QPointer<QQuickWidget> m_quick;
    QColor m_themeColor;
    QColor m_backgroundColor;
    QColor m_seperatorColor;
    QColor m_highlightColor;
};

#endif // QMOQUICKASSIST_H
