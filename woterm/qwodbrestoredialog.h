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
