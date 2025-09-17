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

#ifndef QKXSCRIPTLOCALCOMMAND_H
#define QKXSCRIPTLOCALCOMMAND_H

#include <QObject>
#include <QPointer>

class QProcess;
class QKxScriptLocalCommand : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged)
    Q_PROPERTY(int lastExitCode READ lastExitCode NOTIFY commandFinished)
public:
    explicit QKxScriptLocalCommand(QObject *parent = nullptr);

    QString command() const;
    void setCommand(const QString& cmd);

    int lastExitCode() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void abort();
signals:
    void commandChanged();
    void commandStart();
    void commandFinished();
    void dataArrived(const QByteArray& data);

private slots:
    void onFinished(int code);
    void onReadyRead();
    void onErrorReadyRead();
    void onError();
    void onProcessDestroy();
private:
    QString m_command;
    QPointer<QProcess> m_proc;
    int m_lastExitCode;
};

#endif // QKXSCRIPTLOCALCOMMAND_H
