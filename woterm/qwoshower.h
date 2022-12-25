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

class QTabBar;
class QWoTermWidget;
class QWoTermWidgetImpl;
class QWoShowerWidget;
class QWoUsageWidget;
class QMenu;
class QWoFloatWindow;

#define TAB_TYPE_NAME ("tabtype")
#define TAB_TARGET_IMPL ("tabimpl")

class QWoShower : public QStackedWidget
{
    Q_OBJECT
private:
    typedef enum {
        ETShell = 0x01,
        ETSsh = 0x02,
        ETScript = 0x03
    } ETabType;
public:
    explicit QWoShower(QTabBar *tab, QWidget *parent=nullptr);
    virtual ~QWoShower();
    bool openLocalShell();
    bool openScriptRuner(const QString& script);
    bool openSsh(const QString& target, int gid = -1);
    bool openSsh(const QStringList& targets, int gid = -1);
    bool openSftp(const QString& target, int gid = -1);
    bool openTelnet(const QString& target);
    bool openRLogin(const QString& target);
    bool openMstsc(const QString& target);
    bool openVnc(const QString& target);
    bool openSerialPort();
    void setBackgroundColor(const QColor& clr);
    void openFindDialog();

    Q_INVOKABLE bool restoreSession(QWoShowerWidget *impl);
    Q_INVOKABLE void floatSession(QWoShowerWidget *impl, bool full);

    static void forceToCloseTopLevelWidget(QWidget *child);

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
    void closeSession(int idx);
    void createTab(QWoShowerWidget *widget, const QIcon& icon, const QString& tabName);
    bool tabMouseButtonPress(QMouseEvent *ev);

private slots:
    void onTabCloseRequested(int index);
    void onTabCurrentChanged(int index);
    void onTabWidgetDestroy(QObject *it);
    void onTabbarDoubleClicked(int index);
    void onCloseThisTabSession();
    void onCloseOtherTabSession();
    void onFloatThisTabSession();
    void onCleanNilFloatWindow();
    void onTabContextMenu(QMouseEvent *ev);
    void onOpenRemoteSession();
    void onOpenLocalSession();

private:
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

    QList<QPointer<QWoFloatWindow>> m_floats;
};
