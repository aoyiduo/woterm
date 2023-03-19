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
