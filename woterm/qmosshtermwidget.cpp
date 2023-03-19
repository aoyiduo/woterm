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

#include "qmosshtermwidget.h"

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxmessagebox.h"
#include "qwossh.h"
#include "qwosshconf.h"
#include "qwomodem.h"
#include "qwoutils.h"
#include "qwosetting.h"
#include "qwopasswordinput.h"

#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QResizeEvent>
#include <QPushButton>
#include <QMessageBox>

int QMoSshTermWidget::m_gid = 1000;

QMoSshTermWidget::QMoSshTermWidget(const QString& target, QWidget *parent)
    : QMoTermWidget(target, ETTRemoteTarget,  parent)
    , m_savePassword(false)
    , m_stateConnected(ESC_Ready)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(QByteArray)), this, SLOT(onSendData(QByteArray)));
    QObject::connect(m_term, SIGNAL(activePathArrived(QString)), this, SLOT(onActivePathArrived(QString)));
    m_modem = QWoModemFactory::instance()->create(false);
    QObject::connect(m_modem, SIGNAL(dataArrived(QByteArray)), this, SLOT(onZmodemDataArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(statusArrived(QByteArray)), this, SLOT(onZmodemStatusArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(finished()), this, SLOT(onZmodemFinished()));

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QMoSshTermWidget::~QMoSshTermWidget()
{
    if(m_ssh){
        QWoSshFactory::instance()->release(m_ssh);
    }
    if(m_cmd) {
        QWoSshFactory::instance()->release(m_cmd);
    }
    if(m_modem) {
        QWoModemFactory::instance()->release(m_modem);
    }
}

void QMoSshTermWidget::onConnectionFinished(bool ok)
{
    if(!ok) {
        onFinishArrived(-1);
    }
}

void QMoSshTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    showLoading(false);
    if(m_stateConnected != ESC_Disconnected) {
        m_stateConnected = ESC_Disconnected;
        m_term->parseError("\r\nthe session is closed");
        m_term->parseError("\r\npress any key to popup selection dialog.");
    }
}

void QMoSshTermWidget::onDataArrived(const QByteArray &buf)
{
    showLoading(false);
    static bool bOut = false;
    if(bOut) {
        qDebug() << "record:" << buf;
    }
    m_stateConnected = ESC_Connected;
    if(m_modem->isRunning()) {
        if(!m_modem->onReceive(buf)) {
            //qDebug() << "onDataArrived" << buf;
        }
    }else{
        if(m_restoreLastActivePath) {
            m_ssh->write("\r\ncd " + m_lastActivePath.toUtf8() + "\r\n");
            m_restoreLastActivePath = false;
        }
        if(m_term->appMode()) {
            m_term->parse(buf);
        }else{
            int err = isZmodemCommand(buf);
            m_term->parse(buf);
            if(err == 0) {
                QMetaObject::invokeMethod(this, "onZmodemRecv", Qt::QueuedConnection, Q_ARG(bool, false));
            }else if(err == 1) {
                QMetaObject::invokeMethod(this, "onZmodemSend", Qt::QueuedConnection, Q_ARG(bool, false));
            }
        }

    }

}

void QMoSshTermWidget::onErrorArrived(const QString &buf, const QVariantMap& userData)
{
    Q_UNUSED(userData);
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf.toUtf8());
}

void QMoSshTermWidget::onInputArrived(const QString &title, const QString &prompt, bool visble)
{
    showLoading(false);
    showPasswordInput(title, prompt, visble);
}

void QMoSshTermWidget::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword){
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QMoSshTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_ssh) {
        m_ssh->updateSize(columns, lines);
    }
}

