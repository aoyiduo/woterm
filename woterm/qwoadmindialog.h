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

#ifndef QWOADMINDIALOG_H
#define QWOADMINDIALOG_H

#include <QDialog>

namespace Ui {
class QWoAdminDialog;
}

class QWoAdminDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoAdminDialog(QWidget *parent = nullptr);
    ~QWoAdminDialog();
private slots:
    void onAssistButtonClicked(int idx);
    void onPasswordResetClicked();
    void onTextChanged(const QString& txt);
    void onStartupClicked();
    void onLookupPasswordClicked();
    void onAllowConnectTunnelClicked();
    void onLocalTerminalOnAppStartClicked();
    void onSerialportOnAppStartClicked();
    void onTunnelDaemonClicked();
    void onAllowMainOpacityClicked();
private:
    Ui::QWoAdminDialog *ui;
};

#endif // QWOADMINDIALOG_H
