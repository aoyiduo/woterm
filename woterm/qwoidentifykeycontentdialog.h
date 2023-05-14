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

#ifndef QWOIDENTIFYKEYCONTENTDIALOG_H
#define QWOIDENTIFYKEYCONTENTDIALOG_H

#include <QDialog>

namespace Ui {
class QWoIdentifyKeyContentDialog;
}

class QWoIdentifyKeyContentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoIdentifyKeyContentDialog(bool isPublicKey, const QString& key, QWidget *parent = nullptr);
    ~QWoIdentifyKeyContentDialog();

private slots:
    void onCopy();
private:
    Ui::QWoIdentifyKeyContentDialog *ui;
};

#endif // QWOIDENTIFYKEYCONTENTDIALOG_H
