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
