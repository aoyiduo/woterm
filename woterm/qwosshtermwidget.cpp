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

#include "qwosshtermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotermwidgetimpl.h"
#include "qwossh.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwohostsimplelist.h"
#include "qmodem.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qwosessionproperty.h"
#include "qwofloatwindow.h"

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qwoshower.h"


#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QResizeEvent>

QWoSshTermWidget::QWoSshTermWidget(const QString& target, int gid, QWidget *parent)
    : QWoTermWidget(target, gid, parent)
    , m_savePassword(false)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(const QByteArray&)), this, SLOT(onSendData(const QByteArray&)));
    m_modem = QWoModemFactory::instance()->create(false);
    QObject::connect(m_modem, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onZmodemDataArrived(const QByteArray&)));
    QObject::connect(m_modem, SIGNAL(statusArrived(const QByteArray&)), this, SLOT(onZmodemStatusArrived(const QByteArray&)));
    QObject::connect(m_modem, SIGNAL(finished()), this, SLOT(onZmodemFinished()));

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QWoSshTermWidget::~QWoSshTermWidget()
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

void QWoSshTermWidget::onConnectionFinished(bool ok)
{
    if(!ok) {
        onFinishArrived(-1);
    }
}

void QWoSshTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    showLoading(false);
    if(m_mask && m_mask->isVisible()) {
        return;
    }

    if(m_mask == nullptr) {
        m_mask = new QWoTermMask(this);
        QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseThisSession()));
        QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));
    }
    m_mask->setGeometry(0, 0, width(), height());
    m_mask->show();
}

void QWoSshTermWidget::onDataArrived(const QByteArray &buf)
{
    showLoading(false);
    static bool bOut = false;
    if(bOut) {
        qDebug() << "record:" << buf;
    }
    if(m_modem->isRunning()) {
        if(!m_modem->onReceive(buf)) {
            //qDebug() << "onDataArrived" << buf;
        }
    }else{
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

void QWoSshTermWidget::onErrorArrived(const QByteArray &buf)
{
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf);
}

void QWoSshTermWidget::onInputArrived(const QString &title, const QString &prompt, bool visble)
{
    showPasswordInput(title, prompt, visble);
}

void QWoSshTermWidget::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword){
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QWoSshTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_ssh) {
        m_ssh->updateSize(columns, lines);
    }
}

void QWoSshTermWidget::onSendData(const QByteArray &buf)
{
#if 0
    if(buf.length() == 1 && buf.at(0) == '0') {
        //m_term->parse("\x1B[?1049h\x1B[1;44r\x1B(B\x1B[m\x1B[4l\x1B[?7h\x1B[39;49m\x1B[?1h\x1B=\x1B[39;49m");
        executeCommand("ls && df && free h");
        return;
    }
#endif
    if(m_ssh) {
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

void QWoSshTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QWoSshTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QWoSshTermWidget::onPasswordInputResult(const QString &pass, bool isSave)
{
    m_savePassword = isSave;
    if(m_ssh){
        m_ssh->setInputResult(pass);
    }
}

void QWoSshTermWidget::onSessionReconnect()
{    
    reconnect();
}

void QWoSshTermWidget::onVerticalSplitView()
{
    splitWidget(m_target, m_gid, true);
}

void QWoSshTermWidget::onHorizontalSplitView()
{
    splitWidget(m_target, m_gid, false);
}

void QWoSshTermWidget::onVerticalInviteView()
{
    QWoHostSimpleList dlg(SshWithSftp, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), true);
    }
}

void QWoSshTermWidget::onHorizontalInviteView()
{
    QWoHostSimpleList dlg(SshWithSftp, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), false);
    }
}

void QWoSshTermWidget::onCloseThisSession()
{
    closeAndDelete();
}

void QWoSshTermWidget::onForceToCloseThisSession()
{
    closeAndDelete();
}

void QWoSshTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QWoSshTermWidget::onDuplicateInNewWindow()
{
    QWoUtils::openself("ssh", m_target, false);
}

