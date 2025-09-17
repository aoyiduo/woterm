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
private:
    Ui::QWoAboutDialog *ui;

    QPointer<QKxHttpClient> m_httpClient;
};

#endif // QWOABOUTDIALOG_H
