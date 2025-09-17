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

#ifndef QWODBSFTPDETAILDIALOG_H
#define QWODBSFTPDETAILDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoDbSftpDetailDialog;
}

class QWoSshFtp;
class QTimer;

class QWoDbSftpDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDbSftpDetailDialog(QWidget *parent = nullptr);
    ~QWoDbSftpDetailDialog();

private slots:
    void onCurrentTextChanged();
    void onIdentifyButtonClicked();
    void onTestButtonClicked();
    void onApplyButtonClicked();
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onFinishArrived(int code);
    void onInputArrived(const QString &title, const QString &prompt, bool visible);
    void onTestTimeout();
    void onAssistButtonClicked(int i);
    void onSetEditToPasswordMode();
private:
    void release();
    void saveDetail();
private:
    Ui::QWoDbSftpDetailDialog *ui;
    QPointer<QWoSshFtp> m_sftp;
    QPointer<QTimer> m_timer;
};

#endif // QWODBSFTPDETAILDIALOG_H
