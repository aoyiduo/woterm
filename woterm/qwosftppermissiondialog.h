/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
