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

#ifndef QWOAPPLICATION_H
#define QWOAPPLICATION_H

#include <QPointer>
#include <QApplication>
#include <QSystemTrayIcon>

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
class QWoTunnelApplication : public QWoApplication
{
    Q_OBJECT
public:
    explicit QWoTunnelApplication(int &argc, char **argv);
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

    void onMessageReceived(const QString& msg);
    void onDelayActiveWindow();
private:
    QPointer<QWoTunnelDialog> m_main;
    QPointer<QMenu> m_menu;
    QSystemTrayIcon m_tray;
};

#endif // QWOAPPLICATION_H
