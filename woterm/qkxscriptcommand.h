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

#ifndef QKXSCRIPTCOMMAND_H
#define QKXSCRIPTCOMMAND_H

#include <QObject>
#include <QPointer>
#include <QVariant>

class QWoSshChannel;
class QKxScriptCommand : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList hosts READ targetHosts WRITE setTargetHosts NOTIFY hostsChanged)
public:
    explicit QKxScriptCommand(QObject *parent = nullptr);
    virtual ~QKxScriptCommand();

    QStringList targetHosts() const;
    void setTargetHosts(const QStringList& hosts);

    Q_INVOKABLE void start();
    Q_INVOKABLE void abort();

signals:
    void hostsChanged();

    void commandStart();
    void subcommandStart(const QString& host);
    void errorArrived(const QString& host, const QString& error);
    void subcommandFinished(const QString& host, int code);
    void commandFinished();
private slots:
    void onConnectionFinished(bool ok);
    void onFinishArrived(int code);
    void onErrorArrived(const QString& error, const QVariantMap& userData);
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
protected:
    bool runTask(int idx);
    virtual QWoSshChannel *channelGet() = 0;
    virtual bool runTask(QWoSshChannel *channel) = 0;
    virtual int lastExitCode(QWoSshChannel *channel) = 0;
    virtual void runAbort();
protected:
    int groupId(const QString& key);
private:
    QPointer<QWoSshChannel> m_channel;
    QStringList m_hosts;
    bool m_abort;
    int m_index;
};

#endif // QKXSCRIPTCOMMAND_H
