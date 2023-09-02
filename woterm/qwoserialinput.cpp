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

#include "qwoserialinput.h"
#include "ui_qwoserialinput.h"
#include "qwosetting.h"
#include "qkxmessagebox.h"
#include "qwotermwidget.h"
#include "qkxtermitem.h"
#include "qwoserialinputhistorydialog.h"

#include <QSerialPortInfo>
#include <QStringListModel>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSerialPort>
#include <QNetworkDatagram>
#include <QTextCodec>
#include <QFile>
#include <QFileDialog>

// >202.201.120.120:65535|xx xx cc xx dd xx sskskskskkssksksk
// <202.201.120.120:65535|xx xx cc xx dd xx sskskskskkssksksk
#define NAME_SECTION_LENGTH     (23) // IP(15)+1+5
#define HEX_TEXT_DISTANCE       (3)

static const QStringList gcsType={"TCPServer", "TCPClient", "UDPServer", "UDPClient"};
QWoSerialInput::QWoSerialInput(QWoTermWidget *term, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoSerialInput)
    , m_term(term)
    , m_codec(nullptr)
    , m_timeLastSplit(0)
    , m_dlgExecInWriteData(false)
{
    ui->setupUi(this);
    QObject::connect(ui->btnMore, SIGNAL(clicked(bool)), this, SIGNAL(moreReady()));
    ui->btnConnect->setEnabled(true);
    setAttribute(Qt::WA_StyledBackground);

    setStyleSheet("QWoSerialInput{min-height: 700px}");
    QObject::connect(term, SIGNAL(sendData(QByteArray)), this, SLOT(onTermDataSend(QByteArray)));
    {
        QString hostLocal = QWoSetting::value("serialPort/hostTcpLocal", "0.0.0.0").toString();
        int portLocal = QWoSetting::value("serialPort/portTcpLocal", 2221).toInt();
        ui->hostTcpLocal->setText(hostLocal);
        ui->portTcpLocal->setText(QString::number(portLocal));
        ui->portTcpLocal->setValidator(new QIntValidator(0, 65535, this));
        QString hostRemote = QWoSetting::value("serialPort/hostTcpRemote", "127.0.0.1").toString();
        int portRemote = QWoSetting::value("serialPort/portTcpRemote", 2222).toInt();
        ui->hostTcpRemote->setText(hostRemote);
        ui->portTcpRemote->setText(QString::number(portRemote));
        ui->portTcpRemote->setValidator(new QIntValidator(0, 65535, this));

        ui->tcpClientBox->setModel(new QStringListModel(this));
        QObject::connect(ui->tcpClientBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onClientTcpCurrentTextChanged(QString)));
    }
    {
        QString hostLocal = QWoSetting::value("serialPort/hostUdpLocal", "0.0.0.0").toString();
        int portLocal = QWoSetting::value("serialPort/portUdpLocal", 2223).toInt();
        ui->hostUdpLocal->setText(hostLocal);
        ui->portUdpLocal->setText(QString::number(portLocal));
        ui->portUdpLocal->setValidator(new QIntValidator(0, 65535, this));
        QString hostRemote = QWoSetting::value("serialPort/hostUdpRemote", "127.0.0.1").toString();
        int portRemote = QWoSetting::value("serialPort/portUdpRemote", 2224).toInt();
        ui->hostUdpRemote->setText(hostRemote);
        ui->portUdpRemote->setText(QString::number(portRemote));
        ui->portUdpRemote->setValidator(new QIntValidator(0, 65535, this));
        ui->udpClientBox->setModel(new QStringListModel(this));
    }

    {
        QStringListModel *model = new QStringListModel(gcsType, this);
        ui->comx->setModel(model);
        ui->comx->setCurrentIndex(0);
        QObject::connect(ui->comx, SIGNAL(currentTextChanged(QString)), this, SLOT(onSerialPortCurrentTextChanged(QString)));
    }
    {
        QStringList rate;
        rate.append("1200");
        rate.append("2400");
        rate.append("4800");
        rate.append("9600");
        rate.append("19200");
        rate.append("38400");
        rate.append("57600");
        rate.append("115200");
        ui->baudRate->setModel(new QStringListModel(rate, this));
        QString txtSelected = QWoSetting::value("serialPort/baudRate", "9600").toString();
        ui->baudRate->setCurrentText(txtSelected);
    }
    {
        QStringList bits;
        bits.append("5");
        bits.append("6");
        bits.append("7");
        bits.append("8");
        ui->dataBits->setModel(new QStringListModel(bits, this));
        QString txtSelected = QWoSetting::value("serialPort/dataBits", "8").toString();
        ui->dataBits->setCurrentText(txtSelected);
    }
    {
        QStringList parity;
        parity.append("None");
        parity.append("Even");
        parity.append("Odd");
        parity.append("Space");
        parity.append("Mark");
        ui->parity->setModel(new QStringListModel(parity, this));
        QString txtSelected = QWoSetting::value("serialPort/parity", "None").toString();
        ui->parity->setCurrentText(txtSelected);
    }
    {
        QStringList bits;
        bits.append("1");
        bits.append("1.5");
        bits.append("2");
        ui->stopBits->setModel(new QStringListModel(bits, this));
        QString txtSelected = QWoSetting::value("serialPort/stopBits", "1").toString();
        ui->stopBits->setCurrentText(txtSelected);
    }
    {
        QStringList flow;
        flow.append("None");
        flow.append("RTS/CTS");
        flow.append("XON/XOFF");
        ui->flowControl->setModel(new QStringListModel(flow, this));
        QString txtSelected = QWoSetting::value("serialPort/flowControl", "None").toString();
        ui->flowControl->setCurrentText(txtSelected);
    }

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onSerialPortRefresh()));
    timer->start(1000);
    onSerialPortRefresh();
    QString txtSelected = ui->comx->currentText();
    onSerialPortCurrentTextChanged(txtSelected);

    QObject::connect(ui->btnTcpListen, SIGNAL(clicked()), this, SLOT(onTcpListenButtonClicked()));
    QObject::connect(ui->btnUdpListen, SIGNAL(clicked()), this, SLOT(onUdpListenButtonClicked()));
    QObject::connect(ui->btnTcpConnect, SIGNAL(clicked()), this, SLOT(onTcpConnectButtonClicked()));
    QObject::connect(ui->btnUdpStart, SIGNAL(clicked()), this, SLOT(onUdpStartButtonClicked()));
    QObject::connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(onComxConnectButtonClicked()));
    QObject::connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(onTextSendButtonClicked()));    
    QObject::connect(ui->edit, SIGNAL(textChanged()), this, SLOT(onPlainEditTextChanged()));
    QObject::connect(ui->btnClear, SIGNAL(clicked()), ui->edit, SLOT(clear()));

    ui->tcpServerTip->setVisible(false);
    ui->udpServerTip->setVisible(false);
    ui->tcpClientTip->setVisible(false);
    ui->udpClientTip->setVisible(false);    

    {

        QStringList items;
        items.append(tr("Print string"));
        items.append(tr("Hex string"));
        ui->modeInput->setModel(new QStringListModel(items, ui->modeInput));
        int type = QWoSetting::value("serialPort/inputType", 0).toInt();
        ui->modeInput->setCurrentIndex(type);
        QObject::connect(ui->modeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeInputIndexChanged(int)));
        onModeInputIndexChanged(ui->modeInput->currentIndex());

        bool yes = QWoSetting::value("serialPort/checkEchoInput", true).toBool();
        ui->chkEchoInput->setChecked(yes);
        QObject::connect(ui->chkEchoInput, &QCheckBox::clicked, this, [=](){
            QWoSetting::setValue("serialPort/checkEchoInput", ui->chkEchoInput->isChecked());
        });

        bool checked = QWoSetting::value("serialPort/checkInputSuffix", true).toBool();
        ui->chkInSuffix->setChecked(checked);
        QObject::connect(ui->chkInSuffix, &QCheckBox::clicked, this, [=](){
            QWoSetting::setValue("serialPort/checkInputSuffix", ui->chkInSuffix->isChecked());
        });

        QString suffix = QWoSetting::value("serialPort/inputSuffix").toString();
        ui->hexInSuffix->setText(suffix);
        QObject::connect(ui->hexInSuffix, &QLineEdit::editingFinished, this, [=](){
            QWoSetting::setValue("serialPort/inputSuffix", ui->hexInSuffix->text());
        });

        QString msg = QWoSetting::value("serialPort/lastInput").toString();
        ui->edit->setPlainText(msg);
        QObject::connect(ui->edit, &QPlainTextEdit::textChanged, this, [=](){
            QWoSetting::setValue("serialPort/lastInput", ui->edit->toPlainText());
        });
    }
    {
        QStringList items;
        items.append(tr("No filter"));
        items.append(tr("Split by timeout"));
        items.append(tr("Split by hex string"));
        ui->modeSplit->setModel(new QStringListModel(items, ui->modeSplit));
        int type = QWoSetting::value("serialPort/splitType", 0).toInt();
        ui->modeSplit->setCurrentIndex(type);
        QObject::connect(ui->modeSplit, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeSplitIndexChanged(int)));
        onModeSplitIndexChanged(ui->modeSplit->currentIndex());

        int ms = QWoSetting::value("serialPort/splitInterval", 300).toInt();
        ui->splitInterval->setText(QString("%1").arg(ms));
        ui->splitInterval->setValidator(new QIntValidator(1, 1000, ui->splitInterval));

        QObject::connect(ui->splitInterval, &QLineEdit::editingFinished, this, [=](){
            QWoSetting::setValue("serialPort/splitInterval", ui->splitInterval->text());
        });

        QString special = QWoSetting::value("serialPort/splitChars", "0D0A").toString();
        ui->splitChars->setText(special);

        QObject::connect(ui->splitChars, &QLineEdit::editingFinished, this, [=](){
            QWoSetting::setValue("serialPort/splitChars", ui->splitChars->text());
        });
    }

    {
        QStringList items;
        items.append(tr("No filter"));
        items.append(tr("Latin1 hex string"));
        items.append(tr("Unicode hex string"));
        ui->modeOutput->setModel(new QStringListModel(items, ui->modeOutput));
        int type = QWoSetting::value("serialPort/outputType", 0).toInt();
        ui->modeOutput->setCurrentIndex(type);
        QObject::connect(ui->modeOutput, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeOutputIndexChanged(int)));
        onModeOutputIndexChanged(ui->modeOutput->currentIndex());

        bool checked = QWoSetting::value("serialPort/checkDataSource", true).toBool();
        ui->chkDataSource->setChecked(checked);
        QObject::connect(ui->chkDataSource, &QCheckBox::clicked, this, [=](){
            QWoSetting::setValue("serialPort/checkDataSource", ui->chkDataSource->isChecked());
        });

    }
    {
        bool checked = QWoSetting::value("serialPort/chkTermInteractive", false).toBool();
        m_term->setReadOnly(!checked);
        ui->chkTermInteractive->setChecked(checked);
        QObject::connect(ui->chkTermInteractive, &QCheckBox::clicked, this, [=](){
            QWoSetting::setValue("serialPort/chkTermInteractive", ui->chkTermInteractive->isChecked());
            m_term->setReadOnly(!ui->chkTermInteractive->isChecked());
        });

        QObject::connect((QWoTermWidget*)m_term, &QWoTermWidget::readOnlyChanged, this, [=](){
            ui->chkTermInteractive->setChecked(!m_term->readOnly());
        });
    }
    {
        QObject::connect(ui->btnSendFile, SIGNAL(clicked()), this, SLOT(onButtonSendFileClicked()));
    }

    ui->edit->installEventFilter(this);

    m_outTimer = new QTimer(this);
    QObject::connect(m_outTimer, SIGNAL(timeout()), this, SLOT(onOutputTimeout()));
    m_outTimer->start(10);


    QObject::connect(ui->btnMsgLibrary, SIGNAL(clicked()), this, SLOT(onMsgLibraryButtonClicked()));