void QWoSshTermWidget::onNewSessionMultiplex()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSsh(m_target, m_gid);
}

void QWoSshTermWidget::onModifyThisSession()
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

void QWoSshTermWidget::onZmodemSend(bool local)
{
    if(local) {
//        if(!checkProgram("rz")) {
//            if(local) {
//                m_term->parseError("failed to find rz program, please install lrzsz.");
//                m_term->waitInput();
//                QMessageBox::warning(this, tr("warning"), tr("failed to find rz program, please install lrzsz."));
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

void QWoSshTermWidget::onZmodemRecv(bool local)
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

void QWoSshTermWidget::onZmodemAbort()
{
    m_modem->stop();
}

void QWoSshTermWidget::onZmodemDataArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        //qDebug() << "onZmodemDataArrived" << objectName() << buf;
        m_ssh->write(buf);
    }
}

void QWoSshTermWidget::onZmodemStatusArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        m_term->parse(buf);
    }
}

void QWoSshTermWidget::onZmodemFinished()
{
    //qDebug() << "onZmodemFinished" << objectName();
    m_ssh->write("\r");
    m_term->parse("\033[?25h");
}

void QWoSshTermWidget::onSftpConnectReady()
{
    emit sftpAssistant();
    //QMetaObject::invokeMethod(QWoMainWindow::instance(), "onSessionReadyToSftpConnect", Qt::QueuedConnection, Q_ARG(QString, m_target));
}

void QWoSshTermWidget::onForceToReconnect()
{
    reconnect();
}

void QWoSshTermWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(this);
        QObject::connect(m_passInput, SIGNAL(result(const QString&,bool)), this, SLOT(onPasswordInputResult(const QString&,bool)));
    }
    if(m_passInput->isVisible()) {
        return;
    }
    m_passInput->reset(title, prompt, echo);
    m_passInput->setGeometry(0, 0, width(), height());
    m_passInput->show();
}

int QWoSshTermWidget::isZmodemCommand(const QByteArray &data)
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

bool QWoSshTermWidget::checkProgram(const QByteArray &name)
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

void QWoSshTermWidget::reconnect()
{
    if(m_ssh) {
        QWoSshFactory::instance()->release(m_ssh);
    }
    m_term->resetState();
    m_ssh = QWoSshFactory::instance()->createShell(false);

    QObject::connect(m_ssh, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_ssh, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_ssh, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onDataArrived(const QByteArray&)));
    QObject::connect(m_ssh, SIGNAL(errorArrived(const QByteArray&)), this, SLOT(onErrorArrived(const QByteArray&)));
    QObject::connect(m_ssh, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SLOT(onPasswordArrived(const QString&,const QByteArray&)));
    QObject::connect(m_ssh, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));
    m_ssh->start(m_target, m_gid);
    QSize sz = m_term->termSize();
    m_ssh->updateSize(sz.width(), sz.height());
    showLoading(true);


    HostInfo hi = QWoSshConf::instance()->find(m_target);
    if(!hi.script.isEmpty()){
        executeCommand(hi.script.toUtf8());
    }
}

void QWoSshTermWidget::executeCommand(const QByteArray &cmd)
{
    if(m_cmd) {
        QWoSshFactory::instance()->release(m_cmd);
    }
    m_cmd = QWoSshFactory::instance()->createShell(true);
    QObject::connect(m_cmd, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onDataArrived(const QByteArray&)));
    QObject::connect(m_cmd, SIGNAL(errorArrived(const QByteArray&)), this, SLOT(onErrorArrived(const QByteArray&)));
    QObject::connect(m_cmd, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SLOT(onPasswordArrived(const QString&,const QByteArray&)));
    QObject::connect(m_cmd, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));
    m_cmd->start(m_target, m_gid);
    QSize sz = m_term->termSize();
    m_cmd->updateSize(sz.width(), sz.height());
    m_cmd->write(cmd);
}

void QWoSshTermWidget::resizeEvent(QResizeEvent *ev)
{
    QWoTermWidget::resizeEvent(ev);
    QSize sz = ev->size();
    //qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
    if(m_mask) {
        m_mask->setGeometry(0, 0, sz.width(), sz.height());
    }
    if(m_passInput) {
        m_passInput->setGeometry(0, 0, sz.width(), sz.height());
    }
}

void QWoSshTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    if(m_menu == nullptr) {
        m_menu = new QMenu(this);        
        m_copy = m_menu->addAction(tr("Copy"));
        QObject::connect(m_copy, SIGNAL(triggered()), this, SLOT(onCopyToClipboard()));
        m_paste = m_menu->addAction(tr("Paste"));
        QObject::connect(m_paste, SIGNAL(triggered()), this, SLOT(onPasteFromClipboard()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Force Reconnect"), this, SLOT(onForceToReconnect()));
        QAction *vsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/vsplit.png"), tr("Split Vertical"));
        QObject::connect(vsplit, SIGNAL(triggered()), this, SLOT(onVerticalSplitView()));
        QAction *hsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/hsplit.png"), tr("Split Horizontal"));
        QObject::connect(hsplit, SIGNAL(triggered()), this, SLOT(onHorizontalSplitView()));
       // QAction *vinvite = m_menu->addAction(QIcon(":/woterm/resource/skin/vaddsplit.png"), tr("Add To Vertical"));
       // QObject::connect(vinvite, SIGNAL(triggered()), this, SLOT(onVerticalInviteView()));
       // QAction *hinvite = m_menu->addAction(QIcon(":/woterm/resource/skin/haddsplit.png"), tr("Add To Horizontal"));
       // QObject::connect(hinvite, SIGNAL(triggered()), this, SLOT(onHorizontalInviteView()));        
        m_menu->addAction(QIcon(":/woterm/resource/skin/sftp.png"), tr("Sftp Assistant"), this, SLOT(onSftpConnectReady()));

        m_menu->addAction(QIcon(":/woterm/resource/skin/find.png"), tr("Find..."), this, SLOT(onShowFindBar()), QKeySequence(Qt::CTRL +  Qt::Key_F));
        m_menu->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
        //m_menu->addAction(QIcon(":/woterm/resource/skin/history.png"), tr("History"), this, SLOT(onSessionCommandHistory()));
        m_menu->addAction(tr("Duplicate In New Window"), this, SLOT(onDuplicateInNewWindow()));
        m_menu->addAction(tr("New Session Multiplex"), this, SLOT(onNewSessionMultiplex()));
        m_menu->addAction(tr("Clean history"), this, SLOT(onCleanHistory()));
        m_output = m_menu->addAction(tr("Output history to file"), this, SLOT(onOutputHistoryToFile()));
        m_stop = m_menu->addAction(tr("Stop history to file"), this, SLOT(onStopOutputHistoryFile()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/upload.png"), tr("Zmodem Upload"), this, SLOT(onZmodemSend()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/download.png"), tr("Zmodem Receive"), this, SLOT(onZmodemRecv()));
        m_menu->addAction(tr("Zmoddem Abort"), this, SLOT(onZmodemAbort()), QKeySequence(Qt::CTRL +  Qt::Key_C));
        m_menu->addAction(tr("Close Session"), this, SLOT(onCloseThisSession()));
        QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
        if(wfloat == nullptr) {
            m_menu->addAction(tr("Float This Tab"), this, SLOT(onFloatThisTab()));
        }
    }
    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    //qDebug() << "selectText" << selTxt;
    m_copy->setDisabled(selTxt.isEmpty());
    m_output->setVisible(m_historyFile.isEmpty());
    m_stop->setVisible(!m_historyFile.isEmpty());
    QClipboard *clip = QGuiApplication::clipboard();
    QString clipTxt = clip->text();
    m_paste->setDisabled(clipTxt.isEmpty());
    m_menu->exec(QCursor::pos());
    m_menu->deleteLater();
}

QList<QString> QWoSshTermWidget::collectUnsafeCloseMessage()
{
    QList<QString> all;
    if(m_modem->isRunning()) {
        QString msg = tr("Files are transfering...");
        all.append(msg);
    }
    return all;
}
