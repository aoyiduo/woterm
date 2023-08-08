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
class QWoTermWidget;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QHBoxLayout;
class QVBoxLayout;
class QWoUnionInputItem;

class QWoTermWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent=nullptr);
    virtual ~QWoTermWidgetImpl();
    void joinToVertical(const QString& target);
    void joinToHorizontal(int row, const QString& target);
    // different function for diffent thing, donot merge them.
    // add for merge.
    void attachBy(QWoTermWidget *term);
    // add for seperate.
    void setForCreated(QWoTermWidget *term);

    QList<QPointer<QWoTermWidget>> termAll();
    int termCount();

    Q_INVOKABLE void openInput();
    void closeInput();
signals:
    void aboutToClose(QCloseEvent* event);

private slots:
    void onRootSplitterDestroy();
    void onUnionInputArrived(const QString& cmd);
    void onMergeTheRightTabTriggered();
    void onSeperateChildWindowTriggered();
private:
    Q_INVOKABLE void init();
protected:
    virtual QWoTermWidget *createTermWidget(const QString& target, int gid, QWidget *parent) = 0;
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
    virtual void closeEvent(QCloseEvent *event);
    virtual bool handleTabMouseEvent(QMouseEvent* ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual bool handleCustomProperties();
    virtual void updateEnable(bool on);
    virtual bool eventFilter(QObject *watched, QEvent *event);

    virtual EHistoryFileState historyFileState();
    virtual void outputHistoryToFile();
    virtual void stopOutputHistoryToFile(bool all);
protected:
    void addAssistant(QWidget *w, bool first);
private:
    void broadcastMessage(int type, QVariant msg);
    void addToList(QWoTermWidget *w);
    void removeFromList(QWoTermWidget *w);
    void resetTabText();
    void setTabText(const QString& title);
    bool event(QEvent* e);
private:
    void resetUnionInput();
protected:
    friend class QWoTermWidget;
    const int m_gid;
    const QPointer<QTabBar> m_tab;
    QPointer<QSplitter> m_termRoot;
    QPointer<QSplitter> m_root;
    QPointer<QWoTermWidget> m_inject; //
    QList<QPointer<QWoTermWidget>> m_terms;
    QPointer<QMenu> m_menu;

    QPointer<QWoUnionInputItem> m_input;

};
