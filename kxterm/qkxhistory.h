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

#ifndef QHISTORY_H
#define QHISTORY_H

#include <QObject>
#include <QList>
#include <QFile>

#include "qvtedef.h"

class QKxHistory: public QObject
{
public:
    explicit QKxHistory(QObject *parent=nullptr)
        : QObject(parent){
    }
    // > 0: remove line count;
    // = 0: success.
    // < 0: failed;
    virtual int append(const TermLine& line) = 0;
    virtual void setMaxLine(int n=1000) = 0;
    virtual int maxLine() const = 0;
    virtual int lineCount() const = 0;
    virtual void clear() = 0;
    virtual TermLine lineAt(int y) const = 0;
    virtual TermLine takeLast() = 0;
};

class QKxHistoryNone: public QKxHistory
{
public:
    explicit QKxHistoryNone(QObject *parent)
        : QKxHistory(parent){

    }    
    int maxLine() const {
        return 0;
    }
    void setMaxLine(int n){
        Q_UNUSED(n)
    }
    int append(const TermLine& line){
        Q_UNUSED(line)
        return 1;
    }    
    int lineCount() const{
        return 0;
    }
    void clear(){
    }
    TermLine lineAt(int y) const{
        Q_UNUSED(y)
        return TermLine();
    }
    TermLine takeLast(){
        return TermLine();
    }
};

class QKxHistoryBuffer : public QKxHistory
{
public:
    explicit QKxHistoryBuffer(QObject *parent = nullptr);
    int maxLine() const;
    void setMaxLine(int n);
    int append(const TermLine& line);
    int lineCount() const;
    void clear();
    TermLine lineAt(int y) const;
    TermLine takeLast();
private:
    QList<TermLine> m_lines;
    int m_max;
};

class QKxHistoryFile : public QKxHistory
{
public:
    explicit QKxHistoryFile(const QString& file, QObject *parent = nullptr);
    virtual ~QKxHistoryFile();
    int maxLine() const;
    void setMaxLine(int n);
    int append(const TermLine& line);
    int lineCount() const;
    void clear();
    TermLine lineAt(int y) const;
    TermLine takeLast();
private:
    QFile m_file;
};

#endif // QHISTORY_H
