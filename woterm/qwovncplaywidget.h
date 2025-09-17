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

#ifndef QWOVNCPLAYWIDGET_H
#define QWOVNCPLAYWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QPropertyAnimation>

class QScrollArea;
class QWoVncWidget;
class QWoPasswordInput;

class QWoVncToolForm;
class QWoVncFtpWidget;

class QWoVncPlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoVncPlayWidget(const QString& target, QWidget *parent = 0);
    ~QWoVncPlayWidget();

    bool smartResize();
    void setSmartResize(bool on);
    bool isConnected();
    void stop();
    void reconnect();
private slots:
    // self
    void onToolEnter();
    void onToolLeave();
    void onFtpClicked();
    void onDeleteLater();
    void onToolSizeChanged(const QSize& sz);
    void onForceToClose();
private:
    void resizeEvent(QResizeEvent *e);
    void updateToolbarPosition();
private:
    QPointer<QScrollArea> m_area;
    QPointer<QWoVncWidget> m_player;
    QPointer<QWoVncToolForm> m_tool;
    QPointer<QPropertyAnimation> m_ani;
    QPointer<QWoVncFtpWidget> m_ftp;
};

#endif // QWOVNCPLAYWIDGET_H
