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

#include <QWidget>
#include <QPointer>


class QWoLoadingWidget;
class QWoTermMask;
class QRdpWork;

class QWoRdpWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWoRdpWidget(const QString& target, QWidget *parent=nullptr);
    ~QWoRdpWidget();

    void setFullScreen(bool full);
signals:
    void aboutToClose(QCloseEvent* event);
private slots:
    void onTimeout();
private:
    Q_INVOKABLE void reconnect();
private:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void focusInEvent(QFocusEvent *ev);
    void focusOutEvent(QFocusEvent *ev);
    bool focusNextPrevChild(bool next);
private slots:
    void onSessionReconnect();
    void onForceToCloseThisSession();
    void onFinished();
private:
    const QString m_target;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QWoTermMask> m_mask;
    QPointer<QRdpWork> m_rdp;
};
