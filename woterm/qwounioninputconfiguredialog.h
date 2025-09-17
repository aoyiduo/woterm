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

#ifndef QWOUNIONINPUTCONFIGUREDIALOG_H
#define QWOUNIONINPUTCONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class QWoUnionInputConfigureDialog;
}

class QWoUnionInputConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoUnionInputConfigureDialog(bool isRN, QWidget *parent = nullptr);
    ~QWoUnionInputConfigureDialog();

    bool isEnterRN();

private slots:
    void onYesButtonClicked();
private:
    Ui::QWoUnionInputConfigureDialog *ui;
};

#endif // QWOUNIONINPUTCONFIGUREDIALOG_H
