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


#ifndef QWODBBACKUPDIALOG_H
#define QWODBBACKUPDIALOG_H

#include <QDialog>

namespace Ui {
class QWoDbBackupDialog;
}

class QWoDbBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDbBackupDialog(QWidget *parent = nullptr);
    ~QWoDbBackupDialog();

private slots:
    void onBackupButtonClicked();
    void onAssistButtonClicked(int idx);
private:
    Ui::QWoDbBackupDialog *ui;
};

#endif // QWODBBACKUPDIALOG_H
