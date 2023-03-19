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
