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

#ifndef QWOTERMMASK_H
#define QWOTERMMASK_H

#include "qwowidget.h"

namespace Ui {
class QWoTermMask;
}

class QTimer;
class QWoTermMask : public QWoWidget
{
    Q_OBJECT

public:
    explicit QWoTermMask(QWidget *parent = nullptr);
    ~QWoTermMask();

    void setAutoReconnect(bool on);
signals:
    void reconnect();

private slots:
    void onReconnect();
    void onTimeout();

private:
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual void showEvent(QShowEvent *ev);
    virtual void hideEvent(QHideEvent *ev);
private:
    Ui::QWoTermMask *ui;
    QString m_connectLabel;
    bool m_autoReconnect;
    QPointer<QTimer> m_timer;
    int m_reconnectWaitCount;
};

#endif // QWOTERMMASK_H
