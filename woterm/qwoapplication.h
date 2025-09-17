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

#ifndef QWOAPPLICATION_H
#define QWOAPPLICATION_H

#include <QPointer>
#include <QApplication>
#include <QSystemTrayIcon>

class QKxCryptFileEngine;
class QWoMainWindow;
class QWoApplication : public QApplication
{
    Q_OBJECT
public:
    enum EAppType {
        eTunnel,
        eMain
    };
public:
    explicit QWoApplication(int &argc, char **argv);
    static QWoApplication *instance();
    virtual QWidget *mainWindow() = 0;
    virtual EAppType type() = 0;
protected:
    Q_INVOKABLE virtual void init() = 0;
    Q_INVOKABLE void setWindowOpacity(QWidget* w);
    virtual bool notify(QObject *receiver, QEvent *ev);
};

class QWoMainApplication : public QWoApplication
{
    Q_OBJECT
public:
    explicit QWoMainApplication(int &argc, char **argv);
protected:
    QWidget *mainWindow();
    virtual void init();
    virtual EAppType type() {
        return eMain;
    }    
private:
    QPointer<QWoMainWindow> m_main;
};

class QWoTunnelDialog;
class QKxLocalPeer;
class QWoTunnelApplication : public QWoApplication
{
    Q_OBJECT
public:
    explicit QWoTunnelApplication(int &argc, char **argv);
    bool isClient();
    void sendMessage(const QString& msg);
protected:
    QWidget *mainWindow();
    virtual void init();
    virtual EAppType type() {
        return eTunnel;
    }
protected slots:
    void onTrayActive(QSystemTrayIcon::ActivationReason reason);
    void onMenuAboutToShow();
    void onShowWindow();
    void onNewSessionWindow();
    void onAboutToQuit();
    void onRestartApplication();

    void onMessageReceived(const QString& msg);
    void onDelayActiveWindow();
private:
    QPointer<QWoTunnelDialog> m_main;
    QPointer<QKxLocalPeer> m_peer;
    QPointer<QMenu> m_menu;
    QSystemTrayIcon m_tray;    
};

#endif // QWOAPPLICATION_H
