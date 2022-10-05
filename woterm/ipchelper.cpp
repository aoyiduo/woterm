#include "ipchelper.h"

#include <QLocalSocket>
#include <QDataStream>

static const int intsize = sizeof(int);

FunArgReader::FunArgReader(QLocalSocket *local, QObject *parent)
    :QObject (parent)
{
    m_socket = local;
}

void FunArgReader::readAll() {
    m_buf.append(m_socket->readAll());
}

QStringList FunArgReader::next() {
    int length;
    if(m_buf.size() <= 0) {
        return QStringList();
    }
    memcpy(&length, m_buf.data(), intsize);
    if(m_buf.size() < length) {
        return QStringList();
    }
    QByteArray funArgs = m_buf.mid(intsize, length);
    if(funArgs.length() != length) {
        qFatal("not so much memory for store.");
    }
    m_buf.remove(0, length+intsize);
    char *pbuf = funArgs.data();
    int count;
    memcpy(&count, pbuf, intsize);
    pbuf += intsize;
    QStringList data;
    for(int i = 0; i < count; i++) {
        int length;
        memcpy(&length, pbuf, intsize);
        pbuf += intsize;
        QByteArray arg;
        arg.append(pbuf, length);
        pbuf += length;
        data << arg;
    }
    return data;
}

FunArgWriter::FunArgWriter(QLocalSocket *local, QObject *parent)
    :QObject (parent)
{
    m_socket = local;
}

void FunArgWriter::write(const QStringList &funArgs) {
    QByteArray fun;
    int count = funArgs.length();
    fun.append((char*)&count, intsize);
    for(int i = 0; i < funArgs.length(); i++) {
        QByteArray arg = funArgs.at(i).toLocal8Bit();
        int length = arg.length() + 1;
        fun.append((char*)&length, intsize);
        fun.append(arg.data(), arg.length());
        fun.append('\0');
    }
    QByteArray buf;
    int length = fun.length() + 1;
    buf.append((char*)&length, intsize);
    buf.append(fun.data(), fun.length());
    buf.append('\0');

    m_socket->write((char*)buf.data(), buf.length());
}
