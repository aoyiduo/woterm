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

#ifndef QKXFILESYSTEMWATCHER_H
#define QKXFILESYSTEMWATCHER_H

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QDateTime>

class QTimer;
class QKxFileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit QKxFileSystemWatcher(QObject *parent = nullptr);
    virtual ~QKxFileSystemWatcher();

    bool addPath(const QString &file);
    void removePath(const QString &file);

    QStringList files() const;

signals:
    void fileChanged(const QString &path);
private slots:
    void onFileCheckTimeout();
private:
    QMap<QString, QDateTime> m_files;
    QPointer<QTimer> m_timer;
};

#endif // QKXFILESYSTEMWATCHER_H
