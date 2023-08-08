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
private:
    QPointer<QWoVncPlayWidget> m_vnc;
};
