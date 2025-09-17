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

#pragma once

#include "qwotermwidget.h"

#include <QPointer>

class QWoRLoginTermWidgetImpl;
class QWoRLogin;
class QWoModem;
class QMessageBox;

class QWoRLoginTermWidget : public QWoTermWidget
{
    Q_OBJECT
private:
    enum EStateConnect {
        ESC_Ready = 0,
        ESC_Connecting = 1,
        ESC_Connected = 2,
        ESC_Disconnected = 3
    };
public:
    explicit QWoRLoginTermWidget(const QString& target, int gid, QWidget *parent=nullptr);
    virtual ~QWoRLoginTermWidget();
    bool isConnected();
    void stop();
    Q_INVOKABLE void reconnect(bool restore = false);
private slots:
    void onFinishArrived(int code);
    void onDataArrived(const QByteArray& buf);
    void onErrorArrived(const QByteArray& buf);
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onTermSizeChanged(int lines, int columns);
    void onSendData(const QByteArray& buf);
    void onCopyToClipboard();
    void onRestoreLastPath();
    void onPasteFromClipboard();
    void onForceToReconnect();
    void onPasswordInputResult(const QString& pass, bool isSave);
    void onSessionReconnect();
    void onVerticalSplitView();
    void onHorizontalSplitView();
    void onVerticalInviteView();
    void onHorizontalInviteView();
    void onCloseThisSession();
    void onForceToCloseThisSession();
    void onShowFindBar();
    void onDuplicateInNewWindow();
    void onModifyThisSession();
    void onZmodemSend(bool local=true);
    void onZmodemRecv(bool local=true);
    void onZmodemAbort();
    void onZmodemDataArrived(const QByteArray& buf);
    void onZmodemStatusArrived(const QByteArray& buf);
    void onZmodemFinished();
    void onTitleChanged(const QString& title);
    void onAdjustPosition();

    void onActivePathArrived(const QString &path);

    // login
    void onTimeoutToInputUserName();
    void onTimeoutToInputUserPassword();
protected:
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    int isZmodemCommand(const QByteArray &data);
    bool checkZmodemInstall();
private:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual QList<QString> collectUnsafeCloseMessage();
private:
    QPointer<QWoRLogin> m_rlogin;
    QPointer<QWoPasswordInput> m_passInput;
    QPointer<QMessageBox> m_dlgConfirm;
    QPointer<QMenu> m_menu;
    QPointer<QAction> m_copy;
    QPointer<QAction> m_paste;
    QPointer<QAction> m_output;
    QPointer<QAction> m_stop;
    QPointer<QWoModem> m_modem;
    bool m_savePassword;
    int m_loginCount;

    EStateConnect m_stateConnected;
    QString m_lastActivePath;
    bool m_restoreLastActivePath;
};
