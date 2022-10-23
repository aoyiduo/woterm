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

#ifndef QWOIDENTIFYCREATEDIALOG_H
#define QWOIDENTIFYCREATEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoIdentifyCreateDialog;
}

/***
 * Only 4 type
 * ssh-keygen -t dsa | ecdsa | ed25519 | rsa
 * dsa: Because of low security, it is basically no longer used.
 * ecdsa: Because of low security, it is basically no longer used
 * ed25519: Fast encryption and decryption, fast generation time, and higher security.
 * rsa: The encryption and decryption speed is slow, the generation time is slow, and the security is not as high as ed25519. (Many users have good compatibility)
 **/

class QWoIdentifyCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoIdentifyCreateDialog(QWidget *parent = 0);
    ~QWoIdentifyCreateDialog();
private slots:
    void onButtonCreateClicked();
    void onButtonTypeRsaClicked();
    void onButtonTypeED25519Clicked();
    void onAdjustSize();
private:
    Ui::QWoIdentifyCreateDialog *ui;
};

#endif // QWOIDENTIFYCREATEDIALOG_H
