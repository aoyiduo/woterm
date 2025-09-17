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

#include <QScrollArea>
#include <QPointer>


class QWoLoadingWidget;
class QWoTermMask;
class QRdpWidget;

class QWoRdpWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWoRdpWidget(const QString& target, QWidget *parent=nullptr);
    ~QWoRdpWidget();

    bool smartResize() const;
    void setSmartResize(bool on);

    bool isConnected();
    void stop();
    Q_INVOKABLE void reconnect();
signals:
    void aboutToClose(QCloseEvent* event);
private:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool focusNextPrevChild(bool next);
private slots:
    void onSessionReconnect();
    void onForceToCloseThisSession();
    void onFinished();

    void onConnectingArrived();
    void onConnectedArrived();
    void onDisconnectedArrived();

private:    
    Q_INVOKABLE void resizeRdpWidget();
private:
    const QString m_target;
    QPointer<QScrollArea> m_area;
    QPointer<QRdpWidget> m_rdp;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QWoTermMask> m_mask;    
    bool m_smartResize;
    bool m_autoReconnect;
};
