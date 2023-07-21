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

#include "qwotunnelserver.h"

#include "qwoutils.h"

#include <qssh.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/sftp.h>

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDateTime>
#include <QMap>
#include <QCoreApplication>

#define MAX_BUFFER   (1024*70)

static int gMaxSessionMultiplexCount = 1000;

QWoTunnelChannel::QWoTunnelChannel(QTcpSocket *socket, const QString& sessionName, QObject *parent)
    : QWoSshChannel(parent)
    , m_socket(socket)
    , m_sessionName(sessionName)
{
    m_socket->setParent(this);
    QObject::connect(this, SIGNAL(dataArrived(QByteArray)), this, SLOT(onRemoteDataArrived(QByteArray)));
    QObject::connect(this, SIGNAL(connectionFinished(bool)), this, SLOT(onInternalConnectionFinished(bool)));
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(onLocalReadyRead()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

QWoTunnelChannel::~QWoTunnelChannel()
{
    m_socket->deleteLater();
}

void QWoTunnelChannel::stop()
{
    QWoSshChannel::stop();
    m_socket->disconnect();
    m_socket->close();
}

void QWoTunnelChannel::onDisconnected()
{
    if(m_socket->isOpen()) {
        m_socket->close();
    }
    QWoSshChannel::stop();
}

void QWoTunnelChannel::flushAndClose()
{
    m_socket->flush();
    m_socket->close();
}

int QWoTunnelChannel::mygid(const QString &host, int port)
{
    struct SIDData {
        int gid;
        int count;
    };
    static QMap<QString, SIDData> gids;
    QString key = host + QString::number(port);
    SIDData sid = gids.take(key);
    if(sid.gid == 0 || sid.count > gMaxSessionMultiplexCount) {
        sid.gid = QWoUtils::gid();
        //qDebug() << "==================================multiplex" << key << sid.count;
        sid.count = 0;
    }
    sid.count++;
    gids.insert(key, sid);
    return sid.gid;
}

bool QWoTunnelChannel::connectToRemote(int gid)
{
    if(!start(m_sessionName, gid)) {
        emit errorArrived(tr("Failed to connect to server."), QVariantMap());
        emit finishArrived(-1);
        return false;
    }
    return true;
}


class QWoPortmappingChannel: public QWoTunnelChannel
{
private:
    ssh_channel m_channel;
    QString m_hostRemote;
    int m_portRemote;
public:
    explicit QWoPortmappingChannel(QTcpSocket *socket, const QString& sessionName, const QString& hostRemote, int portRemote, QWoTunnelServer *parent= nullptr)
        : QWoTunnelChannel(socket, sessionName, parent)
        , m_channel(nullptr)
        , m_hostRemote(hostRemote)
        , m_portRemote(portRemote) {
        connectToRemote(mygid(hostRemote, portRemote));
    }

    virtual ~QWoPortmappingChannel() {
        if(m_channel) {
            ssh_channel_close(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
        m_socket->deleteLater();
    }
protected:
    void onInternalConnectionFinished(bool ok) {

    }

    int nextPort() {
        static int lport = 1024;
        lport++;
        if(lport > 65530) {
            lport = 1024;
        }
        return lport;
    }

    void onRemoteDataArrived(const QByteArray &buf)
    {
        m_socket->write(buf);
    }

    void onLocalReadyRead()
    {
        QByteArray buf = m_socket->readAll();
        if(m_conn) {
            m_conn->customWrite(this, buf);
        }
    }

    bool _handleOpen(void *session)
    {
        m_channel = ssh_channel_new(ssh_session(session));
        if (m_channel == nullptr){
            return false;
        }
        ssh_channel_set_blocking(m_channel, 1);
        // you need a separate channel for the tunnel as first parameter;
        // second and third parameters are the remote endpoint;
        // fourth and fifth parameters are sent to the remote server so that they can be logged on that server.
        if (ssh_channel_open_forward(m_channel, m_hostRemote.toUtf8(), m_portRemote, "127.0.0.168", nextPort()) != SSH_OK) {
            return false;
        }
        return true;
    }

    bool handleOpen(void *session) {
        bool ok = _handleOpen(session);
        emit connectionFinished(ok);
        return ok;
    }

    void handleClose(int code) {
        m_lastCommandExitStatus = 0;
       // qDebug() << "handleClose" << "ssh_channel_get_exit_status" << m_lastCommandExitStatus;
        if(m_channel) {
            ssh_channel_close(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
        emit finishArrived(code);
    }

    bool handleRead()
    {
        int total = 0;
        while(1){
            QByteArray buf(MAX_BUFFER, Qt::Uninitialized);
            int nbytes = ssh_channel_read_nonblocking(m_channel, buf.data(), MAX_BUFFER, 0);
            if(nbytes == 0) {
                return true;
            }else if(nbytes < 0) {
                return false;
            }
            total += nbytes;
            buf.resize(nbytes);
            emit dataArrived(buf);
        }
        return true;
    }

    bool handleRequest(MsgRequest &msg)
    {
        if(m_channel == nullptr) {
            return false;
        }
        if(msg.type == MT_CUSTOMDATA) {
            int err = ssh_channel_write(m_channel, msg.data.data(), msg.data.length());
            return err != SSH_ERROR;
        }
        return true;
    }

    void *channel()
    {
        return m_channel;
    }

    void init()
    {

    }
};

class QWoSocks45Channel: public QWoTunnelChannel
{
    enum ERunStep {
        eNotDefined = 0,
        eNegotiation,
        eEstablishConnection,
        eDataTransmission
    };    

    enum EError {
        eException = -2,
        eNotSupport = -1,
        eSuccess = 0,
        eNotEnoughData
    };

    enum ESocksType {
        eUnknow,
        eSocks4,
        eSocks5
    };

private:
    ssh_channel m_channel;
    QString m_hostRemote;
    int m_portRemote;
    ERunStep m_step;
    QByteArray m_bufferRead;
    ESocksType m_socksType;
public:
    explicit QWoSocks45Channel(QTcpSocket *socket, const QString& sessionName, QWoTunnelServer *parent= nullptr)
        : QWoTunnelChannel(socket, sessionName, parent)
        , m_channel(nullptr)
        , m_step(eNotDefined)
        , m_socksType(eUnknow){
        m_bufferRead.reserve(1024 * 5);
    }

    virtual ~QWoSocks45Channel() {
        if(m_channel) {
            ssh_channel_close(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
        m_socket->deleteLater();
    }
protected:

    void onInternalConnectionFinished(bool ok) {
        if(m_socksType == eSocks5) {
            handleConnectionV5Finished(ok);
        }else{
            handleConnectionV4Finished(ok);
        }
    }

    void handleConnectionV5Finished(bool ok) {
        if(m_socket == nullptr || !m_socket->isOpen()) {
            return;
        }
        if(m_socksType != eSocks5 || m_step != eEstablishConnection) {
            return;
        }

        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        QHostAddress addr = m_socket->localAddress();
        quint16 port = m_socket->localPort();

        if(ok) {
            // // o  X'00' succeeded
            out << quint8(5) << quint8(0) << quint8(0) << quint8(0x01) << quint32(addr.toIPv4Address()) << port;
        }else{
            // o  X'01' general SOCKS server failure
            // o  X'03' Network unreachable
            // o  X'05' Connection refused
            out << quint8(5) << quint8(5) << quint8(0) << quint8(0x01) << quint32(0) << quint16(0);
        }
        m_socket->write(buf);
        m_step = eDataTransmission;
    }

    void handleConnectionV4Finished(bool ok) {
        if(m_socksType != eSocks4 || m_step != eEstablishConnection) {
            return;
        }
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        QHostAddress addr = m_socket->localAddress();
        quint16 port = m_socket->localPort();
        quint32 ipv4 = addr.toIPv4Address();
        out << quint8(0) << quint8(ok ? 0x5a : 0x5b) << port << ipv4;
        m_socket->write(buf);
        m_step = eDataTransmission;
    }


    inline bool streamEnough(QDataStream& in) {
        return in.status() == QDataStream::Ok;
    }

    int nextPort() {
        static int lport = 1024;
        lport++;
        if(lport > 65530) {
            lport = 1024;
        }
        return lport;
    }

    void onRemoteDataArrived(const QByteArray &buf)
    {
        m_socket->write(buf);
    }

    void onLocalReadyRead()
    {
        m_bufferRead.append(m_socket->readAll());

        if(m_step == eDataTransmission) {
            if(m_conn) {
                m_conn->customWrite(this, m_bufferRead);
                m_bufferRead.clear();
            }
            return;
        }

        if(m_socksType == eUnknow) {
            QDataStream in(&m_bufferRead, QIODevice::ReadOnly);

            quint8 protocol;
            in >> protocol;
            if(protocol == 5) {
                m_socksType = eSocks5;
                m_step = eNegotiation;
            }else if(protocol == 4) {
                m_socksType = eSocks4;
                m_step = eEstablishConnection;
            }else{
                m_socket->close();
                return;
            }
        }
        if(m_step == eNegotiation) {
            if(handleNegotiation() < 0) {
                flushAndClose();
                return;
            }
        }else if(m_step == eEstablishConnection) {
            if(handleConnection() < 0) {
                flushAndClose();
                return;
            }
        }
    }

    EError handleV5Negotiation() {
        QDataStream in(&m_bufferRead, QIODevice::ReadOnly);

        quint8 protocol, nMethods;
        in >> protocol >> nMethods;
        QVector<quint8> methods;
        for(int i = 0; i < nMethods; i++) {
            quint8 method;
            in >> method;
            methods.append(method);
        }

        if(!streamEnough(in)) {
            return eNotEnoughData;
        }

        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        if(!methods.contains(0x00)) {
            // If the selected METHOD is X'FF', none of the methods listed by the
            // client are acceptable, and the client MUST close the connection.
            out << quint8(0x5) << quint8(0xff);
            m_socket->write(buf);
            return eNotSupport;
        }
        // o  X'00' NO AUTHENTICATION REQUIRED
        out << quint8(0x5) << quint8(0x00);
        m_socket->write(buf);
        m_step = eEstablishConnection;
        m_bufferRead.remove(0, in.device()->pos());
        return eSuccess;
    }

    EError handleNegotiation() {
        if(m_socksType == eSocks5) {
            return handleV5Negotiation();
        }
        //socks v4/v4a no such phase.
        return eNotSupport;
    }

    EError handleV5Connection() {
        QDataStream in(&m_bufferRead, QIODevice::ReadOnly);
        quint8 protocol, cmd, rsv, atyp;

        in >> protocol >> cmd >> rsv >> atyp;
        if(!streamEnough(in)) {
            return eNotEnoughData;
        }
        if(atyp == 0x01) {
            // IPv4
            quint32 ipv4;
            in >> ipv4;
            if(!streamEnough(in)) {
                return eNotEnoughData;
            }
            m_hostRemote = QHostAddress(ipv4).toString();
            if(m_hostRemote.isNull()) {
                return eException;
            }
        }else if(atyp == 0x03) {
            // Domain name.
            quint8 cnt;
            in >> cnt;
            QByteArray domain;
            domain.resize(cnt);
            in.readRawData(domain.data(), cnt);
            if(!streamEnough(in)) {
                return eNotEnoughData;
            }
            m_hostRemote = domain;
        }else if(atyp == 0x04){
            // IPv6
            quint8 ipv6[16];
            in.readRawData((char*)ipv6, 16);
            if(!streamEnough(in)) {
                return eNotEnoughData;
            }
            m_hostRemote = QHostAddress(ipv6).toString();
            if(m_hostRemote.isNull()) {
                return eException;
            }
        }else{
            return eException;
        }

        quint16 port;
        in >> port;
        m_portRemote = port;
        if(!streamEnough(in)) {
            return eNotEnoughData;
        }

        if(cmd != 0x01) {
            // Bind, UDP.
            m_socket->close();
            return eNotSupport;
        }

        m_bufferRead.remove(0, in.device()->pos());        
        return connectToRemote(mygid(m_hostRemote, m_portRemote)) ? eSuccess : eException;
    }

    EError handleV4Connection() {
        QDataStream in(&m_bufferRead, QIODevice::ReadOnly);
        quint8 protocol, cmd;
        quint16 port;
        quint32 ipv4;
        in >> protocol >> cmd >> port >> ipv4;
        if(!streamEnough(in)) {
            return eNotEnoughData;
        }
        int posUserID = in.device()->pos();
        int posNull = m_bufferRead.indexOf('\0', posUserID);
        if(posNull == -1) {
            return eException;
        }
        QByteArray user;
        user.resize(posNull - posUserID + 1);
        in.readRawData(user.data(), user.length());
        if(!streamEnough(in)) {
            return eNotEnoughData;
        }
        bool isDomain = (ipv4 & 0xFF) == ipv4 && ipv4 > 0;
        if(!isDomain) {
            QHostAddress host(ipv4);
            m_hostRemote = host.toString();
            m_portRemote = port;
            m_bufferRead.remove(0, in.device()->pos());
            return connectToRemote(mygid(m_hostRemote, m_portRemote)) ? eSuccess : eException;;
        }
        // v4a protocol for domain support.
        int posDomain = posNull + 1;
        int posDomainNull = m_bufferRead.indexOf('\0', posDomain);
        if(posDomainNull == -1) {
            return eException;
        }
        int domainLength = posDomainNull - posDomain + 1;
        QByteArray domain;
        domain.resize(domainLength);
        in.readRawData(domain.data(), domainLength);
        if(!streamEnough(in)) {
            return eNotEnoughData;
        }
        m_hostRemote = domain;
        m_portRemote = port;
        m_bufferRead.remove(0, in.device()->pos());
        return connectToRemote(mygid(m_hostRemote, m_portRemote)) ? eSuccess : eException;
    }

    EError handleConnection() {
        if(m_socksType == eSocks5) {
            return handleV5Connection();
        }else if(m_socksType == eSocks4) {
            return handleV4Connection();
        }
        return eNotSupport;
    }

    bool _handleOpen(void *session)
    {
        m_channel = ssh_channel_new(ssh_session(session));
        if (m_channel == nullptr){
            return false;
        }
        ssh_channel_set_blocking(m_channel, 1);
        // you need a separate channel for the tunnel as first parameter;
        // second and third parameters are the remote endpoint;
        // fourth and fifth parameters are sent to the remote server so that they can be logged on that server.
        qint64 t1 = QDateTime::currentMSecsSinceEpoch();
       // qDebug() << "---------------" << m_hostRemote << m_portRemote << t1 << qint64(m_channel);
        if (ssh_channel_open_forward(m_channel, m_hostRemote.toUtf8(), m_portRemote, "127.0.0.158", nextPort()) != SSH_OK) {            
            qint64 t2 = QDateTime::currentMSecsSinceEpoch();
           // qDebug() << "+++++++++++++++++++" << m_hostRemote << m_portRemote << t2 - t1 << qint64(m_channel);
            return false;
        }
        return true;
    }

    bool handleOpen(void *session) {
        bool ok = _handleOpen(session);
        emit connectionFinished(ok);
        return ok;
    }

    void handleClose(int code) {
        m_lastCommandExitStatus = 0;
       // qDebug() << "handleClose" << "ssh_channel_get_exit_status" << m_lastCommandExitStatus;
        if(m_channel) {
            ssh_channel_close(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
        emit finishArrived(code);
    }

    bool handleRead()
    {
        int total = 0;
        while(1){
            QByteArray buf(MAX_BUFFER, Qt::Uninitialized);
            int nbytes = ssh_channel_read_nonblocking(m_channel, buf.data(), MAX_BUFFER, 0);
            if(nbytes == 0) {
                return true;
            }else if(nbytes < 0) {
                return false;
            }
            total += nbytes;
            buf.resize(nbytes);
            emit dataArrived(buf);
        }
        return true;
    }

    bool handleRequest(MsgRequest &msg)
    {
        if(m_channel == nullptr) {
            return false;
        }
        if(msg.type == MT_CUSTOMDATA) {
            int err = ssh_channel_write(m_channel, msg.data.data(), msg.data.length());
            return err != SSH_ERROR;
        }
        return true;
    }

    void *channel()
    {
        return m_channel;
    }

    void init()
    {

    }
};


QWoTunnelServer::QWoTunnelServer(QObject *parent)
    : QObject(parent)
{

}

QWoTunnelServer::~QWoTunnelServer()
{

}

bool QWoTunnelServer::isRunning()
{
    return m_server && m_server->isListening();
}

void QWoTunnelServer::stop()
{
    if(m_server == nullptr) {
        return;
    }
    m_server->close();
    for(auto it = m_channels.begin(); it != m_channels.end(); it++) {
        QWoSshChannel *chn = *it;
        QWoSshFactory::instance()->release(chn);
    }
    m_channels.clear();
}

void QWoTunnelServer::setMaxSessionMultiplexCount(int cnt)
{
    gMaxSessionMultiplexCount = cnt;
}

void QWoTunnelServer::onConnectionFinished(bool ok)
{
//    if(!ok) {
//        emit errorArrived(tr("Failed to connect server."));
//    }
}

void QWoTunnelServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        QWoTunnelChannel *channel = createChannel(client, m_sessionName);

        QObject::connect(channel, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
        QObject::connect(channel, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
        QObject::connect(channel, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
        QObject::connect(channel, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));

        m_channels.append(channel);

        //qDebug() << "onNewConnection" << qint64(channel);
    }
}

void QWoTunnelServer::onFinishArrived(int code)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    QWoSshFactory::instance()->release(channel);
    //qDebug() << "onFinishArrived" << (qint64)channel;
    for(auto it = m_channels.begin(); it != m_channels.end();) {
        QWoSshChannel *hit = *it;
        if(channel == hit) {
            m_channels.erase(it);
            return;
        }else{
            it++;
        }
    }
}

void QWoTunnelServer::onErrorArrived(const QString &buf, const QVariantMap &userData)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    if(channel == nullptr) {
        return;
    }
    emit errorArrived(buf);
}

void QWoTunnelServer::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    if(channel == nullptr) {
        return;
    }
    channel->stop();
}

bool QWoTunnelServer::start(const QString &sessionName, const QString& hostLocal, int portLocal)
{
    if(isRunning()) {
        return false;
    }
    m_sessionName = sessionName;

    if(m_server != nullptr) {
        m_server->deleteLater();
    }
    m_server = new QTcpServer(this);
    QObject::connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    if(!m_server->listen(QHostAddress(hostLocal), portLocal)) {
        m_server->deleteLater();
        emit errorArrived(m_server->errorString());
        return false;
    }
    return true;
}

QWoPortmappingServer::QWoPortmappingServer(QObject *parent)
    : QWoTunnelServer(parent)
{

}

bool QWoPortmappingServer::start(const QString &sessionName, const QString &hostLocal, int portLocal, const QString &hostRemote, int portRemote)
{
    if(QWoTunnelServer::start(sessionName, hostLocal, portLocal)) {
        m_hostRemote = hostRemote;
        m_portRemote = portRemote;
        return true;
    }
    return false;
}

QWoTunnelChannel *QWoPortmappingServer::createChannel(QTcpSocket *client, const QString &sessionName)
{
    return new QWoPortmappingChannel(client, sessionName, m_hostRemote, m_portRemote, this);
}


QWoSocks45Server::QWoSocks45Server(QObject *parent)
    : QWoTunnelServer(parent)
{

}

bool QWoSocks45Server::start(const QString &sessionName, const QString &hostLocal, int portLocal)
{
    return QWoTunnelServer::start(sessionName, hostLocal, portLocal);
}

QWoTunnelChannel *QWoSocks45Server::createChannel(QTcpSocket *client, const QString& sessionName)
{
    return new QWoSocks45Channel(client, sessionName, this);
}


QWoTunnelFactory::QWoTunnelFactory(QObject *parent)
    : QObject(parent)
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
}

QWoTunnelFactory *QWoTunnelFactory::instance()
{
    static QPointer<QWoTunnelFactory> factory = new QWoTunnelFactory();
    return factory;
}

QWoTunnelServer *QWoTunnelFactory::find(qint64 id)
{
    return m_servers.value(id);
}

bool QWoTunnelFactory::isRunning(qint64 id)
{
    QWoTunnelServer* server = find(id);
    return server != nullptr && server->isRunning();
}

void QWoTunnelFactory::release(qint64 id)
{
    QWoTunnelServer* server = m_servers.take(id);
    if(server != nullptr) {
        server->stop();
        server->deleteLater();
    }
}

void QWoTunnelFactory::releaseAll()
{
    for(auto it = m_servers.begin(); it != m_servers.end(); it++) {
        QWoTunnelServer *server = it.value();
        if(server != nullptr) {
            server->stop();
            server->deleteLater();
        }
    }
}

void QWoTunnelFactory::onAboutToQuit()
{

}
