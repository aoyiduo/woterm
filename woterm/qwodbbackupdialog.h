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
