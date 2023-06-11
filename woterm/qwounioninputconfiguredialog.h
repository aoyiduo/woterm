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