#ifdef QT_DEBUG2
    ui->simulateArea->show();
    QObject::connect(ui->btnSimulateSend, SIGNAL(clicked()), this, SLOT(onSimulateSendButtonClicked()));
    {
        QString txt = QWoSetting::value("serialPort/simulateEdit").toString();
        ui->simulateEdit->setPlainText(txt);
        QObject::connect(ui->simulateEdit, &QPlainTextEdit::textChanged, this, [=](){
            QWoSetting::setValue("serialPort/simulateEdit", ui->simulateEdit->toPlainText());
        });
    }
    {
        QString txt = QWoSetting::value("serialPort/editText").toString();
        ui->edit->setPlainText(txt);
        QObject::connect(ui->edit, &QPlainTextEdit::textChanged, this, [=](){
            QWoSetting::setValue("serialPort/editText", ui->edit->toPlainText());
        });
    }
#else
    ui->simulateArea->hide();
    ui->simulateArea->deleteLater();
#endif
}

QWoSerialInput::~QWoSerialInput()
{
    delete ui;
}

void QWoSerialInput::reset()
{
    ui->btnConnect->setEnabled(true);
}

bool QWoSerialInput::isConnected()
{
    QString portName = ui->comx->currentText();
    if(portName == "TCPServer") {
        if(m_tcpClient) {
            return true;
        }
    }else if(portName == "TCPClient") {
        if(m_tcpConnect) {
            return true;
        }
    }else if(portName == "UDPServer") {
        if(m_udpServer) {
            return true;
        }
    }else if(portName == "UDPClient") {
        if(m_udpConnect) {
            return true;
        }
    }else{
        // comx.
        if(m_serialPort){
            return true;
        }
    }
    return false;
}

