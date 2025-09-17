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

#pragma once

#include "qwotermwidgetimpl.h"

#include <QPointer>

class QSplitter;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QWoVncPlayWidget;

class QWoVncWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoVncWidgetImpl(const QString& target, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoVncWidgetImpl();
protected:
    virtual bool handleTabMouseEvent(QMouseEvent *ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
    virtual ESessionState sessionState();
    virtual void stopSession();
    virtual void reconnectSession(bool all);
private slots:
    void onRootDestroy();
    void onSmartResize();
private:
    QPointer<QWoVncPlayWidget> m_vnc;
};
