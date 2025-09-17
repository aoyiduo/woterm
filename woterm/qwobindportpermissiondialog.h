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

#ifndef QWOBINDPORTPERMISSIONDIALOG_H
#define QWOBINDPORTPERMISSIONDIALOG_H

#include <QDialog>

namespace Ui {
class QWoBindPortPermissionDialog;
}

class QWoBindPortPermissionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QWoBindPortPermissionDialog(const QString& target, QWidget *parent = nullptr);
    ~QWoBindPortPermissionDialog();
private slots:
    void onAuthorizationButtonClicked();
    void onNewProcessButtonClicked();
private:
    Ui::QWoBindPortPermissionDialog *ui;
    QString m_target;
};

#endif // QWOBINDPORTPERMISSIONDIALOG_H