void QWoSerialInput::onSerialPortRefresh()
{
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    QList<QString> coms = gcsType;
    for(int i = 0; i < infos.length(); i++) {
        coms.append(infos.at(i).portName());
    }

    QString txtSelected = ui->comx->currentText();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->comx->model());
    QStringList old = model->stringList();
    if(old != coms) {
        model->setStringList(coms);
        if(coms.contains(txtSelected)) {
            ui->comx->setCurrentText(txtSelected);
        }else{
            ui->comx->setCurrentIndex(0);
        }
    }
}

void QWoSerialInput::onServerTcpReadyRead()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
    QByteArray all = sock->readAll();
    QString who = socketName(sock);
    handleDataRecv(who, all);    
}

void QWoSerialInput::onServerTcpCleanup()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
    m_tcpClients.removeOne(sock);

    refleshTcpClients();
}

void QWoSerialInput::onClientTcpReadyRead()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
    QByteArray all = sock->readAll();
    QString who = socketName(sock);
    handleDataRecv(who, all);
}

void QWoSerialInput::onClientTcpCleanup()
{
    if(m_tcpConnect) {
        QString errMsg = m_tcpConnect->errorString();
        ui->tcpClientTip->setText(errMsg);
        ui->tcpClientTip->setVisible(!errMsg.isEmpty());
        m_tcpConnect->close();
        m_tcpConnect->deleteLater();
        m_tcpConnect = nullptr;

        ui->btnTcpConnect->setText(tr("Connect"));
    }
}

void QWoSerialInput::onClientTcpCurrentTextChanged(const QString &name)
{
    for(auto it = m_tcpClients.begin(); it != m_tcpClients.end(); it++) {
        QTcpSocket *sock = *it;
        QString nameHit = socketName(sock);
        if(nameHit == name) {
            m_tcpClient = sock;
            return;
        }
    }
}

void QWoSerialInput::onServerUdpReadyRead()
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    QStringList hps;
    while(m_udpServer->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpServer->receiveDatagram();
        QByteArray data = datagram.data();
        QHostAddress host = datagram.senderAddress();
        int port = datagram.senderPort();
        QString hp = host.toString() + ":" + QString::number(port);
        if(!hps.contains(hp)) {
            hps.append(hp);
        }
        handleDataRecv(hp, data);
        m_udpActived.insert(hp, now);
    }

    onServerUdpCleanup();
}

void QWoSerialInput::onClientUdpReadyRead()
{
    QString host = ui->hostUdpRemote->text();
    QString port = ui->portUdpRemote->text();
    while(m_udpConnect->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpConnect->receiveDatagram();
        QByteArray data = datagram.data();
        QString hp = host + ":" + port;
        handleDataRecv(hp, data);
    }
}

void QWoSerialInput::onServerUdpCleanup()
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    for(auto it = m_udpActived.begin(); it != m_udpActived.end(); ) {
        QString hp = it.key();
        qint64 tmLast = it.value();
        if(now - tmLast > 90) {
            it = m_udpActived.erase(it);
        }else{
            it++;
        }
    }
    QStringList all = m_udpActived.keys();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->udpClientBox->model());
    QStringList lsOld = model->stringList();
    QString txtSelected = ui->udpClientBox->currentText();
    if(lsOld != all) {
        model->setStringList(all);
        if(all.contains(txtSelected)) {
            ui->udpClientBox->setCurrentText(txtSelected);
        }else{
            ui->udpClientBox->setCurrentIndex(0);
        }
    }
}

void QWoSerialInput::onComxReadyRead()
{
    QByteArray all = m_serialPort->readAll();
    QString who = m_serialPort->portName();
    handleDataRecv(who, all);
}

void QWoSerialInput::onDeviceBytesWritten(qint64 bytes)
{
    if(!m_fileSendBuffer.isEmpty()) {
        int cnt = writeComxData(m_fileSendBuffer, false);
        if(cnt < 0) {
            QKxMessageBox::warning(this, tr("File send error"), tr("Abnormal error, terminating sending."));
            m_fileSendBuffer.clear();
        }else{
            m_fileSendBuffer = m_fileSendBuffer.mid(cnt);
        }
    }
}

void QWoSerialInput::onComxError()
{
    QString msg = m_serialPort->errorString();
    ui->comxTip->setVisible(!msg.isEmpty());
    ui->comxTip->setText(msg);
}

void QWoSerialInput::onOutputTimeout()
{
    QList<LineOutput> lines;
    handleSplitFilter(lines);
    handleOutputFilter(leftArrow(), lines);
}

void QWoSerialInput::onTermDataSend(const QByteArray &data)
{
    if(!ui->chkTermInteractive->isChecked()) {
        return;
    }
    if(!m_fileSendBuffer.isEmpty()) {
        QKxMessageBox::information(this, tr("Error"), tr("The current file content has not been sent yet."));
        return;
    }
    writeComxData(data);
}

void QWoSerialInput::onServerTcpNewConnection()
{
    while(m_tcpServer->hasPendingConnections()) {
        QTcpSocket *client = m_tcpServer->nextPendingConnection();
        QObject::connect(client, SIGNAL(readyRead()), this, SLOT(onServerTcpReadyRead()));
        QObject::connect(client, SIGNAL(bytesWritten(qint64)), this, SLOT(onDeviceBytesWritten(qint64)));
        QObject::connect(client, SIGNAL(disconnected()), this, SLOT(onServerTcpCleanup()));
        QObject::connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onServerTcpCleanup()));
        m_tcpClients.append(client);
    }

    refleshTcpClients();
}

void QWoSerialInput::onTcpListenButtonClicked()
{
    ui->tcpServerTip->setVisible(false);
    QString txt = ui->btnTcpListen->text();
    if(txt == tr("Stop")) {
        if(handleTcpListen(false)) {
            ui->btnTcpListen->setText(tr("Listen"));
        }
    }else{
        if(handleTcpListen(true)){
            ui->btnTcpListen->setText(tr("Stop"));
        }
    }
    m_fileSendBuffer.clear();
}

