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
