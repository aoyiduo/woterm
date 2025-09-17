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

#ifndef QWOPLAYBOOKCONFIGUREDIALOG_H
#define QWOPLAYBOOKCONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class QWoPlaybookConfigureDialog;
}

class QWoPlaybookConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoPlaybookConfigureDialog(QWidget *parent = nullptr);
    ~QWoPlaybookConfigureDialog();

    QString path() const;
private slots:
    void onApply();
    void onBrowserButtonClicked();
private:
    Ui::QWoPlaybookConfigureDialog *ui;
};

#endif // QWOPLAYBOOKCONFIGUREDIALOG_H
