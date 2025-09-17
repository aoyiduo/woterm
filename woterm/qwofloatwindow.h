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

#ifndef QWOFLOATWINDOW_H
#define QWOFLOATWINDOW_H

#include "qwoglobal.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPointer>

class QWoShower;
class QWoShowerWidget;
class QPropertyAnimation;
class QWidget;

class QWoFloatWindow : public QWidget
{
    Q_OBJECT
public:
    enum EToolType {
        ETT_Term = 1,
        ETT_Vnc = 2,
        ETT_Mstsc = 3,
        ETT_Common = 4
    };
public:
    explicit QWoFloatWindow(QWoShowerWidget *child, QWoShower *shower, EToolType type);
    void restoreToShower();
signals:

public slots:
    void onToolEnter();
    void onToolLeave();
    void onDeleteLater();
private:
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent* ev);
    void updateToolbarPosition();
private:
    EToolType m_type;
    QPointer<QVBoxLayout> m_layout;
    QPointer<QWoShower> m_shower;
    QPointer<QWoShowerWidget> m_child;

    QPointer<QWidget> m_tool;
    QPointer<QPropertyAnimation> m_ani;
};

Q_DECLARE_METATYPE(QWoFloatWindow::EToolType)

#endif // QWOFLOATWINDOW_H
