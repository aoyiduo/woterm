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

#include "qwosessionlist.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwohostinfoedit.h"
#include "qwohostlistmodel.h"
#include "qwolistview.h"
#include "qwosessionproperty.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QApplication>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QtAlgorithms>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QTimer>
#include <QModelIndexList>
#include <QMenu>
#include <QAction>
#include <QPlainTextEdit>
#include <QMessageBox>

#define MAX_TRY_LEFT  (10)

QWoSessionList::QWoSessionList(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    layout->addLayout(hlayout);
    m_list = new QWoListView(this);
    m_input = new QLineEdit(this);
    m_info = new QPlainTextEdit(this);
    hlayout->addWidget(m_input);
    layout->addWidget(m_list);
    layout->addWidget(m_info);
    layout->setSpacing(5);
    m_info->setReadOnly(true);
    m_info->setFixedHeight(150);
    m_list->installEventFilter(this);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setSelectionMode(QAbstractItemView::MultiSelection);

    m_model = QWoHostListModel::instance();
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_list->setModel(m_proxyModel);
    refreshList();

    QObject::connect(m_input, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    QObject::connect(m_input, SIGNAL(textChanged(const QString&)), this, SLOT(onEditTextChanged(const QString&)));
    QObject::connect(m_list, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onListItemDoubleClicked(const QModelIndex&)));
    QObject::connect(m_list, SIGNAL(itemChanged(const QModelIndex&)), this, SLOT(onListCurrentItemChanged(const QModelIndex&)));
    QObject::connect(m_list, SIGNAL(returnKeyPressed()), this, SLOT(onListReturnKeyPressed()));

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(3000);
    m_countLeft = -1;
    m_input->clear();
}

QWoSessionList::~QWoSessionList()
{
    if(m_menu) {
        delete m_menu;
    }
}

void QWoSessionList::init()
{

}


void QWoSessionList::refreshList()
{
    QWoSshConf::instance()->save();
    QWoSshConf::instance()->refresh();
    m_model->refreshList();
}

void QWoSessionList::onReloadSessionList()
{
    refreshList();
}

void QWoSessionList::onEditTextChanged(const QString &txt)
{
    m_countLeft = MAX_TRY_LEFT;
    QStringList sets = txt.split(' ');
    for(QStringList::iterator iter = sets.begin(); iter != sets.end(); ) {
        if(*iter == "") {
            iter = sets.erase(iter);
        }else{
            iter++;
        }
    }

    QRegExp regex(sets.join(".*"), Qt::CaseInsensitive);
    regex.setPatternSyntax(QRegExp::RegExp2);
    m_proxyModel->setFilterRegExp(regex);
    QModelIndex idx = m_proxyModel->index(0, 0);
    if(idx.isValid()) {
        m_list->clearSelection();
        m_list->setCurrentIndex(idx);
    }
}

void QWoSessionList::onListItemDoubleClicked(const QModelIndex &item)
{
    HostInfo hi = item.data(ROLE_HOSTINFO).value<HostInfo>();
    if(!hi.isValid()) {
        return;
    }
    qDebug() << "server:" << hi.name;
    switch (hi.type) {
    case SshWithSftp:
        emit readyToConnect(hi.name, EOT_SSH);
        break;
    case SftpOnly:
        emit readyToConnect(hi.name, EOT_SFTP);
        break;
    case Telnet:
        emit readyToConnect(hi.name, EOT_TELNET);
        break;
    case RLogin:
        emit readyToConnect(hi.name, EOT_RLOGIN);
        break;
    case Mstsc:
        emit readyToConnect(hi.name, EOT_MSTSC);
        break;
    case Vnc:
        emit readyToConnect(hi.name, EOT_VNC);
        break;
    case SerialPort:
        emit readyToConnect(hi.name, EOT_SERIALPORT);
        break;
    }
}

void QWoSessionList::onListCurrentItemChanged(const QModelIndex &item)
{
    HostInfo hi = item.data(ROLE_HOSTINFO).value<HostInfo>();
    if(hi.name == "") {
        return;
    }
    QString info;
    info.append(QString("%1 : %2\r\n").arg(tr("name")).arg(hi.name));
    info.append(QString("%1 : %2\r\n").arg(tr("host")).arg(hi.host));
    info.append(QString("%1 : %2").arg(tr("memo")).arg(hi.memo));

    m_info->setPlainText(info);
}

void QWoSessionList::onListReturnKeyPressed()
{
    QModelIndex idx = m_list->currentIndex();
    onListItemDoubleClicked(idx);
}

void QWoSessionList::onTimeout()
{
    if(m_countLeft < 0) {
        return;
    }
    QPoint screenPt = QCursor::pos();
    QPoint pt = mapFromGlobal(screenPt);
    QRect rt = rect();
    if(rt.contains(pt)) {
        m_countLeft = MAX_TRY_LEFT;
    }
    if(m_countLeft > 0) {
        m_countLeft--;
        return;
    }
    m_input->setText("");
}

