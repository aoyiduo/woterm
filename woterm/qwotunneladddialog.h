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

#ifndef QWOTUNNELADDDIALOG_H
#define QWOTUNNELADDDIALOG_H

#include "qwotunnelmodel.h"

#include <QDialog>

namespace Ui {
class QWoTunnelAddDialog;
}

class QWoTunnelAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoTunnelAddDialog(QWidget *parent = nullptr);
    ~QWoTunnelAddDialog();

    TunnelInfo result() const;
private slots:
    void onApplyButtonClicked();
    void onSessionsButtonClicked();
    void onTypeCurrentIndexChanged(int idx);
    void onHostLocalAssistClicked();
    void onHostRemoteAssistClicked();
protected:
    Ui::QWoTunnelAddDialog *ui;
    qint64 m_id;
};

#endif // QWOTUNNELADDDIALOG_H
