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
    void onTerminalOnAppStartClicked();
    void onTunnelDaemonClicked();
private:
    Ui::QWoAdminDialog *ui;
};

#endif // QWOADMINDIALOG_H
