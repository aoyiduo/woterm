#pragma once

#include <QStringList>
#include <QObject>
#include <QPointer>

class QLocalSocket;

class FunArgReader : public QObject
{
    Q_OBJECT
public:
    explicit FunArgReader(QLocalSocket *local, QObject *parent=nullptr);
    void readAll();

    QStringList next();
private:
    QPointer<QLocalSocket> m_socket;
    QByteArray m_buf;
};

class FunArgWriter : public QObject
{
    Q_OBJECT
public:
    explicit FunArgWriter(QLocalSocket *local, QObject *parent=nullptr);
    void write(const QStringList& funArgs);
private:
    QPointer<QLocalSocket> m_socket;
};