void QWoSerialInput::onTcpConnectButtonClicked()
{
    ui->tcpClientTip->setVisible(false);
    QString txt = ui->btnTcpConnect->text();
    if(txt == tr("Stop")) {
        if(handleTcpConnect(false)){
            ui->btnTcpConnect->setText(tr("Connect"));
        }
    }else{
        if(handleTcpConnect(true)){
            ui->btnTcpConnect->setText(tr("Stop"));
        }
    }
    m_fileSendBuffer.clear();
}

void QWoSerialInput::onUdpListenButtonClicked()
{
    ui->udpServerTip->setVisible(false);
    QString txt = ui->btnUdpListen->text();
    if(txt == tr("Stop")) {
        if(handleUdpListen(false)){
            ui->btnUdpListen->setText(tr("Listen"));
        }
    }else{
        if(handleUdpListen(true)) {
            ui->btnUdpListen->setText(tr("Stop"));
        }
    }
    m_fileSendBuffer.clear();
}

void QWoSerialInput::onUdpStartButtonClicked()
{
    ui->udpClientTip->setVisible(false);
    QString txt = ui->btnUdpStart->text();
    if(txt == tr("Stop")) {
        if(handleUdpConnect(false)){
            ui->btnUdpStart->setText(tr("Start"));
        }
    }else{
        if(handleUdpConnect(true)){
            ui->btnUdpStart->setText(tr("Stop"));
        }
    }
    m_fileSendBuffer.clear();
}

void QWoSerialInput::onComxConnectButtonClicked()
{
    ui->comxTip->setVisible(false);
    QString txt = ui->btnConnect->text();
    if(txt == tr("Stop")) {
        if(handleComxConnect(false)){
            ui->btnConnect->setText(tr("Connect"));
        }
    }else{
        if(handleComxConnect(true)){
            ui->btnConnect->setText(tr("Stop"));
        }
    }
    m_fileSendBuffer.clear();
}

void QWoSerialInput::onTextSendButtonClicked()
{
    QString msg = ui->edit->toPlainText();

    if(!m_fileSendBuffer.isEmpty()) {
        QKxMessageBox::information(this, tr("Error"), tr("The current file content has not been sent yet."));
        return;
    }

    recheckCodePage();

    QByteArray data = m_codec->fromUnicode(msg);
    if(ui->modeInput->currentIndex() == 1) {
        if(!isHexString(data)) {
            QKxMessageBox::information(this, tr("Error"), tr("make sure all string are hex code."));
            return;
        }
        QByteArray result = data.replace(QChar::Space, QByteArray());
        if((result.length() % 2) == 1) {
            QKxMessageBox::information(this, tr("Error"), tr("the hex code is not completed."));
            return;
        }
        data = QByteArray::fromHex(result);
    }


    QByteArray suffix;
    if(ui->chkInSuffix->isChecked()){
        suffix = ui->hexInSuffix->text().toLatin1();
        if(suffix.isEmpty()) {
            QKxMessageBox::information(this, tr("Suffix information"), tr("The input suffix string should not be empty."));
            return;
        }
        if(!isHexString(suffix)) {
            QKxMessageBox::information(this, tr("Suffix information"), tr("The input suffix string must be hex string."));
            return;
        }
        suffix = QByteArray::fromHex(suffix.replace(" ", ""));
    }
    data.append(suffix);

    writeComxData(data);
}

void QWoSerialInput::onButtonSendFileClicked()
{
    int type = QKxMessageBox::warning(this, tr("File sending information"), tr("Sending files is a dangerous behavior. Please carefully confirm whether the content of the file will damage the target device or cause other unpredictable events. Do you want to continue the operation."), QMessageBox::Yes|QMessageBox::No);
    if(type != QMessageBox::Yes) {
        return;
    }
    QString pathLast = QWoSetting::value("serialPort/lastFilePath", QDir::homePath()).toString();
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Please select a file to send."), pathLast);
    if(pathFile.isEmpty()) {
        return;
    }
    QWoSetting::setValue("serialPort/lastFilePath", pathFile);
    QFile file(pathFile);
    if(!file.open(QFile::ReadOnly)) {
        QKxMessageBox::warning(this, tr("File open error"), tr("Failed to open file:")+pathFile);
        return;
    }
    if(file.size() > 1024 * 1024 * 5) {
        QKxMessageBox::warning(this, tr("File size limitation"), tr("The size of the sent file cannot exceed 5M bytes."));
        return;
    }
    m_fileSendBuffer = file.readAll();
    int cnt = writeComxData(m_fileSendBuffer, false);
    if(cnt < 0) {
        QKxMessageBox::warning(this, tr("File send error"), tr("Unknow error and terminat the file sending."));
        m_fileSendBuffer.clear();
    }else{
        m_fileSendBuffer = m_fileSendBuffer.mid(cnt);
    }
}


void QWoSerialInput::onSerialPortCurrentTextChanged(const QString &portName)
{
    if(portName == "TCPServer") {
        ui->tcpServerArea->show();
        ui->tcpClientArea->hide();
        ui->udpServerArea->hide();
        ui->udpClientArea->hide();
        ui->comxArea->hide();
    }else if(portName == "TCPClient") {
        ui->tcpServerArea->hide();
        ui->tcpClientArea->show();
        ui->udpServerArea->hide();
        ui->udpClientArea->hide();
        ui->comxArea->hide();
    }else if(portName == "UDPServer") {
        ui->tcpServerArea->hide();
        ui->tcpClientArea->hide();
        ui->udpServerArea->show();
        ui->udpClientArea->hide();
        ui->comxArea->hide();
    }else if(portName == "UDPClient") {
        ui->tcpServerArea->hide();
        ui->tcpClientArea->hide();
        ui->udpServerArea->hide();
        ui->udpClientArea->show();
        ui->comxArea->hide();
    }else{
        // comx.
        ui->tcpServerArea->hide();
        ui->tcpClientArea->hide();
        ui->udpServerArea->hide();
        ui->udpClientArea->hide();
        ui->comxArea->show();
    }
}

void QWoSerialInput::onPlainEditTextChanged()
{
    if(ui->modeInput->currentIndex() != 1) {
        return;
    }
    QTextCursor tc = ui->edit->textCursor();
    int pos = tc.position();
    QString txtOld = ui->edit->toPlainText();
    QString txtBefore = txtOld.left(pos);
    QString txtNew = formatHexString(txtOld);
    if(txtNew.simplified() == txtOld.simplified()) {
        return;
    }
    QString txtBeforeNow = formatHexString(txtBefore);
    ui->edit->setPlainText(txtNew);
    QTextCursor tc2 = ui->edit->textCursor();
    tc2.setPosition(txtBeforeNow.length());
    ui->edit->setTextCursor(tc2);
}

