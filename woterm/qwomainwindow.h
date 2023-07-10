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
    void onActionFindTriggered();
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



private:
    void initMenuBar();
    void initToolBar();
    void initStatusBar();
    void restoreLastState();
    void saveLastState();

    bool checkAdminLogin();
    Q_INVOKABLE void tryToMakeLicenseTrial();
private:
    Ui::QWoMainWindow *ui;
    QPointer<QTabBar> m_tab;
    QPointer<QWoSessionList> m_sessions;
    QPointer<QDockWidget> m_sessionDock;
    QPointer<QWoShower> m_shower;
    QPointer<QWoRecentHistory> m_recent;
    QPointer<QPushButton> m_btnTabMerge;
};
