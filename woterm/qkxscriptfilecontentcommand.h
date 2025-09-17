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

#ifndef QKXSCRIPTIFILECONTENTCOMMAND_H
#define QKXSCRIPTIFILECONTENTCOMMAND_H

#include "qkxscriptcommand.h"

class QKxScriptFileContentCommand : public QKxScriptCommand
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
    Q_PROPERTY(bool fileRead READ fileRead WRITE setFileRead)
    Q_PROPERTY(int maxFileSize READ maxFileSize WRITE setMaxFileSize)
    Q_PROPERTY(QByteArray content READ fileContent WRITE setFileContent NOTIFY contentChanged)
public:
    explicit QKxScriptFileContentCommand(QObject *parent = nullptr);
    virtual ~QKxScriptFileContentCommand();

    QString filePath() const;
    void setFilePath(const QString& path);

    bool fileRead();
    void setFileRead(bool on);

    int maxFileSize() const;
    void setMaxFileSize(int size);

    QByteArray fileContent() const;
    void setFileContent(const QByteArray& all);
signals:
    void contentChanged();
private slots:
    void onSftpCommandStart(int type, const QVariantMap& userData);
    void onSftpCommandFinish(int type, const QVariantMap& userData);
protected:
    virtual QWoSshChannel *channelGet();
    virtual bool runTask(QWoSshChannel *channel);
    virtual int lastExitCode(QWoSshChannel *channel);
private:
    QString m_filePath;
    int m_fileMaxSize;
    bool m_fileRead;
    int m_opcode;
    QByteArray m_content;
};

#endif // QKXSCRIPTIFILECONTENTCOMMAND_H
