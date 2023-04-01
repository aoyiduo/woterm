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

#ifndef QMOVNCWIDGETIMPL_H
#define QMOVNCWIDGETIMPL_H

#include "qmoshowwidget.h"

class QMoVncWidget;
class QMoMenu;
class QScrollArea;
class QKxTouchPoint;
class QMoKeyboard;

class QMoVncWidgetImpl : public QMoShowWidget
{
    Q_OBJECT
public:
    explicit QMoVncWidgetImpl(const QString &target, QWidget *parent = nullptr);
    virtual ~QMoVncWidgetImpl();
signals:

private:
    virtual void resizeEvent(QResizeEvent* ev);
private slots:
    void onForceToClose();
    void onTouchPointClicked();
    void onMenuItemClickedArrived(int id);
    void onKeyboardKeyEvent(QKeyEvent* ev);
private:
    Q_INVOKABLE void showTouchPoint(bool show, bool async);
    Q_INVOKABLE void resetTouchPointPosition();
private:
    QPointer<QKxTouchPoint> m_touchPoint;
    QPointer<QMoVncWidget> m_player;
    QPointer<QMoMenu> m_menu;
    QPointer<QScrollArea> m_area;
    QPointer<QMoKeyboard> m_keyboard;
};

#endif // QMOVNCWIDGETIMPL_H
