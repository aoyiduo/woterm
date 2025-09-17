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

#ifndef QMOREDPOINT_H
#define QMOREDPOINT_H

#include <QWidget>

class QMoRedPoint : public QWidget
{
    Q_OBJECT
public:
    explicit QMoRedPoint(const QSize& size, QWidget *parent = nullptr);

signals:

private:
    virtual void paintEvent(QPaintEvent *event);
private:
    QSize m_size;
};

#endif // QMOREDPOINT_H
