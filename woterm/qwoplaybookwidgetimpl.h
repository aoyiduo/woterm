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

#include "qwoshowerwidget.h"

#include <QPointer>

class QSplitter;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QWoPlaybookWidget;

class QWoPlaybookWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoPlaybookWidgetImpl(const QString& path, const QString& name, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoPlaybookWidgetImpl();
protected:
    virtual bool handleTabMouseEvent(QMouseEvent *ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual bool handleCustomProperties();
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
    virtual bool isRemoteSession();
private slots:
    void onRootDestroy();
    void onEnterScriptDebugTriggered();
    void onLeaveScriptDebugTriggered();
    void onOpenConsoleWindow();
    void onCloseConsoleWindow();
private:
    QPointer<QWoPlaybookWidget> m_playbook;
};
