/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxhistory.h"

#include <QDebug>
#include <QTimer>

QKxHistoryBuffer::QKxHistoryBuffer(QObject *parent)
    : QKxHistory(parent)
    , m_max(1000)
{

}

int QKxHistoryBuffer::maxLine() const {
    return m_max;
}

int QKxHistoryBuffer::append(const TermLine &line)
{
    int nthrow = 100;
    m_lines.append(line);
    if(m_lines.length() > m_max) {
        m_lines = m_lines.mid(nthrow);
        return nthrow;
    }
    return 0;
}

int QKxHistoryBuffer::lineCount() const
{
    return m_lines.length();
}

void QKxHistoryBuffer::clear()
{
    m_lines.clear();
}

TermLine QKxHistoryBuffer::lineAt(int y) const
{
    return m_lines.at(y);
}

TermLine QKxHistoryBuffer::takeLast()
{
    if(m_lines.isEmpty()) {
        return TermLine();
    }
    return m_lines.takeLast();
}

void QKxHistoryBuffer::setMaxLine(int n)
{
    m_max = n;
}

QKxHistoryFile::QKxHistoryFile(const QString &file, QObject *parent)
    : QKxHistory(parent)
{
    m_file.setFileName(file);
    m_file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onFlushTimeout()));
    timer->start(1000);
}

QKxHistoryFile::~QKxHistoryFile()
{
    if(m_file.isOpen()){
        m_file.close();
    }
}

int QKxHistoryFile::maxLine() const
{
    return 0x7FFFFFFF;
}

void QKxHistoryFile::setMaxLine(int n)
{

}

int QKxHistoryFile::append(const TermLine &line)
{
    if(!m_file.isOpen()) {
        return 0;
    }
    QString out;
    for(int i = 0; i < line.cs.length(); i++) {
        TermChar c = line.cs.at(i);
        if(c.count > 0) {
            out.append(QChar(c.c));
        }
    }
    int i = 0;
    for(i = out.length() - 1; i >= 0; i--) {
        if(out.at(i) != QChar::Space) {
            out.resize(i+1);
            break;
        }
    }
    if(i < 0) {
        out.resize(0);
    }
    m_file.write(out.toUtf8());
    m_file.write("\r\n");
    return 0;
}

int QKxHistoryFile::lineCount() const
{
    return 0;
}

void QKxHistoryFile::clear()
{

}

TermLine QKxHistoryFile::lineAt(int y) const
{
    return TermLine();
}

TermLine QKxHistoryFile::takeLast()
{
    return TermLine();
}

void QKxHistoryFile::onFlushTimeout()
{
    if(m_file.isOpen()) {
        m_file.flush();
    }
}
