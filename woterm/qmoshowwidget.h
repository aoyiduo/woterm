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

#ifndef QMOSHOWWIDGET_H
#define QMOSHOWWIDGET_H

#include <QWidget>
#include <QPointer>

class QMoShowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMoShowWidget(const QString& target, QWidget *parent = nullptr);
signals:

private slots:
    void onResizeAgaint();
protected:
    QString m_target;
};

#endif // QMOSHOWWIDGET_H
