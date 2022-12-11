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

#ifndef QWODBSFTPDOWNLISTDIALOG_H
#define QWODBSFTPDOWNLISTDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoDBSftpDownListDialog;
}

class QWoDBSftpDownSync;
class QWoDBSftpDownListDialog : public QDialog
{
    Q_OBJECT

public:
    static QString result(QWoDBSftpDownSync *sync, QWidget *parent = nullptr);
private:
    explicit QWoDBSftpDownListDialog(QWoDBSftpDownSync *sync, QWidget *parent = nullptr);
    ~QWoDBSftpDownListDialog();
private slots:
    void onButtonRefleshClicked();
    void onButtonSelectClicked();
    void onSyncListArrived(const QStringList& fileNames);
    void onListItemDoubleClicked(const QModelIndex& idx);
private:
    Ui::QWoDBSftpDownListDialog *ui;
    QPointer<QWoDBSftpDownSync> m_sync;
    QString m_fileName;
};

#endif // QWODBSFTPDOWNLISTDIALOG_H
