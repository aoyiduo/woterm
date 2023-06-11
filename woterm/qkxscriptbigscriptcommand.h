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

#ifndef QKXSCRIPTBIGSCRIPTCOMMAND_H
#define QKXSCRIPTBIGSCRIPTCOMMAND_H

#include "qkxscriptcommand.h"

class QWoSshShell;
class QWoSshFtp;

class QKxScriptBigScriptCommand : public QKxScriptCommand
{
    Q_OBJECT
    Q_PROPERTY(QString script READ scriptContent WRITE setScriptContent NOTIFY scriptChanged)
    Q_PROPERTY(QString enterSymbols READ enterSymbols WRITE setEnterSymbols NOTIFY enterSymbolsChanged)
public:
    explicit QKxScriptBigScriptCommand(QObject *parent = nullptr);
    ~QKxScriptBigScriptCommand();


    QString scriptContent() const;
    void setScriptContent(const QString& content);

    QString enterSymbols() const;
    void setEnterSymbols(const QString& key);

    Q_INVOKABLE bool loadFromFile(const QString& fileName);
    Q_INVOKABLE bool saveToFile(const QString& fileName);
    Q_INVOKABLE void setKeyboardInteractiveWithTerminal(bool on);

signals:
    void enterSymbolsChanged();
    void scriptChanged();
    void dataArrived(const QString& host, const QByteArray& data);
private slots:
    void onSftpCommandStart(int type, const QVariantMap& userData);
    void onSftpCommandFinish(int type, const QVariantMap& userData);
    void onShellDataArrived(const QByteArray& data);
    void onSendData(const QByteArray& data);
private:
    virtual QWoSshChannel *channelGet();
    virtual bool runTask(QWoSshChannel *channel);
    virtual int lastExitCode(QWoSshChannel *channel);
    virtual void runAbort();
private:
    QString m_scriptContent, m_enterSymbols;
    bool m_keyboardInteractive;
    QPointer<QWoSshShell> m_shell;
    QPointer<QWoSshFtp> m_sftp;
    int m_opcode;
};

#endif // QKXSCRIPTBIGSCRIPTCOMMAND_H
