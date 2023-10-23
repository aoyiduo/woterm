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