void QMoSshTermWidget::onSendData(const QByteArray &buf)
{
#if 0
    if(buf.length() == 1 && buf.at(0) == '0') {
        //m_term->parse("\x1B[?1049h\x1B[1;44r\x1B(B\x1B[m\x1B[4l\x1B[?7h\x1B[39;49m\x1B[?1h\x1B=\x1B[39;49m");
        executeCommand("ls && df && free h");
        return;
    }
#endif
    if(m_stateConnected == ESC_Disconnected) {
        if(m_dlgConfirm == nullptr) {
            m_dlgConfirm = new QKxMessageBox(QMessageBox::Question, tr("Reconnection confirmation"), tr("Continue to connect to the server?"), QMessageBox::Yes|QMessageBox::No, this);
            QPushButton *btn = new QPushButton(tr("Restore"), m_dlgConfirm);
            QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onRestoreLastPath()));
            m_dlgConfirm->addButton(btn, QMessageBox::ActionRole);
            int code = m_dlgConfirm->exec();
            if(code == QMessageBox::Yes) {
                QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
            }
            m_dlgConfirm->deleteLater();
        }
    }else if(m_ssh) {
        m_term->scrollToEnd();
        if(!m_modem->isRunning()) {
            m_ssh->write(buf);
        }else{
            if(buf == "\003") {
                onZmodemAbort();
            }
        }
    }
}

void QMoSshTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QMoSshTermWidget::onRestoreLastPath()
{
    if(m_dlgConfirm) {
        m_dlgConfirm->done(QMessageBox::LastButton + 1);
        reconnect(true);
    }
}

void QMoSshTermWidget::onActivePathArrived(const QString &path)
{
    m_lastActivePath = path;
}

void QMoSshTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QMoSshTermWidget::onPasswordInputResult(const QString &pass, bool isSave)
{
    m_savePassword = isSave;
    if(m_ssh){
        showLoading(true);
        m_ssh->setInputResult(pass);
    }
}

void QMoSshTermWidget::onSessionReconnect()
{
    reconnect();
}

void QMoSshTermWidget::onCloseThisSession()
{

}

void QMoSshTermWidget::onPasteTestFont()
{
    m_term->parseTest();
}

void QMoSshTermWidget::onForceToCloseThisSession()
{

}

void QMoSshTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QMoSshTermWidget::onDuplicateInNewWindow()
{
    QWoUtils::openself("ssh", m_target, false);
}

void QMoSshTermWidget::onModifyThisSession()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QKxMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete ago"));
        return;
    }
}

void QMoSshTermWidget::onZmodemSend(bool local)
{
    if(local) {
//        if(!checkProgram("rz")) {
//            if(local) {
//                m_term->parseError("failed to find rz program, please install lrzsz.");
//                m_term->waitInput();
//                QKxMessageBox::warning(this, tr("warning"), tr("failed to find rz program, please install lrzsz."));
//            }
//            return;
//        }
    }
    QString pathLast = QWoSetting::value("zmodem/lastPath", "").toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Files"), pathLast);
    //qDebug() << "zmodem send " << files;
    if(files.isEmpty()) {
        return;
    }
    QStringList args;
    QString path = files.front();
    int idx = path.lastIndexOf('/');
    if(idx > 0) {
        path = path.left(idx);
        QWoSetting::setValue("zmodem/lastPath", path);
    }

    m_modem->ZSendFiles(files);
}

void QMoSshTermWidget::onZmodemRecv(bool local)
{
    QString path = QWoSetting::value("zmodem/lastPath").toString();
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), path,  QFileDialog::ShowDirsOnly);
    qDebug() << "filePath" << filePath;
    if(filePath.isEmpty()) {
        onZmodemAbort();
        return;
    }
    filePath = QDir::toNativeSeparators(filePath);
    QWoSetting::setValue("zmodem/lastPath", filePath);
    m_modem->ZReceive(filePath);
}

void QMoSshTermWidget::onZmodemAbort()
{
    m_modem->stop();
}

void QMoSshTermWidget::onZmodemDataArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        //qDebug() << "onZmodemDataArrived" << objectName() << buf;
        m_ssh->write(buf);
    }
}

