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
class QToolButton;
class QWoRecentHistory;
class QPushButton;
class QWoTunnelDialog;
class QWoShowerWidget;
class QWoTermWidget;

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

    void updateHistory(const QString& target, int type);

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onNewSession();
    void onJavascriptRunner();
    void onOpenRemoteSession();
    void onOpenLocalSession();
    void onOpenSerialPort();
    void onLayout();
    void onEditConfig();    
    void onSessionReadyToConnect(const QString& target, int type);
    void onAppStart();
    void onShouldAppExit();
    void onButtonAssistClicked(QToolButton *btn);

    void onRecentMenuAboutToShow();

    void onReportResult(int code, const QByteArray& body);

    void onTabMergeButtonClicked();
    void onTabSeperateButtonClicked();    
    void onTabCurrentChanged(int idx);

    void onPlaybookButtonClicked();
    void onPlaybookAssistButtonClicked(QToolButton* btn);

    void onTunnelButtonClicked();
    void onTunnelAssistButtonClicked(QToolButton* btn);

private slots:
    void onActionNewTriggered();
    void onActionOpenRemoteTriggered();
    void onActionOpenLocalTriggered();
    void onActionOpenSerialportTriggered();
    void onActionBackupTriggered();
    void onActionRestoreTriggered();
    void onActionExitTriggered();
    void onActionToolbarTriggered();
    void onActionSessionListTriggered();
    void onActionTTYOptionsTriggered();
    void onActionVNCOptionsTriggered();
    void onActionRDPOptionsTriggered();
    void onActionToolOptionsTriggered();
    void onActionSystemOptionsTriggered();
    void onActionRestartOptionsTriggered();
    void onActionSettingTriggered();    
    void onActionAboutTriggered();
    void onActionHelpTriggered();
    void onActionWebsiteTriggered();
    void onActionSshKeyManageTriggered();
    void onActionAdminTriggered();
    void onActionLicenseTriggered();
    void onActionUpgradeTriggered();
    void onActionPlaybookOptionsTriggered();
    void onActionactionPurchaseLicenseTriggered();

    void onFilterArrivedArrived(const QString& name, int type);
    void onFilterCreateArrived(const QString& name);


    // file
    void onActionStopSessionTriggered();
    void onActionReconnectSessionTriggered();
    void onActionReconnectAllSessionTriggered();
    void onActionLogToFileTriggered();
    void onActionStopLogFileTriggered();
    void onActionStopAllLogFileTriggered();
    void onActionOpenLogFileTriggered();
    void onActionOpenLogDirectoryTriggered();
    // edit
    void onActionCopyTriggered();
    void onActionPasteTriggered();
    void onActionPasteSelectionTextTriggered();
    void onActionSelectAllTriggered();
    void onActionFindTriggered();
    void onActionFindNextTriggered();
    void onActionFindPreviousTriggered();
    void onActionFindAllTriggered();
    void onActionClearScreenTriggered();
    void onActionClearHistoryTriggered();
    void onActionClearAllTriggered();
    void onActionResetWindowSizeTriggered();
    // tab
    void onActionTabCloseTriggered();
    void onActionLeftTabCloseTriggered();
    void onActionRightTabCloseTriggered();
    void onActionOtherTabCloseTriggered();
    void onActionAllTabCloseTriggered();

    // tool
    void onActionTunnelTriggered();

    // window
    void onActionTopAlwayTriggered();
    void onActionTrayModeTriggered();
    void onActionTranslucentTriggered();

    // menu show.
    void onMenuFileAboutToShow();
    void onMenuLogAboutToShow();
    void onMenuEditAboutToShow();
    void onMenuViewAboutToShow();
    void onMenuToolAboutToShow();
    void onMenuTabAboutToShow();
    void onMenuWindowAboutToShow();
    void onMenuHelpAboutToShow();


private:
    void initMenuBar();
    void initToolBar();
    void initStatusBar();
    void restoreLastState();
    void saveLastState();

    void resetWindowOpacity(bool on, int v);

    bool checkAdminLogin();
    Q_INVOKABLE void tryToMakeLicenseTrial();

    // QWoShowWidget
    QWoShowerWidget *activeShowerWidget();
    QWoTermWidget *focusTermWidget();
private:
    Ui::QWoMainWindow *ui;
    QPointer<QTabBar> m_tab;
    QPointer<QWoSessionList> m_sessions;
    QPointer<QDockWidget> m_sessionDock;
    QPointer<QWoShower> m_shower;
    QPointer<QWoRecentHistory> m_recent;
    QPointer<QPushButton> m_btnTabMerge;
    QPointer<QWoTunnelDialog> m_dlgTunnel;
};
