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

#include "qwotermwidget.h"

#include <QPointer>

class QWoPtyTermWidgetImpl;
class QWoPty;
class QMessageBox;

class QWoPtyTermWidget : public QWoTermWidget
{
    Q_OBJECT
    enum EStateConnect {
        ESC_Ready = 0,
        ESC_Connecting = 1,
        ESC_Connected = 2,
        ESC_Disconnected = 3
    };
public:
    explicit QWoPtyTermWidget(const QString& target, int gid, QWidget *parent=nullptr);
    virtual ~QWoPtyTermWidget();

    bool isConnected() {
        return true;
    }
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
    void onVerticalSplitView();
    void onHorizontalSplitView();
    void onVerticalInviteView();
    void onHorizontalInviteView();
    void onCloseThisSession();    
    void onForceToCloseThisSession();
    void onShowFindBar();
    void onModifyThisSession();
    void onTitleChanged(const QString& title);

    void onPasteTestFont();
private:
    Q_INVOKABLE void reconnect();
    QString shellPath() const;
private:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual QList<QString> collectUnsafeCloseMessage();
private:
    QPointer<QWoPty> m_pty;
    QPointer<QMenu> m_menu;
    EStateConnect m_stateConnected;
    QPointer<QMessageBox> m_dlgConfirm;
    QPointer<QAction> m_copy;
    QPointer<QAction> m_paste;
    QPointer<QAction> m_output;
    QPointer<QAction> m_stop;
};
