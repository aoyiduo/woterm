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

#ifndef QMODBSFTPDETAILASSIST_H
#define QMODBSFTPDETAILASSIST_H

#include <QPointer>
#include <QObject>

class QWoSshFtp;
class QTimer;
class QWidget;

class QMoDbSftpDetailAssist : public QObject
{
    Q_OBJECT
public:
    explicit QMoDbSftpDetailAssist(QWidget *parent = nullptr);
    ~QMoDbSftpDetailAssist();
    Q_INVOKABLE void verify(const QVariant& v);
signals:
    void verifyResult(int err);
private slots:
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onFinishArrived(int code);
    void onInputArrived(const QString &title, const QString &prompt, bool visible);
    void onTestTimeout();
private:
    void release();
private:
    QPointer<QWidget> m_widgetParent;
    QPointer<QWoSshFtp> m_sftp;
    QPointer<QTimer> m_timer;
};

#endif // QMODBSFTPDETAILASSIST_H
