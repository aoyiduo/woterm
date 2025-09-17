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

#ifndef QKXPLAINTEXTEDIT_H
#define QKXPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class QMouseEvent;

class QKxPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit QKxPlainTextEdit(QWidget *parent=nullptr);
    virtual ~QKxPlainTextEdit();

signals:
    void linkActivated(QString);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual bool event(QEvent *e);
private:
    QString m_clickedAnchor;
};

#endif // QKXPLAINTEXTEDIT_H
