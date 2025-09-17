/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWOLINEEDIT_H
#define QWOLINEEDIT_H

#include <QLineEdit>
#include <QPointer>

class QToolButton;

class QWoLineEdit : public QObject
{
public:    
    static QWoLineEdit *decorator(QLineEdit *parent=nullptr);
private:
    explicit QWoLineEdit(QLineEdit *parent=nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
private:
    QPointer<QLineEdit> m_edit;
    QPointer<QToolButton> m_btn;
};

#endif // QWOLINEEDIT_H
