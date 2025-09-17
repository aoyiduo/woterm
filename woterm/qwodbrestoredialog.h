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

#ifndef QWODBRESTOREDIALOG_H
#define QWODBRESTOREDIALOG_H

#include <QDialog>

namespace Ui {
class QWoDBRestoreDialog;
}

class QWoDBRestoreDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDBRestoreDialog(QWidget *parent = nullptr);
    ~QWoDBRestoreDialog();
private slots:
    void onAssistButtonClicked(int idx);
    void onRestoreButtonClicked();
private:
    Ui::QWoDBRestoreDialog *ui;
};

#endif // QWODBRESTOREDIALOG_H
