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
