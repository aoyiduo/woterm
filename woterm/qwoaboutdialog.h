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

#ifndef QWOABOUTDIALOG_H
#define QWOABOUTDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoAboutDialog;
}

class QKxHttpClient;

class QWoAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoAboutDialog(QWidget *parent = nullptr);
    ~QWoAboutDialog();

private slots:
    void onVersionCheck(int code, const QByteArray& body);
    void onVersionCheckButtonClicked();
    void onResult(int code, const QByteArray& body);
private:
    Ui::QWoAboutDialog *ui;

    QPointer<QKxHttpClient> m_httpClient;
};

#endif // QWOABOUTDIALOG_H
