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

#ifndef QKXSCRIPTFILELISTCOMMAND_H
#define QKXSCRIPTFILELISTCOMMAND_H

#include "qkxscriptcommand.h"

class QKxScriptFileListCommand : public QKxScriptCommand
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QVariantList fileInfos READ fileInfos)
    Q_PROPERTY(QStringList fileNames READ fileNames)
public:
    explicit QKxScriptFileListCommand(QObject *parent = nullptr);
    virtual ~QKxScriptFileListCommand();

    QString path() const;
    void setPath(const QString& path);

    QVariantList fileInfos() const;
    QStringList fileNames() const;

signals:
    void pathChanged();
private slots:
    void onSftpCommandStart(int type, const QVariantMap& userData);
    void onSftpCommandFinish(int type, const QVariantMap& userData);
    void onDirOpen(const QString &path, const QList<QVariant> &v, const QVariantMap& userData);
protected:
    virtual QWoSshChannel *channelGet();
    virtual bool runTask(QWoSshChannel *channel);
    virtual int lastExitCode(QWoSshChannel *channel);

private:
    QString m_path;
    QVariantList m_fileLists;
    bool m_dirOpenOk;
};

#endif // QKXSCRIPTFILELISTCOMMAND_H
