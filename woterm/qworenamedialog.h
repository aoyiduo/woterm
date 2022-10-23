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

#ifndef QWORENAMEDIALOG_H
#define QWORENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoRenameDialog;
}

class QWoRenameDialog : public QDialog
{
    Q_OBJECT

public:
    static QString open(const QString& name, QWidget *parent= nullptr);
    static QString open(const QString& name, const QString& errMsg, QWidget *parent= nullptr);
    static QString open(const QString& name, const QString& title, const QString& errMsg, QWidget *parent= nullptr);
private slots:
    void onButtonSaveClicked();
    void onAdjustSize();
protected:
    explicit QWoRenameDialog(const QString& name, QWidget *parent = nullptr);
    ~QWoRenameDialog();
    void setErrorMessage(const QString& msg);
    QString result() const;
private:
    Ui::QWoRenameDialog *ui;
    QString m_result;
};

#endif // QWORENAMEDIALOG_H