void QMoSshTermWidget::onZmodemStatusArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        m_term->parse(buf);
    }
}

void QMoSshTermWidget::onZmodemFinished()
{
    //qDebug() << "onZmodemFinished" << objectName();
    m_ssh->write("\r");
    m_term->parse("\033[?25h");
}

void QMoSshTermWidget::onSftpConnectReady()
{

}

void QMoSshTermWidget::onForceToReconnect()
{
    reconnect();
}

void QMoSshTermWidget::onAdjustPosition()
{
    if(m_passInput) {
        QSize sz = m_passInput->minimumSize();
        if(sz.width() == 0 || sz.height() == 0) {
            sz = m_passInput->size();
        }
        QRect rt(0, 0, sz.width(), sz.height());
        rt.moveCenter(QPoint(width() / 2, height() / 2));
        m_passInput->setGeometry(rt);
    }
}

void QMoSshTermWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(this);
        QObject::connect(m_passInput, SIGNAL(result(const QString&,bool)), this, SLOT(onPasswordInputResult(const QString&,bool)));
    }
    if(m_passInput->isVisible()) {
        return;
    }
    m_passInput->reset(title, prompt, echo);
    m_passInput->adjustSize();
    m_passInput->showNormal();
    QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
}

int QMoSshTermWidget::isZmodemCommand(const QByteArray &data)
{
    bool isApp = m_term->appMode();
    if(isApp || data.length() < 6) {
        return -1;
    }
    // hex way.
    //char zmodem_init_hex[] = {'*','*','\030', 'B', '0', '0', '\0'};
    const char *buf = data.data();
    for(int i = 0; i < data.length() && i < 100; i++) {
        if(buf[i] == '*' && buf[i+1] == '*' && buf[i+2] == '\030'
                && buf[i+3] == 'B' && buf[i+4] == '0') {
            if(buf[i+5] == '1') {
                // shell trigger rz command.
                return 1;
            }else if(buf[i+5] == '0') {
                // shell trigger sz command.
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

bool QMoSshTermWidget::checkProgram(const QByteArray &name)
{
    if(m_modem->isRunning()) {
        return false;
    }
    if(m_term->appMode()) {
        return false;
    }
    QString txt;
    int code;
    if(!m_term->trapCommand("which "+name, txt, code)) {
        return false;
    }
    return code == 0;
}

void QMoSshTermWidget::reconnect(bool restore)
{
    if(m_passInput) {
        m_passInput->deleteLater();
    }
    if(m_ssh) {
        QWoSshFactory::instance()->release(m_ssh);
    }
    m_term->resetState();
    m_ssh = QWoSshFactory::instance()->createShell(false);

    QObject::connect(m_ssh, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_ssh, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_ssh, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));
    QObject::connect(m_ssh, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_ssh, SIGNAL(passwordArrived(QString,QByteArray)), this, SLOT(onPasswordArrived(QString,QByteArray)));
    QObject::connect(m_ssh, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    m_ssh->start(m_target, m_gid);
    QSize sz = m_term->termSize();
    m_ssh->updateSize(sz.width(), sz.height());
    showLoading(true);


    if(restore) {
        qDebug() << "path";
        m_restoreLastActivePath = true;
    }else{
        m_restoreLastActivePath = false;
        HostInfo hi = QWoSshConf::instance()->find(m_target);
        if(!hi.script.isEmpty()){
            executeCommand(hi.script.toUtf8());
        }
    }

    m_stateConnected = ESC_Connecting;
}

void QMoSshTermWidget::executeCommand(const QByteArray &cmd)
{
    if(m_cmd) {
        QWoSshFactory::instance()->release(m_cmd);
    }
    m_cmd = QWoSshFactory::instance()->createShell(true);
    QObject::connect(m_cmd, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));
    QObject::connect(m_cmd, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_cmd, SIGNAL(passwordArrived(QString,QByteArray)), this, SLOT(onPasswordArrived(QString,QByteArray)));
    QObject::connect(m_cmd, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    m_cmd->start(m_target, m_gid);
    QSize sz = m_term->termSize();
    m_cmd->updateSize(sz.width(), sz.height());
    m_cmd->write(cmd);
}

void QMoSshTermWidget::resizeEvent(QResizeEvent *ev)
{
    QMoTermWidget::resizeEvent(ev);
    QSize sz = ev->size();
    //qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
    if(m_passInput) {
        QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
    }
}

void QMoSshTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    if(m_rkeyPaste) {
        if(m_term->isOverSelection(ev->pos())) {
            QString txtSel = m_term->selectedText();
            m_term->directSendData(txtSel.toUtf8());
            return;
        }
    }

    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    //qDebug() << "selectText" << selTxt;
    QClipboard *clip = QGuiApplication::clipboard();
    QString clipTxt = clip->text();

    QMenu menu(this);
    if(!selTxt.isEmpty()) {
        menu.addAction(tr("Copy"), this, SLOT(onCopyToClipboard()));
    }
    if(!clipTxt.isEmpty()) {
        menu.addAction(tr("Paste"), this, SLOT(onPasteFromClipboard()));
    }

    menu.addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Force Reconnect"), this, SLOT(onForceToReconnect()));
    QAction *vsplit = menu.addAction(QIcon(":/woterm/resource/skin/vsplit.png"), tr("Split Vertical"));
    QObject::connect(vsplit, SIGNAL(triggered()), this, SLOT(onVerticalSplitView()));
    QAction *hsplit = menu.addAction(QIcon(":/woterm/resource/skin/hsplit.png"), tr("Split Horizontal"));
    QObject::connect(hsplit, SIGNAL(triggered()), this, SLOT(onHorizontalSplitView()));

    menu.addAction(QIcon(":/woterm/resource/skin/sftp.png"), tr("Sftp Assistant"), this, SLOT(onSftpConnectReady()));
    menu.addAction(QIcon(":/woterm/resource/skin/find.png"), tr("Find..."), this, SLOT(onShowFindBar()), QKeySequence(Qt::CTRL +  Qt::Key_F));
    menu.addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
    menu.addAction(tr("Duplicate in new window"), this, SLOT(onDuplicateInNewWindow()));
    menu.addAction(tr("New session multiplex"), this, SLOT(onNewSessionMultiplex()));
    //menu.addAction(tr("Reset terminal size"), this, SLOT(onResetTermSize()));
    menu.addSeparator();
    menu.addAction(tr("Clean history"), this, SLOT(onCleanHistory()));
    if(m_historyFile.isEmpty()) {
        menu.addAction(tr("Output history to file"), this, SLOT(onOutputHistoryToFile()));
    }else{
        menu.addAction(tr("Stop history to file"), this, SLOT(onStopOutputHistoryFile()));
    }
    menu.addSeparator();
    menu.addAction(QIcon(":/woterm/resource/skin/upload.png"), tr("Zmodem upload"), this, SLOT(onZmodemSend()));
    menu.addAction(QIcon(":/woterm/resource/skin/download.png"), tr("Zmodem receive"), this, SLOT(onZmodemRecv()));
    menu.addAction(tr("Zmodem abort"), this, SLOT(onZmodemAbort()), QKeySequence(Qt::CTRL +  Qt::Key_C));
    menu.addSeparator();
    menu.addAction(tr("Close Session"), this, SLOT(onCloseThisSession()));
#ifdef QT_DEBUG
    menu.addAction(tr("Test font"), this, SLOT(onPasteTestFont()));
#endif
    menu.exec(QCursor::pos());
}

QList<QString> QMoSshTermWidget::collectUnsafeCloseMessage()
{
    QList<QString> all;
    if(m_modem->isRunning()) {
        QString msg = tr("Files are transfering...");
        all.append(msg);
    }
    return all;
}