void QWoSerialInput::onModeInputIndexChanged(int idx)
{
    QWoSetting::setValue("serialPort/inputType", idx);

    QString txt =  ui->edit->toPlainText();
    if(txt.isEmpty()) {
        return;
    }

    recheckCodePage();

    if(idx == 0) {
        // print string.
        QByteArray hex = txt.toLatin1();
        if(!isHexString(hex)) {
            ui->edit->clear();
            return;
        }
        hex = hex.replace(" ", "");
        QByteArray codeTxt = QByteArray::fromHex(hex);
        QString uTxt = m_codec->toUnicode(codeTxt);
        ui->edit->setPlainText(uTxt);
    }else{
        // hex.
        QByteArray codeTxt = m_codec->fromUnicode(txt);
        QByteArray hex = codeTxt.toHex();
        QString uHex = formatHexString(hex);
        ui->edit->setPlainText(uHex);
    }
}

void QWoSerialInput::onModeSplitIndexChanged(int idx)
{
    QWoSetting::setValue("serialPort/splitType", idx);
    if(idx == 0) {
        ui->splitCharsArea->hide();
        ui->splitIntervalArea->hide();
    }else if(idx == 1){
        // timeout
        ui->splitCharsArea->hide();
        ui->splitIntervalArea->show();
    }else{
        ui->splitCharsArea->show();
        ui->splitIntervalArea->hide();
    }
    m_timeLastSplit = 0;
}

void QWoSerialInput::onModeOutputIndexChanged(int idx)
{
    QWoSetting::setValue("serialPort/outputType", idx);
    m_whoBufferLeft.clear();
    ui->chkHexOutput->setVisible(idx != 0);
}

void QWoSerialInput::onSimulateSendButtonClicked()
{
    QString msg = ui->simulateEdit->toPlainText();
    recheckCodePage();
    QByteArray data = m_codec->fromUnicode(msg);
    QByteArray suffix;
    if(ui->chkInSuffix->isChecked()){
        suffix = ui->hexInSuffix->text().toLatin1();
        if(suffix.isEmpty()) {
            QKxMessageBox::information(this, tr("Suffix information"), tr("The input suffix string should not be empty."));
            return;
        }
        if(!isHexString(suffix)) {
            QKxMessageBox::information(this, tr("Suffix information"), tr("The input suffix string must be hex string."));
            return;
        }
        suffix = QByteArray::fromHex(suffix.replace(" ", ""));
    }
    data.append(suffix);
    handleDataRecv("simulate", data);
}

void QWoSerialInput::onMsgLibraryButtonClicked()
{
    QString txt = ui->edit->toPlainText();
    QString msg = QString::number(ui->modeInput->currentIndex()) + ":" + txt;
    QWoSerialInputHistoryDialog dlg(msg, this);
    QObject::connect(&dlg, &QWoSerialInputHistoryDialog::messageArrived, this, [=](const QString& msg){
        QString c = msg.at(0);
        int idx = c.toInt();
        if(idx == 0 || idx == 1) {
            ui->modeInput->setCurrentIndex(idx);
        }
        ui->edit->setPlainText(msg.mid(2));
    });
    dlg.exec();
}

bool QWoSerialInput::handleTcpListen(bool start)
{
    ui->tcpServerTip->setVisible(false);
    if(start) {
        QString host = ui->hostTcpLocal->text();
        QString port = ui->portTcpLocal->text();
        if(host.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Host parameter should not be empty."));
            return false;
        }
        if(port.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not be empty."));
            return false;
        }
        int iport = port.toInt();
        if(iport < 0 || iport >65535) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not range between 0 and 65535."));
            return false;
        }

        QWoSetting::setValue("serialPort/hostTcpLocal", host);
        QWoSetting::setValue("serialPort/portTcpLocal", iport);

        m_tcpServer = new QTcpServer(this);
        QObject::connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onServerTcpNewConnection()));
        if(!m_tcpServer->listen(QHostAddress(host), quint16(iport))) {
            QString szError = m_tcpServer->errorString();
            QKxMessageBox::information(this, tr("Listen errors"), tr("Failed to listen for errors") + ":" + szError);
            m_tcpServer->deleteLater();
            return false;
        }
        return true;
    }else{
        if(m_tcpServer) {
            m_tcpServer->close();
            m_tcpServer->deleteLater();
        }
        QList<QPointer<QTcpSocket>> all = m_tcpClients;
        m_tcpClients.clear();
        for(auto it = all.begin(); it != all.end(); it++) {
            QTcpSocket *cli = *it;
            if(cli) {
                cli->close();
                cli->deleteLater();
            }
        }
    }
    return true;
}

bool QWoSerialInput::handleTcpConnect(bool start)
{
    ui->tcpClientTip->setVisible(false);
    if(start) {
        if(m_tcpConnect) {
            m_tcpConnect->close();
            m_tcpConnect->deleteLater();
        }
        QString host = ui->hostTcpRemote->text();
        QString port = ui->portTcpRemote->text();
        if(host.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Host parameter should not be empty."));
            return false;
        }
        if(port.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not be empty."));
            return false;
        }
        int iport = port.toInt();
        if(iport < 0 || iport >65535) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not range between 0 and 65535."));
            return false;
        }

        QWoSetting::setValue("serialPort/hostTcpRemote", host);
        QWoSetting::setValue("serialPort/portTcpRemote", iport);

        m_tcpConnect = new QTcpSocket(this);
        QObject::connect(m_tcpConnect, SIGNAL(readyRead()), this, SLOT(onClientTcpReadyRead()));
        QObject::connect(m_tcpConnect, SIGNAL(bytesWritten(qint64)), this, SLOT(onDeviceBytesWritten(qint64)));
        QObject::connect(m_tcpConnect, SIGNAL(disconnected()), this, SLOT(onClientTcpCleanup()));
        QObject::connect(m_tcpConnect, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onClientTcpCleanup()));
        m_tcpConnect->connectToHost(host, quint16(iport));
        return true;
    }else{
        if(m_tcpConnect) {
            m_tcpConnect->close();
            m_tcpConnect->deleteLater();
        }
    }
    return true;
}


void QWoSerialInput::refleshTcpClients()
{
    QStringList clients;
    for(auto it = m_tcpClients.begin(); it != m_tcpClients.end(); it++) {
        QTcpSocket *client = *it;
        QString who = socketName(client);
        clients.append(who);
    }
    QString txtSelected = ui->tcpClientBox->currentText();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->tcpClientBox->model());
    QStringList lsOld = model->stringList();
    model->setStringList(clients);
    if(clients.contains(txtSelected)) {
        ui->tcpClientBox->setCurrentText(txtSelected);
    }else if(!clients.isEmpty()){
        ui->tcpClientBox->setCurrentIndex(0);
    }
}

