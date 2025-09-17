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

#ifndef QKXSCRIPTREMOTECOMMAND_H
#define QKXSCRIPTREMOTECOMMAND_H

#include "qkxscriptcommand.h"

class QWoSshShell;
class QKxScriptRemoteCommand : public QKxScriptCommand
{
    Q_OBJECT
    Q_PROPERTY(QString command READ shellCommand WRITE setShellCommand NOTIFY commandChanged)
    Q_PROPERTY(QString enterSymbols READ enterSymbols WRITE setEnterSymbols NOTIFY enterSymbolsChanged)
public:
    explicit QKxScriptRemoteCommand(QObject *parent = nullptr);
    virtual ~QKxScriptRemoteCommand();

    QString shellCommand() const;
    void setShellCommand(const QString& cmd);

    QString enterSymbols() const;
    void setEnterSymbols(const QString& key);

    Q_INVOKABLE void setKeyboardInteractiveWithTerminal(bool on);

signals:
    void commandChanged();
    void enterSymbolsChanged();
    void dataArrived(const QString& host, const QByteArray& data);

private slots:
    void onDataArrived(const QByteArray& data);
    void onSendData(const QByteArray& data);
private:
    virtual QWoSshChannel *channelGet();
    virtual bool runTask(QWoSshChannel *channel);
    virtual int lastExitCode(QWoSshChannel *channel);
    virtual void runAbort();
private:
    QString m_command, m_enterSymbols;
    bool m_keyboardInteractive;
    QPointer<QWoSshShell> m_shell;
};



#endif // QKXSCRIPTREMOTECOMMAND_H
