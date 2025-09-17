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

#ifndef QKXBUBBLESYNCWIDGET_H
#define QKXBUBBLESYNCWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QList>

namespace Ui {
class QKxBubbleSyncWidget;
}

class QTimer;

class QKxBubbleSyncWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QKxBubbleSyncWidget();
    ~QKxBubbleSyncWidget();

    void setMessage(const QString& title, const QString& msg, int timeout = 1000);
    Q_INVOKABLE void resetLayout();
private slots:
    void onResetLayout();
    void onTimeout();
private:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    bool isCursorHover();
private:
    Ui::QKxBubbleSyncWidget *ui;
    QPointer<QTimer> m_timer;
    QPoint m_dragPosition;

    struct Task{
        QString title;
        QString content;
        int timeout;
    };
    QList<Task> m_tasks;
};

#endif // QKXBUBBLESYNCWIDGET_H
