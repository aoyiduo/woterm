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
