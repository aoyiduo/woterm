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

#pragma once

#include "qwotermwidgetimpl.h"

#include <QPointer>

class QSplitter;
class QWoRdpWidget;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QWoRdpWidget;

class QWoRdpWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoRdpWidgetImpl(const QString& target, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoRdpWidgetImpl();
protected:
    virtual bool handleTabMouseEvent(QMouseEvent *ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
private slots:
    void onRootDestroy();
private:
    QPointer<QWoRdpWidget> m_rdp;
};
