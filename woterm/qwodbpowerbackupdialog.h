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
