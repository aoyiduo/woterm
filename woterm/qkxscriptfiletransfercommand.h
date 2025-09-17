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

#ifndef QKXSCRIPTFILETRANSFERCOMMAND_H
#define QKXSCRIPTFILETRANSFERCOMMAND_H

#include "qkxscriptcommand.h"

class QKxScriptFileTransferCommand : public QKxScriptCommand
{
    Q_OBJECT
    Q_PROPERTY(QString fileLocal READ fileLocal WRITE setFileLocal NOTIFY fileLocalChanged)
    Q_PROPERTY(QString fileRemote READ fileRemote WRITE setFileRemote NOTIFY fileRemoteChanged)
    Q_PROPERTY(bool fileAppend READ fileAppend WRITE setFileAppend NOTIFY fileAppendChanged)
    Q_PROPERTY(bool fileDownload READ fileDownload WRITE setFileDownload NOTIFY fileDownloadChanged)
public:
    explicit QKxScriptFileTransferCommand(QObject *parent = nullptr);
    virtual ~QKxScriptFileTransferCommand();

    QString fileLocal() const;
    void setFileLocal(const QString& file);

    QString fileRemote() const;
    void setFileRemote(const QString& file);

    bool fileAppend() const;
    void setFileAppend(bool on);

    bool fileDownload() const;
    void setFileDownload(bool on);

signals:
    void fileLocalChanged();
    void fileRemoteChanged();
    void fileAppendChanged();
    void fileDownloadChanged();
    void progressArrived(const QString& host, int v);
private slots:
    void onTransferCommandStart(int type, const QVariantMap& userData);
    void onTransferCommandFinish(int type, const QVariantMap& userData);
    void onTransferProgress(int type, int v, const QVariantMap& userData);
protected:
    virtual QWoSshChannel *channelGet();
    virtual bool runTask(QWoSshChannel *channel);
    virtual int lastExitCode(QWoSshChannel *channel);
private:
    QString m_fileLocal, m_fileRemote;    
    bool m_fileAppend, m_fileDownload;
    int m_transferProgress;
};

#endif // QKXSCRIPTFILETRANSFERCOMMAND_H
