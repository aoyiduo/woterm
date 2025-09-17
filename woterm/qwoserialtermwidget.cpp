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

#include "qwoserialtermwidget.h"
#include "qwoserialinput.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwosshconf.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxkeytranslator.h"
#include "qwosessionttyproperty.h"
#include "qkxmessagebox.h"
#include "qwosetting.h"
#include "qwomodem.h"

#include <QCloseEvent>
#include <QApplication>
#include <QMenu>
#include <QLineEdit>
#include <QTabBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QClipboard>
#include <QSerialPort>
#include <QTimer>
#include <QFileDialog>
#include <QDebug>


QWoSerialTermWidget::QWoSerialTermWidget(const QString &target, QWidget *parent)
    : QWoTermWidget(target, 0, ETTSerialPort, parent)
{
    m_term->showTermName(false);

    m_term->installEventFilter(this);

    m_modem = QWoModemFactory::instance()->create(false);
    QObject::connect(m_modem, SIGNAL(dataArrived(QByteArray)), this, SLOT(onZmodemDataArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(statusArrived(QByteArray)), this, SLOT(onZmodemStatusArrived(QByteArray)));
    QObject::connect(m_modem, SIGNAL(finished()), this, SLOT(onZmodemFinished()));
}

QWoSerialTermWidget::~QWoSerialTermWidget()
{

}

void QWoSerialTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu(this);
    m_menu = &menu;
    QAction* m_copy = menu.addAction(tr("Copy"));
    QObject::connect(m_copy, SIGNAL(triggered()), this, SLOT(onCopyToClipboard()));
    menu.addAction(QIcon("../private/skins/black/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
    menu.addSeparator();
    menu.addAction(tr("Clean history"), this, SLOT(onCleanThisSession()));
    if(!hasHistoryFile()) {
        menu.addAction(tr("Output history to file"), this, SLOT(onOutputHistoryToFile()));
    }else{
        menu.addAction(tr("Stop history to file"), this, SLOT(onStopOutputHistoryFile()));
    }
    menu.addSeparator();
    if(m_term->readOnly()) {
        menu.addAction(tr("Open interactive mode"), this, SLOT(onOpenInteractiveMode()));
    }else{
        menu.addAction(tr("Close interactive mode"), this, SLOT(onCloseInteractiveMode()));
    }
    QKeySequence ksFind = m_term->keyTranslator()->shortcut(QKxKeyTranslator::EFind);
    menu.addAction(QIcon("../private/skins/black/find.png"), tr("Find..."), this, SLOT(onShowFindBar()), ksFind);
#if 0
    menu.addSeparator();
    menu.addAction(QIcon("../private/skins/black/upload.png"), tr("Zmodem upload"), this, SLOT(onZmodemSend()));
    menu.addAction(QIcon("../private/skins/black/download.png"), tr("Zmodem receive"), this, SLOT(onZmodemRecv()));
    menu.addAction(tr("Zmodem abort"), this, SLOT(onZmodemAbort()), QKeySequence(Qt::CTRL +  Qt::Key_C));
    menu.addSeparator();
#endif
    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    m_copy->setDisabled(selTxt.isEmpty());
    menu.exec(QCursor::pos());
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
    QVariantMap prop = QWoSetting::serialPort();
    QWoSessionTTYProperty dlg(QWoSessionTTYProperty::ETTY_SerialPort, this);
    dlg.setCustom(prop);
    dlg.exec();
    QVariantMap result = dlg.result();
    if(!result.isEmpty()) {
        QWoSetting::setSerialPort(result);
        initCustom();
    }
}

void QWoSerialTermWidget::onCleanThisSession()
{
    m_term->clearAll();
}

void QWoSerialTermWidget::onOpenInteractiveMode()
{
    m_term->setReadOnly(false);
}

void QWoSerialTermWidget::onCloseInteractiveMode()
{
    m_term->setReadOnly(true);
}

void QWoSerialTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

int QWoSerialTermWidget::isZmodemCommand(const QByteArray &data)
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

bool QWoSerialTermWidget::checkZmodemInstall()
{
    if(m_modem->isRunning()) {
        return false;
    }
    if(m_term->appMode()) {
        return false;
    }
    QString content, reason;
    QString yes = "Yes, lrzsz has been installed.";
    QString no = "No, you need to install lrzsz package to execute zmodem.";
    QString cmd = QString("which rz && echo \"%1\" || echo \"%2\"").arg(yes, no);
    if(!m_term->executeCommand(cmd, content, reason)) {
        return false;
    }
    QString result = content;
    int pos = result.indexOf(cmd);
    if(pos >= 0) {
        result = result.mid(pos+cmd.length());
    }
    return result.contains(yes);
}

bool QWoSerialTermWidget::eventFilter(QObject *watched, QEvent *ev)
{
    QKxTermItem *term = qobject_cast<QKxTermItem*>(watched);
    if(term != nullptr) {
        if(ev->type() == QEvent::KeyPress) {
            if(term->readOnly()) {
                int err = QKxMessageBox::information(this, tr("Interactive mode"), tr("The current terminal is in read-only mode. Do you need to temporarily set it to interactive mode."), QKxMessageBox::Yes|QKxMessageBox::No);
                if(err == QKxMessageBox::Yes) {
                    term->setReadOnly(false);
                }
                return false;
            }
        }
    }
    return QWoTermWidget::eventFilter(watched, ev);
}

void QWoSerialTermWidget::onZmodemSend(bool local)
{
    if(local) {
        if(!checkZmodemInstall()) {
            if(local) {
                m_term->parseError("failed to find rz program, please install lrzsz.");
                QKxMessageBox::warning(this, tr("warning"), tr("failed to find rz program, please install lrzsz."));
            }
            return;
        }
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

void QWoSerialTermWidget::onZmodemRecv(bool local)
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

void QWoSerialTermWidget::onZmodemAbort()
{
    m_modem->abort();
}

void QWoSerialTermWidget::onZmodemDataArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        //qDebug() << "onZmodemDataArrived" << objectName() << buf;
        emit sendData(buf);
        //m_ssh->write(buf);
    }
}

void QWoSerialTermWidget::onZmodemStatusArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        m_term->parse(buf);
    }
}

void QWoSerialTermWidget::onZmodemFinished()
{
    //qDebug() << "onZmodemFinished" << objectName();
    emit sendData("\r");
   // m_ssh->write("\r");
    m_term->parse("\033[?25h");
}
