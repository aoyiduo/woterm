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

#ifndef QWOSFTPEDITORDIALOG_H
#define QWOSFTPEDITORDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class QWoSftpEditorDialog;
}

class QWoSftpEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSftpEditorDialog(const QStringList& watchs, QWidget *parent = nullptr);
    ~QWoSftpEditorDialog();

signals:
    void ftpSettingArrived();
    void clearAll();
    void stopArrived(const QString& fileName);
private slots:
    void onClearAllButtonClicked();
    void onStopWatchButtonClicked();
private:
    Ui::QWoSftpEditorDialog *ui;
    QMap<QString, QString> m_watchs;
};

#endif // QWOSFTPEDITORDIALOG_H
