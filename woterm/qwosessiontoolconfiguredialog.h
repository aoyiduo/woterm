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

#ifndef QWOSESSIONTOOLCONFIGUREDIALOG_H
#define QWOSESSIONTOOLCONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSessionToolConfigureDialog;
}

class QWoSessionToolConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionToolConfigureDialog(QWidget *parent = nullptr);
    ~QWoSessionToolConfigureDialog();

private slots:
    void onSaveButtonClicked();
    void onBrowserButtonClicked();
private:
    Ui::QWoSessionToolConfigureDialog *ui;
};

#endif // QWOSESSIONTOOLCONFIGUREDIALOG_H
