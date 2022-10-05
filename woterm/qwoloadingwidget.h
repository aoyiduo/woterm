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

#ifndef QWOLOADINGWIDGET_H
#define QWOLOADINGWIDGET_H

#include <QWidget>
#include <QTimer>

class QWoLoadingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWoLoadingWidget(const QColor& clr, QWidget *parent = 0);
    explicit QWoLoadingWidget(const QString& imagePath, QWidget *parent = 0);
    ~QWoLoadingWidget();

    void setSpeed(int fps=30);
    void setRadius(int n);
protected:
    void paintEvent(QPaintEvent *ev);
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);
private slots:
    void onRotateRequest();
private:
    QColor m_color;
    QPixmap m_pixmap;
    QTimer m_timer;
    qreal m_angle;
    int m_radius;
};

#endif // QWOLOADINGWIDGET_H
