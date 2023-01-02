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

#ifndef QWOPTY_H
#define QWOPTY_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QPointer>

struct PtyMsg;
class QWoPty : public QObject
{
    Q_OBJECT
public:
    explicit QWoPty(QObject *parent = nullptr);
    virtual ~QWoPty();
    bool hasRunning();
    bool start(int cols,int rows, const QString& shellPath);
    void stop();
    virtual void write(const QByteArray& buf) = 0;
    virtual void updateSize(int cols, int rows) = 0;
signals:
    void dataArrived(const QByteArray& buf);
    void errorArrived(const QByteArray& buf);
    void finishArrived(int code);
    void connectionFinished(bool ok);
private slots:
    virtual void onReadyRead() = 0;
    virtual void onFinished() = 0;
private:
    virtual bool init(int cols, int rows, const QString& shell) = 0;
    virtual bool cleanup() = 0;
    virtual bool isRunning() = 0;
};



class QWoPtyFactory : public QObject
{
    Q_OBJECT
public:
    explicit QWoPtyFactory(QObject *parent=nullptr);
    ~QWoPtyFactory();
    static QWoPtyFactory *instance();
    QWoPty *create();
    void release(QWoPty *obj);
private slots:
    void onFinished(int);
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QWoPty>> m_dels;
    QMutex m_mutex;
};


#endif // QWOPTY_H
