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

#ifndef QMOPTYTERMWIDGET_H
#define QMOPTYTERMWIDGET_H

#include "qmotermwidget.h"

#include <QPointer>

class QMessageBox;
class QWoPty;

class QMoPtyTermWidget : public QMoTermWidget
{
    Q_OBJECT
    enum EStateConnect {
        ESC_Ready = 0,
        ESC_Connecting = 1,
        ESC_Connected = 2,
        ESC_Disconnected = 3
    };
public:
    explicit QMoPtyTermWidget(const QString& target, QWidget *parent = nullptr);
    virtual ~QMoPtyTermWidget();
private slots:
    void onFinishArrived(int code);
    void onConnectionFinished(bool ok);
    void onDataArrived(const QByteArray& buf);
    void onErrorArrived(const QByteArray& buf);
    void onTermSizeChanged(int lines, int columns);
    void onSendData(const QByteArray& buf);
    void onCopyToClipboard();
    void onPasteFromClipboard();
    void onForceToReconnect();
    void onSessionReconnect();
    void onCloseThisSession();
    void onForceToCloseThisSession();
    void onShowFindBar();
    void onModifyThisSession();
    void onTitleChanged(const QString& title);
private:
    Q_INVOKABLE void reconnect();
    QString shellPath() const;
private:
    virtual void resizeEvent(QResizeEvent *ev);
private:
    QPointer<QWoPty> m_pty;
    EStateConnect m_stateConnected;
    QPointer<QMessageBox> m_dlgConfirm;
    QPointer<QAction> m_copy;
    QPointer<QAction> m_paste;
    QPointer<QAction> m_output;
    QPointer<QAction> m_stop;
};

#endif // QMOPTYTERMWIDGET_H
