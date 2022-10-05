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

#include "qwoserialwidgetimpl.h"
#include "qwoserialinput.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwosshconf.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qwosessionproperty.h"

#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QTabBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QClipboard>
#include <QSerialPort>
#include <QTimer>

#define INPUT_MINI_HEIGHT   (130)

QWoSerialWidgetImpl::QWoSerialWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, parent)
    , m_gid(gid)
    , m_tab(tab)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
    m_root = new QSplitter(this);
    m_root->setOrientation(Qt::Vertical);
    m_root->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_root);
    m_term = new QWoSerialTermWidget(target, this);
    m_root->addWidget(m_term);
    m_input = new QWoSerialInput(target, this);
    m_input->setMinimumHeight(INPUT_MINI_HEIGHT);
    m_root->addWidget(m_input);
    m_root->setChildrenCollapsible(false);    
    setAutoFillBackground(true);
    QPalette pal;
    QColor clr = Qt::gray;
    pal.setColor(QPalette::Background, clr);
    pal.setColor(QPalette::Window, clr);
    setPalette(pal);

    m_serial = new QSerialPort(this);
    connect(m_serial, SIGNAL(readyRead()), this, SLOT(handleRead()));
    connect(m_serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError()));

    QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));
    QObject::connect(m_input, SIGNAL(connectReady(QString)), this, SLOT(onConnectReady(QString)));
    QObject::connect(m_input, SIGNAL(disconnect()), this, SLOT(onDisconnect()));
    QObject::connect(m_input, SIGNAL(sendText(QString)), this, SLOT(onSendText(QString)));
    QObject::connect(m_input, SIGNAL(moreReady()), this, SLOT(onMoreReady()));
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);

//    QTimer *timer = new QTimer(this);
//    timer->start(1000);
//    QObject::connect(timer, &QTimer::timeout, [=](){
//        pasteFormat("QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));QObject::connect(m_input, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));");
//    });
}

QWoSerialWidgetImpl::~QWoSerialWidgetImpl()
{

}

void QWoSerialWidgetImpl::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

bool QWoSerialWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoSerialWidgetImpl::handleTabContextMenu(QMenu *menu)
{

}

void QWoSerialWidgetImpl::updateEnable(bool on)
{

}

void QWoSerialWidgetImpl::onDestroyReady()
{
    close();
    deleteLater();
}

void QWoSerialWidgetImpl::onNewSessionMultplex()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSftp(m_target, m_gid);
}



void QWoSerialWidgetImpl::onConnectReady(const QString &target)
{
    HostInfo hi;
    if(m_serial->isOpen()) {
        m_serial->close();
    }
    if(!QWoSshConf::instance()->find(m_target, &hi)) {
        m_input->reset();
        QMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete"));
        return;
    }

    m_serial->setPortName(target);
    m_serial->setBaudRate(hi.baudRate.toInt());    
    if(hi.stopBits == "2") {
        m_serial->setStopBits(QSerialPort::TwoStop);
    }else if(hi.stopBits == "1.5") {
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    }else{
        m_serial->setStopBits(QSerialPort::OneStop);
    }
    if(hi.dataBits == "5") {
        m_serial->setDataBits(QSerialPort::Data5);
    }else if(hi.dataBits == "6") {
        m_serial->setDataBits(QSerialPort::Data6);
    }else if(hi.dataBits == "7") {
        m_serial->setDataBits(QSerialPort::Data7);
    }else {
        m_serial->setDataBits(QSerialPort::Data8);
    }

    if(hi.flowContrl == "RTS/CTS") {
        m_serial->setFlowControl(QSerialPort::HardwareControl);
    }else if(hi.flowContrl == "XON/XOFF") {
        m_serial->setFlowControl(QSerialPort::SoftwareControl);
    }else {
        m_serial->setFlowControl(QSerialPort::NoFlowControl);
    }

    if(hi.parity == "Even") {
        m_serial->setParity(QSerialPort::EvenParity);
    }else if(hi.parity == "Odd") {
        m_serial->setParity(QSerialPort::OddParity);
    }else if(hi.parity == "Space") {
        m_serial->setParity(QSerialPort::SpaceParity);
    }else if(hi.parity == "Mark") {
        m_serial->setParity(QSerialPort::MarkParity);
    }else{
        m_serial->setParity(QSerialPort::NoParity);
    }
    if(!m_serial->open(QIODevice::ReadWrite)) {
        m_input->reset();
        QMessageBox::warning(this, tr("Error"), tr("Failed to open device."));
        return;
    }
    QString params;
    params.append("\r\n");
    params.append(QString("PortName:%1\r\n").arg(target));
    params.append(QString("BaudRate:%1\r\n").arg(hi.baudRate));
    params.append(QString("StopBits:%1\r\n").arg(hi.stopBits));
    params.append(QString("DataBits:%1\r\n").arg(hi.dataBits));
    params.append(QString("FlowControl:%1\r\n").arg(hi.flowContrl));
    params.append(QString("Parity:%1").arg(hi.parity));
    params.append("\r\n");
    parseError(params.toUtf8());
}

