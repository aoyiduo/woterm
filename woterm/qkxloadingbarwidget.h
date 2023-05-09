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

#ifndef QKXLOADINGBARWIDGET_H
#define QKXLOADINGBARWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QScopedPointer>

class QTimer;
class QKxLoadingBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QKxLoadingBarWidget(const QColor& clr, QWidget *parent = nullptr);

    void setSpeed(int fps=30);
    void setThickness(int n);
    void setMessageColor(const QColor& clr);
    void setMessageSize(int sz);
    void setMessage(const QString& msg);
signals:

public slots:
    void onTimeout();
protected:
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);
private:
    QColor m_color;
    qreal m_step;
    qreal m_angle;
    int m_thickness;
    int m_txtSize;
    QString m_message;
    QColor m_msgColor;
    QScopedPointer<QTimer> m_timer;
};

#endif // QKXLOADINGBARWIDGET_H
