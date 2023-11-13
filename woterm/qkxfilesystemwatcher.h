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
