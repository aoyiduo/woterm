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

#ifndef QWOGROUPINPUTDIALOG_H
#define QWOGROUPINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class QWoGroupInputDialog;
}

class QWoGroupInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoGroupInputDialog(const QString& name=QString(), int order = 0, QWidget *parent = nullptr);
    ~QWoGroupInputDialog();

signals:
    void apply(const QString& name, int order);
private slots:
    void onApplyClicked();
private:
    Ui::QWoGroupInputDialog *ui;
};

#endif // QWOGROUPINPUTDIALOG_H
