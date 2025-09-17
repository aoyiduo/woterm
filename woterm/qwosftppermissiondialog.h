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

#ifndef QWOSFTPPERMISSIONDIALOG_H
#define QWOSFTPPERMISSIONDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSftpPermissionDialog;
}

class QWoSftpPermissionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSftpPermissionDialog(const QString& path, const QString permission, QWidget *parent = nullptr);
    virtual ~QWoSftpPermissionDialog();

    int permssionResult(bool *subdirs) const;
private slots:
    void onSubmitButtonClicked();
    void onPermissionSet();
private:
    int permissionToNumber();
private:
    Ui::QWoSftpPermissionDialog *ui;
    QString m_permissionOld;
    int m_permissionResult;
};

#endif // QWOSFTPPERMISSIONDIALOG_H
