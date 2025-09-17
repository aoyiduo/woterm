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

#ifndef QMOTERMWIDGETIMPL_H
#define QMOTERMWIDGETIMPL_H

#include "qmoshowwidget.h"
#include "qmomenu.h"

class QMoTermWidget;
class QVBoxLayout;
class QMoKeyboard;

class QMoTermWidgetImpl : public QMoShowWidget
{
    Q_OBJECT
public:
    explicit QMoTermWidgetImpl(const QString& target, QWidget *parent = nullptr);
    virtual ~QMoTermWidgetImpl();
    void showKeyboard(bool on);
private:
    Q_INVOKABLE void init();
    Q_INVOKABLE void showMenu();
private slots:
    void onShowMenu();
    void onClickedArrived(int mid);
    void onSimulateKeyEvent(QKeyEvent *ev);
protected:
    virtual QMoTermWidget *createTermWidget(const QString& target, QWidget *parent) = 0;

protected:
    QPointer<QMoTermWidget> m_term;
    QPointer<QMoKeyboard> m_keyboard;
    QPointer<QMoMenu> m_menu;
};

#endif // QMOTERMWIDGETIMPL_H
