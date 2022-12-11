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

#ifndef QWODBBACKUPDIALOG_H
#define QWODBBACKUPDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoDbBackupDialog;
}

class QWoDBSftpUploadSync;
class QWoDbBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDbBackupDialog(QWidget *parent = nullptr);
    ~QWoDbBackupDialog();

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
    Ui::QWoDbBackupDialog *ui;
    QPointer<QWoDBSftpUploadSync> m_sync;
};

#endif // QWODBBACKUPDIALOG_H
