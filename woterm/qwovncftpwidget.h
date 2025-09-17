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

#ifndef QWOVNCFTPWIDGET_H
#define QWOVNCFTPWIDGET_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class QWoVncFtpWidget;
}

class QKxFtpTransferWidget;
class QKxVncWidget;
class QKxFtpRequest;

class QWoVncFtpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoVncFtpWidget(QKxVncWidget *vnc, QWidget *parent = nullptr);
    ~QWoVncFtpWidget();

private slots:
    void onAboutToClose();
    void onFinished();
private:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);
private:
    Ui::QWoVncFtpWidget *ui;
    QPoint m_dragPosition;
    QPointer<QKxFtpTransferWidget> m_transfer;
    QPointer<QKxVncWidget> m_vnc;
    QPointer<QKxFtpRequest> m_ftpRequest;
};

#endif // QWOVNCFTPWIDGET_H
