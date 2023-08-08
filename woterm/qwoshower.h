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

#include <QStackedWidget>
#include <QPointer>
#include <QIcon>
#include <QList>
#include <QMap>

class QTabBar;
class QWoTermWidget;
class QWoTermWidgetImpl;
class QWoShowerWidget;
class QWoUsageWidget;
class QMenu;
class QWoFloatWindow;

#define TAB_TYPE_NAME ("tabtype")
#define TAB_TARGET_IMPL ("tabimpl")

class QWoShowerWidget;
class QWoShower : public QStackedWidget
{
    Q_OBJECT
private:
    typedef enum {
        ETShell = 0x01,
        ETSsh = 0x02,
        ETScript = 0x03
    } ETabType;

    enum ELimitType {
        ELT_LOCALSHELL = 0,
        ELT_SCRIPTRUNNER = 1,
        ELT_SSH = 2,
        ELT_SFTP = 3,
        ELT_TELNET = 4,
        ELT_RLOGIN = 5,
        ELT_RDP = 6,
        ELT_VNC = 7,
        ELT_SERIALPORT = 8,
        ELT_PLAYBOOK = 9,
        ELT_SCRIPT

    };
public:
    explicit QWoShower(QTabBar *tab, QWidget *parent=nullptr);
    virtual ~QWoShower();
    bool openPlayBook(const QString& name, const QString& path);
    bool openLocalShell();
    void attachLocalShell(QWoTermWidget* term, int idx);

    bool openSsh(const QString& target, int gid = -1);
    void attachSsh(QWoTermWidget* term, int idx);
    bool openSftp(const QString& target, int gid = -1);

    bool openTelnet(const QString& target);
    void attachTelnet(QWoTermWidget* term, int idx);

    bool openRLogin(const QString& target);
    void attachRLogin(QWoTermWidget* term, int idx);

    bool openMstsc(const QString& target);
    bool openVnc(const QString& target);
    bool openSerialPort();
    void setBackgroundColor(const QColor& clr);

    void closeSession(int idx, bool force);
    void closeTab(int idx);
    void closeOtherTabs(int idx);
    void closeLeftTabs(int idx);
    void closeRightTabs(int idx);
    void closeAllTabs();
    //

    Q_INVOKABLE void mergeFromRightTab();
    Q_INVOKABLE void seperateToRightTab();
    Q_INVOKABLE bool restoreSession(QWoShowerWidget *impl);
    Q_INVOKABLE void floatSession(QWoShowerWidget *impl, bool full);

    Q_INVOKABLE static void forceToCloseTopLevelWidget(QWidget *child);

    int tabCount();
signals:
    void tabEmpty();
    void openSessionManage();
    void floatChanged(QWoShowerWidget *shower, bool bFloat);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    void syncGeometry(QWidget *widget);

    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);


private:
    void createTab(QWoShowerWidget *widget, const QIcon& icon, const QString& tabName, int idx = -1);
    bool tabMouseButtonPress(QMouseEvent *ev);
    bool tabMouseButtonMove(QMouseEvent *ev);

private slots:
    void onTabCloseRequested(int index);
    void onTabCurrentChanged(int index);
    void onTabWidgetDestroy(QObject *it);
    void onTabbarDoubleClicked(int index);
    void onCloseThisTabSession();
    void onCloseOtherTabSession();
    void onNewTheSameSession();
    void onCopyTabSessionAddress();
    void onEditThisTabSession();
    void onFloatThisTabSession();
    void onCleanNilFloatWindow();
    void onTabContextMenu(QMouseEvent *ev);
    void onOpenRemoteSession();
    void onOpenLocalSession();

private:
    QIcon m_bookico;
    QIcon m_jsico;
    QIcon m_ptyico;
    QIcon m_telico;
    QIcon m_sshico;
    QIcon m_ftpico;
    QIcon m_rloico;
    QIcon m_mtsico;
    QIcon m_serico;
    QIcon m_vncico;
    QPointer<QTabBar> m_tab;
    QPointer<QMenu> m_tabMenu;
    QFont m_font;
    int m_implCount;

    QList<QPointer<QWoFloatWindow>> m_floats;

    bool m_tabDragAway;
    QPointer<QWoShowerWidget> m_tabDragWidget;
};
