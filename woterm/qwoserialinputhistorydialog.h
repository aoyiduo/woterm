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

#ifndef QWOSERIALINPUTHISTORYDIALOG_H
#define QWOSERIALINPUTHISTORYDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSerialInputHistoryDialog;
}

class QWoSerialInputHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSerialInputHistoryDialog(const QString& msg, QWidget *parent = nullptr);
    ~QWoSerialInputHistoryDialog();

signals:
    void messageArrived(const QString& msg);
private:
    Ui::QWoSerialInputHistoryDialog *ui;
};

#endif // QWOSERIALINPUTHISTORYDIALOG_H
