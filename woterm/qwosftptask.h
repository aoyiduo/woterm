/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QWOSFTPTASK_H
#define QWOSFTPTASK_H

#include <QObject>
#include <QDateTime>

class QWoSftpTask : public QObject
{
    Q_OBJECT
public:
    explicit QWoSftpTask(QObject *parent = nullptr);

    /* return tid */
    int addTask(const QString& name, const QString& pathLocal, const QString& pathRemote, bool isDir, bool isDown);
    void addError(int tid, const QString& desc);
private:
    void init();
private:
    static bool databaseValid(const QString &dbFile);
signals:

private:
    QString m_dbFile;
    bool m_bInit;
};

#endif // QWOSFTPTASK_H