void QWoSessionList::onEditReturnPressed()
{
    QString txt = m_input->text().trimmed();
    if(txt.isEmpty()) {
        return;
    }
    int cnt = m_proxyModel->rowCount();
    if(cnt <= 0) {
        return;
    }
    QModelIndex idx = m_list->currentIndex();
    onListItemDoubleClicked(idx);
}

void QWoSessionList::onListViewItemOpenSsh()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_SSH);
}

void QWoSessionList::onListViewItemOpenSftp()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_SFTP);
}

void QWoSessionList::onListViewItemOpenTelnet()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_TELNET);
}

void QWoSessionList::onListViewItemOpenRLogin()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_RLOGIN);
}

void QWoSessionList::onListViewItemOpenMstsc()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_MSTSC);
}

void QWoSessionList::onListViewItemOpenVnc()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_VNC);
}

void QWoSessionList::onListViewItemOpenSerialPort()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_SERIALPORT);
}

void QWoSessionList::onListViewItemReload()
{
    refreshList();
}

void QWoSessionList::onListViewItemModify()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    QWoSessionProperty dlg(this);
    dlg.setSession(hi.name);
    QObject::connect(&dlg, SIGNAL(readyToConnect(const QString&,int)), this, SIGNAL(readyToConnect(const QString&,int)));
    dlg.exec();
    refreshList();
}

void QWoSessionList::onListViewItemAdd()
{
    QWoSessionProperty dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(const QString&,int)), this, SIGNAL(readyToConnect(const QString&,int)));
    dlg.exec();
    refreshList();
}

void QWoSessionList::onListViewItemDelete()
{
    QMessageBox::StandardButton btn = QMessageBox::warning(this, "delete", "delete all the selective items?", QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        return ;
    }
    QItemSelectionModel *model = m_list->selectionModel();
    QModelIndexList idxs = model->selectedIndexes();
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        QWoSshConf::instance()->remove(name);
    }
    refreshList();
}

bool QWoSessionList::handleListViewContextMenu(QContextMenuEvent *ev)
{
    QItemSelectionModel *model = m_list->selectionModel();
    QModelIndexList idxs = model->selectedIndexes();

    QMenu menu(this);
    m_menu = &menu;

    if(idxs.isEmpty()) {
        menu.addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("ReloadAll"), this, SLOT(onListViewItemReload()));
        menu.addAction(QIcon(":/woterm/resource/skin/add.png"), tr("Add"), this, SLOT(onListViewItemAdd()));
    }else if(idxs.length() == 1) {
        QModelIndex idx = idxs.at(0);
        HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
        menu.addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("ReloadAll"), this, SLOT(onListViewItemReload()));
        menu.addAction(QIcon(":/woterm/resource/skin/add.png"), tr("Add"), this, SLOT(onListViewItemAdd()));
        if(hi.type == SshWithSftp) {
            menu.addAction(QIcon(":/woterm/resource/skin/ssh2.png"), tr("SshConnect"), this, SLOT(onListViewItemOpenSsh()));
        }
        if(hi.type == SshWithSftp || hi.type == SftpOnly) {
            menu.addAction(QIcon(":/woterm/resource/skin/sftp.png"), tr("SftpConnect"), this, SLOT(onListViewItemOpenSftp()));
        }
        if(hi.type == Telnet) {
            menu.addAction(QIcon(":/woterm/resource/skin/telnet.png"), tr("TelnetConnect"), this, SLOT(onListViewItemOpenTelnet()));
        }
        if(hi.type == RLogin) {
            menu.addAction(QIcon(":/woterm/resource/skin/rlogin.png"), tr("RLoginConnect"), this, SLOT(onListViewItemOpenRLogin()));
        }
        if(hi.type == Mstsc) {
            menu.addAction(QIcon(":/woterm/resource/skin/mstsc2.png"), tr("MstscConnect"), this, SLOT(onListViewItemOpenMstsc()));
        }
        if(hi.type == Vnc) {
            menu.addAction(QIcon(":/woterm/resource/skin/vnc2.png"), tr("VncConnect"), this, SLOT(onListViewItemOpenVnc()));
        }
        if(hi.type == SerialPort) {
            menu.addAction(QIcon(":/woterm/resource/skin/serialport.png"), tr("SerialConnect"), this, SLOT(onListViewItemOpenSerialPort()));
        }
        menu.addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onListViewItemModify()));
        menu.addAction(tr("Delete"), this, SLOT(onListViewItemDelete()));
    }else{
        menu.addAction(tr("Delete"), this, SLOT(onListViewItemDelete()));
    }
    m_menu->exec(ev->globalPos());
    return true;
}

void QWoSessionList::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

bool QWoSessionList::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type t = ev->type();
    if(obj == m_list) {
        if(t == QEvent::ContextMenu) {
            return handleListViewContextMenu(dynamic_cast<QContextMenuEvent*>(ev));
        }else if(t == QEvent::Enter || t == QEvent::Leave) {
            if(!m_input->text().isEmpty()){
                m_countLeft = MAX_TRY_LEFT;
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}
