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

#include "qwoglobal.h"

#include <QMainWindow>
#include <QPointer>
#include <QDockWidget>

class QMenuBar;
class QVBoxLayout;
class QTabBar;
class QToolBar;
class QWoShower;
class QWoSessionList;
class QWoCommandHistoryForm;

namespace Ui {
class QWoMainWindow;
}


class QWoMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit QWoMainWindow(QWidget *parent=nullptr);
    virtual ~QWoMainWindow();
    static QWoMainWindow *instance();
    static QWoShower *shower();

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onNewTerm();
    void onOpenTerm();
    void onLayout();
    void onEditConfig();    
    void onSessionReadyToConnect(const QString& target, int type);
    void onAppStart();
    void onVersionCheck(int code, const QByteArray& body);
    void onShouldAppExit();

private slots:
    void onActionNewTriggered();
    void onActionOpenTriggered();
    void onActionDisconnectTriggered();
    void onActionReconnectTriggered();
    void onActionReconnectAllTriggered();
    void onActionImportTriggered();
    void onActionExportTriggered();
    void onActionSaveTriggered();
    void onActionTransferTriggered();
    void onActionLogTriggered();
    void onActionExitTriggered();
    void onActionProxyTriggered();
    void onActionConfigDefaultTriggered();
    void onActionSettingTriggered();
    void onActionFindTriggered();
    void onActionAboutTriggered();
    void onActionHelpTriggered();
    void onActionScriptRunTriggered();
    void onActionSshKeyManageTriggered();

private:
    void initMenuBar();
    void initToolBar();
    void initStatusBar();
    void restoreLastState();
    void saveLastState();

private:
    Ui::QWoMainWindow *ui;
    QPointer<QTabBar> m_tab;
    QPointer<QWoSessionList> m_sessions;
    QPointer<QDockWidget> m_sessionDock;
    QPointer<QWoShower> m_shower;
};
