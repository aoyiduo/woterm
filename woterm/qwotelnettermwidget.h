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

#pragma once

#include "qwotermwidget.h"

#include <QPointer>

class QWoTelnetTermWidgetImpl;
class QWoTelnet;
class QWoModem;
class QMessageBox;
class QWoTelnetTermWidget : public QWoTermWidget
{
    Q_OBJECT
    enum EStateConnect {
        ESC_Ready = 0,
        ESC_Connecting = 1,
        ESC_Connected = 2,
        ESC_Disconnected = 3
    };

public:
    explicit QWoTelnetTermWidget(const QString& target, int gid, QWidget *parent=nullptr);
    virtual ~QWoTelnetTermWidget();

    bool isConnected();
    void stop();
    Q_INVOKABLE void reconnect();
private slots:
    void onFinishArrived(int code);
    void onDataArrived(const QByteArray& buf);
    void onErrorArrived(const QByteArray& buf);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onTermSizeChanged(int lines, int columns);
    void onSendData(const QByteArray& buf);
    void onCopyToClipboard();
    void onPasteFromClipboard();
    void onForceToReconnect();
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
protected:
    int isZmodemCommand(const QByteArray &data);
    bool checkZmodemInstall();

private:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual QList<QString> collectUnsafeCloseMessage();
private:
    QPointer<QWoTelnet> m_telnet;
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
};
