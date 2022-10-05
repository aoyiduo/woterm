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
    QString result() const;
    bool isPrivate() const;
private slots:
    void onButtonSaveClicked();   
private:
    Ui::QWoSftpNameDialog *ui;
    QString m_result;
};

#endif // QWOSFTPNAMEDIALOG_H