bool QWoSerialInput::handleUdpListen(bool start)
{
    ui->udpServerTip->setVisible(false);
    if(start) {
        QString host = ui->hostUdpLocal->text();
        QString port = ui->portUdpLocal->text();
        if(host.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Host parameter should not be empty."));
            return false;
        }
        if(port.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not be empty."));
            return false;
        }
        int iport = port.toInt();
        if(iport < 0 || iport >65535) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not range between 0 and 65535."));
            return false;
        }

        QWoSetting::setValue("serialPort/hostUdpLocal", host);
        QWoSetting::setValue("serialPort/portUdpLocal", iport);

        if(m_udpServer) {
            m_udpServer->close();
            m_udpServer->deleteLater();
        }
        if(m_udpTimer) {
            m_udpTimer->stop();
            m_udpTimer->deleteLater();
        }
        m_udpTimer = new QTimer(this);
        QObject::connect(m_udpTimer, SIGNAL(timeout()), this, SLOT(onServerUdpCleanup()));
        m_udpTimer->start(10000);
        m_udpServer = new QUdpSocket(this);
        QObject::connect(m_udpServer, SIGNAL(readyRead()), this, SLOT(onServerUdpReadyRead()));
        QObject::connect(m_udpServer, SIGNAL(bytesWritten(qint64)), this, SLOT(onDeviceBytesWritten(qint64)));
        if(!m_udpServer->bind(QHostAddress(host), quint16(iport))) {
            QString szError = m_udpServer->errorString();
            QKxMessageBox::information(this, tr("Listen errors"), tr("Failed to listen for errors") + ":" + szError);
            m_udpServer->deleteLater();
            return false;
        }
        return true;
    }else{
        if(m_udpServer) {
            m_udpServer->close();
            m_udpServer->deleteLater();
        }
        if(m_udpTimer) {
            m_udpTimer->stop();
            m_udpTimer->deleteLater();
        }
        ui->udpClientBox->clear();
    }
    return true;
}

bool QWoSerialInput::handleUdpConnect(bool start)
{
    ui->udpClientTip->setVisible(false);
    if(start) {
        if(m_udpConnect) {
            m_udpConnect->close();
            m_udpConnect->deleteLater();
        }
        QString host = ui->hostUdpRemote->text();
        QString port = ui->portUdpRemote->text();
        if(host.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Host parameter should not be empty."));
            return false;
        }
        if(port.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not be empty."));
            return false;
        }
        int iport = port.toInt();
        if(iport < 0 || iport >65535) {
            QKxMessageBox::information(this, tr("Parameter errors"), tr("Port parameter should not range between 0 and 65535."));
            return false;
        }

        QWoSetting::setValue("serialPort/hostUdpRemote", host);
        QWoSetting::setValue("serialPort/portUdpRemote", iport);

        m_udpConnect = new QUdpSocket(this);
        QObject::connect(m_udpConnect, SIGNAL(readyRead()), this, SLOT(onClientUdpReadyRead()));
        QObject::connect(m_udpConnect, SIGNAL(bytesWritten(qint64)), this, SLOT(onDeviceBytesWritten(qint64)));
        m_udpConnect->bind(QHostAddress(QHostAddress::Any), 0);

        return true;
    }else{
        if(m_udpConnect) {
            m_udpConnect->close();
            m_udpConnect->deleteLater();
        }
    }
    return true;
}

bool QWoSerialInput::handleComxConnect(bool start)
{
    ui->comxTip->setVisible(false);
    if(start) {
        QString target = ui->comx->currentText();
        QString baudRate = ui->baudRate->currentText();
        QString stopBits = ui->stopBits->currentText();
        QString dataBits = ui->dataBits->currentText();
        QString flowControl = ui->flowControl->currentText();
        QString parity = ui->parity->currentText();

        QWoSetting::setValue("serialPort/baudRate", baudRate);
        QWoSetting::setValue("serialPort/dataBits", dataBits);
        QWoSetting::setValue("serialPort/parity", parity);
        QWoSetting::setValue("serialPort/stopBits", stopBits);
        QWoSetting::setValue("serialPort/flowControl", flowControl);

        if(m_serialPort) {
            m_serialPort->close();
            m_serialPort->deleteLater();
        }
        m_serialPort = new QSerialPort(this);
        QObject::connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(onComxReadyRead()));
        QObject::connect(m_serialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(onDeviceBytesWritten(qint64)));
        QObject::connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onComxError()));
        m_serialPort->setPortName(target);
        m_serialPort->setBaudRate(baudRate.toInt());
        if(stopBits == "2") {
            m_serialPort->setStopBits(QSerialPort::TwoStop);
        }else if(stopBits == "1.5") {
            m_serialPort->setStopBits(QSerialPort::OneAndHalfStop);
        }else{
            m_serialPort->setStopBits(QSerialPort::OneStop);
        }
        if(dataBits == "5") {
            m_serialPort->setDataBits(QSerialPort::Data5);
        }else if(dataBits == "6") {
            m_serialPort->setDataBits(QSerialPort::Data6);
        }else if(dataBits == "7") {
            m_serialPort->setDataBits(QSerialPort::Data7);
        }else {
            m_serialPort->setDataBits(QSerialPort::Data8);
        }

        if(flowControl == "RTS/CTS") {
            m_serialPort->setFlowControl(QSerialPort::HardwareControl);
        }else if(flowControl == "XON/XOFF") {
            m_serialPort->setFlowControl(QSerialPort::SoftwareControl);
        }else {
            m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        }

        if(parity == "Even") {
            m_serialPort->setParity(QSerialPort::EvenParity);
        }else if(parity == "Odd") {
            m_serialPort->setParity(QSerialPort::OddParity);
        }else if(parity == "Space") {
            m_serialPort->setParity(QSerialPort::SpaceParity);
        }else if(parity == "Mark") {
            m_serialPort->setParity(QSerialPort::MarkParity);
        }else{
            m_serialPort->setParity(QSerialPort::NoParity);
        }
        if(!m_serialPort->open(QIODevice::ReadWrite)) {
            QKxMessageBox::warning(this, tr("Error"), tr("Failed to open device."));
            return false;
        }
        return true;
    }else{
        if(m_serialPort) {
            m_serialPort->close();
            m_serialPort->deleteLater();
        }
    }
    return true;
}

QString QWoSerialInput::socketName(const QAbstractSocket *socket) const
{
    QHostAddress host = socket->peerAddress();
    int port = socket->peerPort();
    return host.toString() + ":" + QString::number(port);
}

