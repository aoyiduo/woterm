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

#ifndef QWOIDENTIFYPUBLICKEYDIALOG_H
#define QWOIDENTIFYPUBLICKEYDIALOG_H

#include <QDialog>

namespace Ui {
class QWoIdentifyPublicKeyDialog;
}

class QWoIdentifyPublicKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoIdentifyPublicKeyDialog(const QString& key, QWidget *parent = nullptr);
    ~QWoIdentifyPublicKeyDialog();

private slots:
    void onCopy();
private:
    Ui::QWoIdentifyPublicKeyDialog *ui;
};

#endif // QWOIDENTIFYPUBLICKEYDIALOG_H
