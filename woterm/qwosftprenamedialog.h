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

#ifndef QWOSFTPRENAMEDIALOG_H
#define QWOSFTPRENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSftpRenameDialog;
}

class QWoSftpRenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSftpRenameDialog(const QString& path, const QString& name, QWidget *parent = nullptr);
    ~QWoSftpRenameDialog();
    QString pathResult();
private slots:
    void onButtonCommitClicked();
    void onButtonPathModifyClicked();
    void onSavePathChanged(const QString& path);
private:
    Ui::QWoSftpRenameDialog *ui;
    QString m_path, m_nameOld, m_pathResult;
};

#endif // QWOSFTPRENAMEDIALOG_H
