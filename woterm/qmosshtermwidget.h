/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QMOSSHTERMWIDGET_H
#define QMOSSHTERMWIDGET_H

#include "qmotermwidget.h"

#include <QPointer>
#include <QVariantMap>

class QWoSshShell;
class QWoModem;
class QShortCut;
class QWoPasswordInput;
class QMessageBox;

class QMoSshTermWidget : public QMoTermWidget
{
    Q_OBJECT
    enum EStateConnect {
        ESC_Ready = 0,
        ESC_Connecting = 1,
        ESC_Connected = 2,
        ESC_Disconnected = 3
    };
public:
    explicit QMoSshTermWidget(const QString& target, QWidget *parent = nullptr);
    virtual ~QMoSshTermWidget();

signals:

private slots:
    void onConnectionFinished(bool ok);
    void onFinishArrived(int code);
    void onDataArrived(const QByteArray& buf);
    void onErrorArrived(const QString& buf, const QVariantMap& userData= QVariantMap());
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onTermSizeChanged(int lines, int columns);
    void onSendData(const QByteArray& buf);
    void onCopyToClipboard();
    void onRestoreLastPath();
    void onActivePathArrived(const QString& path);
    void onPasteFromClipboard();
    void onPasswordInputResult(const QString& pass, bool isSave);
    void onSessionReconnect();
    void onCloseThisSession();
    void onPasteTestFont();
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
    void onSftpConnectReady();
    void onForceToReconnect();
    void onAdjustPosition();
protected:
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    int isZmodemCommand(const QByteArray &data);
    bool checkZmodemInstall();
private:
    Q_INVOKABLE void reconnect(bool restore=false);
    Q_INVOKABLE void executeCommand(const QByteArray& cmd);
    bool validProxyJumper();
private:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual QList<QString> collectUnsafeCloseMessage();
private:
    static int m_gid;
    QPointer<QWoSshShell> m_ssh;
    QPointer<QWoSshShell> m_cmd;
    QPointer<QWoPasswordInput> m_passInput;
    QPointer<QMessageBox> m_dlgConfirm;
    QPointer<QWoModem> m_modem;
    bool m_savePassword;
    QPointer<QShortCut> m_shortCutCopy;
    QPointer<QShortCut> m_shortCutPaste;
    EStateConnect m_stateConnected;
    QString m_lastActivePath;
    bool m_restoreLastActivePath;
};

#endif // QMOSSHTERMWIDGET_H