void QWoSerialInput::handleDataRecv(const QString &who, const QByteArray &buf)
{
    if(buf.isEmpty()) {
        return;
    }

    TimeOutput out = m_output.take(who);
    out.tmLast = QDateTime::currentMSecsSinceEpoch();
    out.buf.append(buf);
    m_output.insert(who, out);

    int idx = ui->modeSplit->currentIndex();
    if(idx == 0) {
        onOutputTimeout();
    }
}

void QWoSerialInput::handleDataSend(const QString &who, const QByteArray &buf)
{
    if(!ui->chkEchoInput->isChecked()) {
        return;
    }
    LineOutput lo;
    lo.who = who.toUtf8();
    lo.lines.append(buf);
    QList<LineOutput> lines;
    lines.append(lo);
    handleOutputFilter(rightArrow(), lines);
}

int QWoSerialInput::writeComxData(const QByteArray &data, bool tryEcho)
{
    QString portName = ui->comx->currentText();
    int nWrite = -1;
    if(portName == "TCPServer") {
        if(m_tcpClient) {
            nWrite = m_tcpClient->write(data);
            if(tryEcho) {
                handleDataSend(socketName(m_tcpClient), data);
            }
        }
    }else if(portName == "TCPClient") {
        if(m_tcpConnect) {
            nWrite = m_tcpConnect->write(data);
            if(tryEcho) {
                handleDataSend(socketName(m_tcpConnect), data);
            }
        }
    }else if(portName == "UDPServer") {
        if(m_udpServer) {
            QString hostIPName = ui->udpClientBox->currentText();
            if(hostIPName.isEmpty()) {
                return -1;
            }
            QStringList hin = hostIPName.split(':');
            QString host = hin.at(0);
            QString port = hin.at(1);
            int iport = port.toInt();
            nWrite = m_udpServer->writeDatagram(data, QHostAddress(host), quint16(iport));
            if(tryEcho) {
                handleDataSend(QString("%1:%2").arg(host).arg(port), data);
            }
        }
    }else if(portName == "UDPClient") {
        if(m_udpConnect) {
            QString host = ui->hostUdpRemote->text();
            QString port = ui->portUdpRemote->text();
            int iport = port.toInt();
            nWrite = m_udpConnect->writeDatagram(data, QHostAddress(host), quint16(iport));
            if(tryEcho) {
                handleDataSend(QString("%1:%2").arg(host).arg(port), data);
            }
        }
    }else{
        // comx.
        if(m_serialPort){
            nWrite = m_serialPort->write(data);
            QString name = m_serialPort->portName();
            if(tryEcho) {
                handleDataSend(name, data);
            }
        }
    }

    if(nWrite < 0 && !m_dlgExecInWriteData) {
        m_dlgExecInWriteData = true;
        QKxMessageBox::information(this, tr("Connection error"), tr("Please confirm if there is currently a valid device connection."));
        m_dlgExecInWriteData = false;
    }

    return nWrite;
}

QByteArray QWoSerialInput::formatHexText(const QByteArray &buf)
{
    QList<QByteArray> lines;
    int i, j;
    int len = buf.length();
    int cnt = hexModeCharCount();

    for(i=0; i < len; i += cnt) {
        QByteArray line;
        for(j=0; j<cnt && i+j<len; j++){
            char tmp[15] = {0};
            unsigned char c = buf[i+j];
            sprintf(tmp, "%2.2x ", c);
            line.append(tmp);
        }
        line = line.toUpper();
        for(; j < cnt; ++j){
            line.append("   ");
        }
        if(!ui->chkHexOutput->isChecked()) {
            line.append("  |") ;
            for(j=0; j < cnt && i+j<len; j++){
                unsigned char c = buf[i+j];
                line.append((c < 040 || c >= 0177) ? '.' : c ) ;
            }
            line.append("|");
        }
        lines.append(line);
    }
    return lines.join("\r\n");
}

QString QWoSerialInput::formatUnicodeHexText(const QString &buf)
{
    QList<QString> lines;
    int i, j;
    int len = buf.length();
    int cnt = hexModeCharCount() / 2;
    for(i = 0; i < len; i += cnt) {
        QString line;
        for(j = 0; j < cnt && i + j < len; j++) {
            char tmp[15] = {0};
            ushort uc = buf[i+j].unicode();
            uchar *ptr = (uchar*)&uc;
            sprintf(tmp, "%2.2x %2.2x ", ptr[0], ptr[1]);
            line.append(tmp);
        }
        line = line.toUpper();
        for(; j < cnt; ++j){
            line.append("      ");
        }
        if(!ui->chkHexOutput->isChecked()) {
            line.append("  |") ;
            for(j=0; j < cnt && i+j<len; j++){
                QChar c = buf[i+j];
                if(c.isPrint()) {
                    line.append(c);
                    if((c.unicode() & 0xFF00) == 0) {
                        line.append(".");
                    }
                }else{
                    line.append("..");
                }
            }
            line.append("|");
        }
        lines.append(line);
    }
    return lines.join("\r\n");
}

QByteArray QWoSerialInput::formatPrintText(const QByteArray &buf)
{
    int cnt = textModelCharCount();
    QList<QByteArray> lines;
    int i, j;
    int len = buf.length();

    for(i=0; i < len; i += cnt) {
        QByteArray line;
        for(j=0; j < cnt && i+j<len; j++){
            unsigned char c = buf[i+j];
            line.append((c < 040 || c >= 0177) ? '.' : c ) ;
        }
        lines.append(line);
    }
    return lines.join("\r\n");
}

int QWoSerialInput::textModelCharCount() const
{
    QKxTermItem *term = m_term->termItem();
    QSize sz = term->termSize();
    int total = sz.width();
    return total;
}

int QWoSerialInput::hexModeCharCount() const
{
    QKxTermItem *term = m_term->termItem();
    QSize sz = term->termSize();
    int total = sz.width();
    if(ui->modeOutput->currentIndex() > 0) {
        total -= HEX_TEXT_DISTANCE;
    }
    return ui->chkHexOutput->isChecked() ? total / 3 : total / 4;
}

bool QWoSerialInput::isHexString(const QByteArray &hex)
{
    for(int i = 0; i < hex.length(); i++) {
        char c = hex.at(i);
        if(c >= '0' && c <= '9') {
            continue;
        }
        if(c >= 'a' && c <= 'f') {
            continue;
        }
        if(c >= 'A' && c <= 'F') {
            continue;
        }
        if(c == QChar::Space) {
            continue;
        }
        return false;
    }
    return true;
}

