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
        ETT_Mstsc = 3
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
