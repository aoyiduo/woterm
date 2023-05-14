/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
