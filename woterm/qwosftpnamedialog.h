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

#ifndef QWOSFTPNAMEDIALOG_H
#define QWOSFTPNAMEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSftpNameDialog;
}

class QWoSftpNameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QWoSftpNameDialog(const QString& name, QWidget *parent = nullptr);
    ~QWoSftpNameDialog();
    void setPrivateButtonVisible(bool on);
    QString result() const;
    bool isPrivate() const;
private slots:
    void onButtonSaveClicked();   
private:
    Ui::QWoSftpNameDialog *ui;
    QString m_result;
};

#endif // QWOSFTPNAMEDIALOG_H