QString QWoSerialInput::formatHexString(const QString &txtOld)
{
    QString txt = txtOld.toUpper();
    txt = txt.simplified();
    txt = txt.remove(QChar::Space);
    QString txtNew;
    for(int i = 0; i < txt.length(); i++) {
        txtNew.append(txt.at(i));
        if((i % 2) == 1) {
            txtNew.append(QChar::Space);
        }
    }
    return txtNew;
}

void QWoSerialInput::recheckCodePage()
{
    if(m_codec == nullptr || m_codec->name() != m_term->textCodec()) {
        QString codeName = m_term->textCodec();
        m_codec = QTextCodec::codecForName(codeName.toUtf8());
    }
}

void QWoSerialInput::handleSplitFilter(QList<LineOutput> &lines)
{
    int idx = ui->modeSplit->currentIndex();
    qint64 elapse = 0;
    if(idx == 1) {
        elapse = ui->splitInterval->text().toInt();
    }
    QByteArray special;
    if(idx == 2) {
        QByteArray txt = ui->splitChars->text().toLatin1();
        if(isHexString(txt)) {
            special = QByteArray::fromHex(txt);
        }
    }

    QByteArrayList vdef;
    vdef.append(QByteArray());
    QMap<QString, QByteArrayList> whoLines;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 bufLength = 0;
    for(auto it = m_output.begin(); it != m_output.end();){
        QString who = it.key();
        TimeOutput& to = it.value();
        if(to.buf.length() > 512*1024) {
            // cache too many data.
            QByteArrayList all = whoLines.value(who, vdef);
            QByteArray& buf = all.first();
            buf.append(to.buf);
            whoLines.insert(who, all);
            it = m_output.erase(it);
        }else if(idx == 0){
            // no split filter.
            QByteArrayList all = whoLines.value(who, vdef);
            QByteArray& buf = all.first();
            buf.append(to.buf);
            whoLines.insert(who, all);
            it = m_output.erase(it);
        }else if(idx == 1) {
            // split by timeout.
            if(now - to.tmLast > elapse) {
                QByteArrayList all = whoLines.value(who, vdef);
                QByteArray& buf = all.first();
                buf.append(to.buf);
                whoLines.insert(who, all);
                it = m_output.erase(it);
            }else{
                bufLength += to.buf.length();
                it++;
            }
        }else if(idx == 2 && !special.isEmpty()) {
            // Split by special hex string
            QByteArrayList all = whoLines.take(who);
            int idx = to.buf.indexOf(special, 0);
            while(idx >= 0){
                QByteArray tmp = to.buf.left(idx+special.length());
                all.append(tmp);
                to.buf = to.buf.remove(0, tmp.length());
                idx = to.buf.indexOf(special, 0);
                m_timeLastSplit = now;
            }
            if(!to.buf.isEmpty() && m_timeLastSplit > 0 && (now - m_timeLastSplit) > 1000) {
                all.append(to.buf);
                to.buf.clear();
            }
            if(!all.isEmpty()) {
                whoLines.insert(who, all);
            }
            if(to.buf.isEmpty()){
                it = m_output.erase(it);
            }else{
                bufLength += to.buf.length();
                it++;
            }
        }
    }
    for(auto it = whoLines.begin(); it != whoLines.end(); it++) {
        LineOutput lo;
        lo.who = it.key().toUtf8();
        lo.lines = it.value();
        lines.append(lo);
    }
    ui->bufLength->setText(QString::number(bufLength));
}

void QWoSerialInput::handleOutputFilter(const QByteArray &arrow, const QList<LineOutput> &lines)
{
    if(lines.isEmpty()) {
        return;
    }
    QKxTermItem *term = m_term->termItem();
    term->scrollToEnd();

    int idx = ui->modeOutput->currentIndex();
    if(idx == 0) {
        // no filter.
        for(auto it = lines.begin(); it != lines.end(); it++) {
            const LineOutput& lo = *it;
            QByteArray result;
            if(ui->chkDataSource->isChecked()) {
                result += "\r\n" + lo.who + arrow + ": \r\n";
            }
            for(auto jt = lo.lines.begin(); jt != lo.lines.end(); jt++) {
                QByteArray tmp = *jt;
                result += tmp;
            }

            term->parse(result);
        }
    }else if(idx == 1) {
        // hex string.
        for(auto it = lines.begin(); it != lines.end(); it++) {
            const LineOutput& lo = *it;
            QByteArray result;
            if(ui->chkDataSource->isChecked()) {
                result += "\r\n" + lo.who + arrow + ": ";
            }
            for(auto jt = lo.lines.begin(); jt != lo.lines.end(); jt++) {
                const QByteArray& line = *jt;
                result += "\r\n" + formatHexText(line);
            }
            term->parse(result);
        }
    }else{
        // unicode hex string.
        recheckCodePage();
        for(auto it = lines.begin(); it != lines.end(); it++) {
            const LineOutput& lo = *it;
            QString result;
            if(ui->chkDataSource->isChecked()) {
                result += "\r\n" + lo.who + arrow + ": ";
            }
            for(auto jt = lo.lines.begin(); jt != lo.lines.end(); jt++) {
                const QByteArray& line = *jt;
                QByteArray bufLeft = m_whoBufferLeft.take(lo.who);
                bufLeft.append(line);
                QTextCodec::ConverterState left;
                const QString& uLine = m_codec->toUnicode(bufLeft.data(), bufLeft.length(), &left);
                int cnt = left.remainingChars;
                if(cnt > 0) {
                    bufLeft = bufLeft.right(cnt);
                    m_whoBufferLeft.insert(lo.who, bufLeft);
                }
                result += "\r\n" + formatUnicodeHexText(uLine);
            }
            term->unicodeParse(result);
        }
    }
}

bool QWoSerialInput::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type type = ev->type();
    if(obj == ui->edit) {
        if(type == QEvent::KeyPress || type == QEvent::KeyRelease) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(ev);
            Qt::KeyboardModifiers modifiers = ke->modifiers();
            if(modifiers == Qt::NoModifier && ui->modeInput->currentIndex() == 1) {
                int key = ke->key();
                if(key >= Qt::Key_0 && key <= Qt::Key_9) {
                    return false;
                }
                if(key >= Qt::Key_A && key <= Qt::Key_F) {
                    return false;
                }
                if(key == Qt::Key_Space||
                        key == Qt::Key_Left ||
                        key == Qt::Key_Right ||
                        key == Qt::Key_Backspace ||
                        key == Qt::Key_Delete||
                        key == Qt::Key_Home ||
                        key == Qt::Key_End ||
                        key == Qt::Key_Up ||
                        key == Qt::Key_Down) {
                    return false;
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}
