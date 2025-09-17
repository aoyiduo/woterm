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

#ifndef QKXLABELASSIST_H
#define QKXLABELASSIST_H

#include <QPointer>
#include <QLabel>

class QWidget;
class QKxLabelAssist : public QObject
{
    Q_OBJECT
public:
    explicit QKxLabelAssist(QWidget *parent = nullptr);
    void setTextAlignment(Qt::Alignment align);
    void setText(const QString& txt);

    void setVisible(bool on);
    bool isVisible();
private:
    bool eventFilter(QObject *watched, QEvent *event);
    Q_INVOKABLE void resetPosition();
private:
    QPointer<QLabel> m_label;
};

#endif // QKXLABELASSIST_H
