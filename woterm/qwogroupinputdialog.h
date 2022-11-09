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
