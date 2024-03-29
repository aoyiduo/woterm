﻿/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/
#ifndef QWOSERIALWIDGETIMPL_H
#define QWOSERIALWIDGETIMPL_H

#include "qwoshowerwidget.h"
#include "qwotermwidget.h"
#include <QPointer>

class QSplitter;
class QWoSerialInput;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QSerialPort;
class QWoModem;

class QWoSerialWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoSerialWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoSerialWidgetImpl();
signals:
    void aboutToClose(QCloseEvent* event);
protected:
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
    virtual void closeEvent(QCloseEvent *event);
    virtual bool handleTabMouseEvent(QMouseEvent* ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual bool handleCustomProperties();
    virtual void updateEnable(bool on);

    virtual EHistoryFileState historyFileState();
    virtual void outputHistoryToFile();
    virtual void stopOutputHistoryToFile(bool all);
private slots:
    void onDestroyReady();
    void onNewSessionMultplex();
    void onConnectReady(const QString& target);
    void onDisconnect();
    void onMoreReady();
    void handleRead();
    void handleError();
private:
    Q_INVOKABLE void init();
private:
    void setTabText(const QString& title);
private:
    const int m_gid;
    const QPointer<QTabBar> m_tab;
    QPointer<QSplitter> m_root;
    QPointer<QWoTermWidget> m_term;
    QPointer<QWoSerialInput> m_input;
    QPointer<QMenu> m_menu;
    QPointer<QSerialPort> m_serial;
};

#endif
