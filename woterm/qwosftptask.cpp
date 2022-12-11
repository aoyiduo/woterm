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

#include "qwosftptask.h"
#include "qwosetting.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <QDebug>
#include <QFile>

static bool createTasksTable(SQLite::Database& db) {
    QString sql="CREATE TABLE IF NOT EXISTS tasks(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql += "name VARCHAR(100) NOT NULL,";
    sql += "local VARCHAR(1024) NOT NULL,";
    sql += "remote VARCHAR(1024) NOT NULL,";
    sql += "isDir INT DEFAULT (0),";
    sql += "isDown INT DEFAULT (0),";
    sql += "state INT DEFAULT (0),";
    sql += "ct DATETIME NOT NULL,";
    sql += "ut DATETIME NOT NULL";
    sql +=")";
    db.exec(sql.toUtf8());
    if(db.tableExists("tasks")) {
        return true;
    }
    return false;
}

static bool createErrorsTable(SQLite::Database& db) {
    QString sql="CREATE TABLE IF NOT EXISTS errors(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql += "tid INT NOT NULL,";
    sql += "desc TEXT NOT NULL,";
    sql += "ct DATETIME NOT NULL";
    sql +=")";
    db.exec(sql.toUtf8());
    if(db.tableExists("errors")) {
        return true;
    }
    return false;
}

QWoSftpTask::QWoSftpTask(QObject *parent)
    : QObject(parent)
    , m_bInit(false)
{
    m_dbFile = QWoSetting::sftpTaskDbPath();
    init();
}

int QWoSftpTask::addTask(const QString &name, const QString &pathLocal, const QString &pathRemote, bool isDir, bool isDown)
{
    int tid = 0;
    return tid;
}

void QWoSftpTask::addError(int tid, const QString &desc)
{

}

void QWoSftpTask::init()
{
    if(m_bInit) {
        return;
    }
    if(QFile::exists(m_dbFile)) {
        if(!databaseValid(m_dbFile)) {
            // bad db file.
            QFile::remove(m_dbFile);
            init();
        }else{
            m_bInit = true;
            return;
        }
    }else{
        try{
            SQLite::Database db(m_dbFile.toUtf8(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
            createTasksTable(db);
            createErrorsTable(db);
        }catch(...) {
            qWarning() << "failed to create database" << m_dbFile;
        }
    }
    m_bInit = databaseValid(m_dbFile);
}

bool QWoSftpTask::databaseValid(const QString &dbFile)
{
    try {
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        SQLite::Statement health(db, "PRAGMA quick_check");
        if(health.executeStep()) {
            QByteArrayList dbset = {"tasks", "errors"};
            for(auto it = dbset.begin(); it != dbset.end(); it++) {
                QByteArray name = *it;
                if(!db.tableExists(name)) {
                    return false;
                }
            }
            return true;
        }
    } catch (...) {
        qWarning() << "databaseValid" << dbFile;
    }
    return false;
}