void QWoSerialWidgetImpl::onDisconnect()
{
    m_serial->close();
}

void QWoSerialWidgetImpl::onSendText(const QString &txt)
{
    m_serial->write(txt.toUtf8());
}

void QWoSerialWidgetImpl::onMoreReady()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete"));
        return;
    }
    QWoSessionProperty dlg(this);
    dlg.setSession(m_target);
    int ret = dlg.exec();
    if(ret == QWoSessionProperty::Save) {
        m_term->reloadProperty();
    }
}



void QWoSerialWidgetImpl::handleRead()
{
    QByteArray buf = m_serial->readAll();
    if(m_input->isAutoNewLine()) {
        parse("\r\n");
    }
    pasteFormat(buf);
}

void QWoSerialWidgetImpl::handleError()
{
    int err = m_serial->error();
    if(err == QSerialPort::NoError) {
        return;
    }
    QString sz = tr("An I/O error occurred from port %1, error: %2").arg(m_serial->portName()).arg(m_serial->errorString());
    parseError(sz.toUtf8());
    if(m_serial->isOpen()) {
        m_serial->close();
    }
}

void QWoSerialWidgetImpl::init()
{
    QList<int> sz;
    sz << height() - INPUT_MINI_HEIGHT << INPUT_MINI_HEIGHT;
    m_root->setSizes(sz);
}

QMap<QString, QString> QWoSerialWidgetImpl::collectUnsafeCloseMessage()
{
    QMap<QString, QString> all;

    return all;
}

void QWoSerialWidgetImpl::setTabText(const QString &title)
{
    for(int i = 0; i < m_tab->count(); i++){
        QVariant data = m_tab->tabData(i);
        QWoSerialWidgetImpl *impl = data.value<QWoSerialWidgetImpl*>();
        if(impl == this){
            m_tab->setTabText(i, title);
        }
    }
}

void QWoSerialWidgetImpl::parse(const QByteArray &data)
{
    QKxTermItem *term = m_term->termItem();
    term->parse(data);
}

void QWoSerialWidgetImpl::pasteFormat(const QByteArray &buf)
{
    if(!m_input->isTextMode()) {
        QByteArray data;
        QList<QByteArray> lines = buf.split('\n');
        for(int i = 0; i < lines.length(); i++) {
            data.append(hexPrettyFormat(lines.at(i)));
        }
        parse(data);
    }else{
        parse(buf);
    }
}

void QWoSerialWidgetImpl::parseError(const QByteArray &data)
{
    QKxTermItem *term = m_term->termItem();
    term->parseError(data);
}

QByteArray QWoSerialWidgetImpl::hexPrettyFormat(const QByteArray &buf)
{
    int i, j;
    int len = buf.length();
    int cnt = hexCount();
    QByteArray result;
    for(i=0; i < len; i += cnt) {
        result.append("   ") ;
        for(j=0; j<cnt && i+j<len; j++){
            char tmp[15] = {0};
            unsigned char c = buf[i+j];
            sprintf(tmp, "%2.2x ", c);
            result.append(tmp);
        }
        for(; j < cnt; ++j){
            result.append("   ");
        }
        result.append("  |") ;
        for(j=0; j < cnt && i+j<len; j++){
            unsigned char c = buf[i+j];
            result.append((c < 040 || c >= 0177) ? '.' : c ) ;
        }
        result.append("|\r\n") ;
    }
    return result;
}

int QWoSerialWidgetImpl::hexCount()
{
    QKxTermItem *term = m_term->termItem();
    QSize sz = term->termSize();
    return (sz.width() - 10) / 4;
}

QWoSerialTermWidget::QWoSerialTermWidget(const QString &target, QWidget *parent)
    : QWoTermWidget(target, 0, parent)
{

}

QWoSerialTermWidget::~QWoSerialTermWidget()
{

}

void QWoSerialTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    if(m_menu == nullptr) {
        m_menu = new QMenu(this);
        m_copy = m_menu->addAction(tr("Copy"));
        QObject::connect(m_copy, SIGNAL(triggered()), this, SLOT(onCopyToClipboard()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
    }
    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    //qDebug() << "selectText" << selTxt;
    m_copy->setDisabled(selTxt.isEmpty());
    m_menu->exec(QCursor::pos());
}

QList<QString> QWoSerialTermWidget::collectUnsafeCloseMessage()
{
    return QList<QString>();
}

void QWoSerialTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QWoSerialTermWidget::onModifyThisSession()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete ago"));
        return;
    }
    QWoSessionProperty dlg(this);
    dlg.setSession(m_target);
    int ret = dlg.exec();
    if(ret == QWoSessionProperty::Save) {
        initCustom();
    }
}
