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

#ifndef QWODBPOWERBACKUPDIALOG_H
#define QWODBPOWERBACKUPDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoDbPowerBackupDialog;
}

class QWoDBSftpUploadSync;
class QWoDbPowerBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDbPowerBackupDialog(QWidget *parent = nullptr);
    ~QWoDbPowerBackupDialog();

private slots:
    void onCurrentIndexChanged();
    void onAdjustLayout();
    void onSftpDetailButtonClicked();
    void onFileSaveClicked();
    void onFileUploadClicked();
    void onFileBrowserClicked();
    void onInfoArrived(int action, int err, const QString& errDesc);
private:
    void resetSftpUrl();
private:
    Ui::QWoDbPowerBackupDialog *ui;
    QPointer<QWoDBSftpUploadSync> m_sync;
};

#endif // QWODBPOWERBACKUPDIALOG_H
