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
#include "qwohosttreemodel.h"
#include "qwotreeview.h"
#include "qwosessionproperty.h"
#include "qwogroupinputdialog.h"
#include "qwosortfilterproxymodel.h"
#include "qkxmessagebox.h"
#include "qkxver.h"
#include "qwoutils.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QApplication>
#include <QHeaderView>
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
    m_tree = new QWoTreeView(this);
    m_input = new QLineEdit(this);
    m_info = new QPlainTextEdit(this);
    m_btnModel = new QPushButton(this);
    m_btnModel->setObjectName("modelLayout");
    m_btnModel->setIconSize(QSize(16,16));
    QObject::connect(m_btnModel, SIGNAL(clicked()), this, SLOT(onListViewGroupLayout()));
    hlayout->setContentsMargins(3, 0, 0, 0);
    hlayout->addWidget(m_btnModel);
    hlayout->addWidget(m_input);
    layout->addWidget(m_tree);
    layout->addWidget(m_info);
    layout->setSpacing(8);
    m_info->setReadOnly(true);
    m_info->setFixedHeight(150);
    m_tree->installEventFilter(this);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tree->setIndentation(10);
    m_tree->setHeaderHidden(true);

    m_listModel = QWoHostListModel::instance();
    m_treeModel = QWoHostTreeModel::instance();

    QObject::connect(m_treeModel.data(), &QAbstractItemModel::modelReset, this, [=](){
        tryToRestoreSessionsExpandState();
    });

    m_proxyModel = new QWoSortFilterProxyModel(1, this);
    m_proxyModel->setRecursiveFilteringEnabled(true);

    if(QWoSetting::isListModel("docker")) {
        m_btnModel->setIcon(QIcon("../private/skins/black/list.png"));
        m_proxyModel->setSourceModel(m_listModel);
        m_model = m_listModel;
    }else{
        m_btnModel->setIcon(QIcon("../private/skins/black/tree.png"));
        m_proxyModel->setSourceModel(m_treeModel);
        m_model = m_treeModel;
    }    
    m_btnModel->setVisible(true);
    m_tree->setModel(m_proxyModel);

    QObject::connect(m_input, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    QObject::connect(m_input, SIGNAL(textChanged(QString)), this, SLOT(onEditTextChanged(QString)));
    QObject::connect(m_tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onListItemDoubleClicked(QModelIndex)));
    QObject::connect(m_tree, SIGNAL(itemChanged(QModelIndex)), this, SLOT(onListCurrentItemChanged(QModelIndex)));
    QObject::connect(m_tree, SIGNAL(returnKeyPressed()), this, SLOT(onListReturnKeyPressed()));

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(3000);
    m_countLeft = -1;
    m_input->setPlaceholderText(tr("Enter keyword to search"));
    m_input->clear();

    m_input->installEventFilter(this);

    QObject::connect((QWoTreeView*)m_tree, &QTreeView::expanded, this, [=](const QModelIndex& idx){
        QVariant group = idx.data(ROLE_GROUP);
        if(!group.isValid()) {
            return ;
        }
        QString txt = m_input->text();
        if(!txt.isEmpty()) {
            return ;
        }
        saveSessionsExpandState();
    });
    QObject::connect((QWoTreeView*)m_tree, &QTreeView::collapsed, this, [=](const QModelIndex& idx){
        QVariant group = idx.data(ROLE_GROUP);
        if(!group.isValid()) {
            return ;
        }
        QString txt = m_input->text();
        if(!txt.isEmpty()) {
            return ;
        }
        saveSessionsExpandState();
    });
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
    QWoSshConf::instance()->refresh();
    if(m_model == m_listModel) {
        m_listModel->refreshList();
    }else{
        m_treeModel->refreshList();
    }
    tryToRestoreSessionsExpandState();
}

void QWoSessionList::tryToConnect(const HostInfo &hi)
{
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
    m_proxyModel->setFilterRole(ROLE_REFILTER);
    QModelIndex idx = m_proxyModel->index(0, 0);
    if(idx.isValid()) {
        m_tree->clearSelection();
        m_tree->setCurrentIndex(idx);
    }
    if(txt.isEmpty()) {
        restoreSessionsExpandState();
    }else{
        QMetaObject::invokeMethod(m_tree, "expandAll", Qt::QueuedConnection);
    }
}

void QWoSessionList::onListItemDoubleClicked(const QModelIndex &item)
{
    HostInfo hi = item.data(ROLE_HOSTINFO).value<HostInfo>();
    if(!hi.isValid()) {
        QVariant v = item.data(ROLE_GROUP);
        if(!v.isValid()) {
            return;
        }
        return;
    }
    tryToConnect(hi);
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
    QModelIndex idx = m_tree->currentIndex();
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
    int cnt = m_proxyModel->rowCount();
    if(cnt <= 0) {
        return;
    }
    QModelIndex idx = m_tree->currentIndex();
    onListItemDoubleClicked(idx);
}

void QWoSessionList::onListViewItemOpenSsh()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_SSH);
}

void QWoSessionList::onListViewItemOpenSftp()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_SFTP);
}

void QWoSessionList::onListViewItemOpenTelnet()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_TELNET);
}

void QWoSessionList::onListViewItemOpenRLogin()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_RLOGIN);
}

void QWoSessionList::onListViewItemOpenMstsc()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_MSTSC);
}

void QWoSessionList::onListViewItemOpenVnc()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    emit readyToConnect(hi.name, EOT_VNC);
}

void QWoSessionList::onListViewItemOpenSerialPort()
{
    QModelIndex idx = m_tree->currentIndex();
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
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    QVariant group = idx.data(ROLE_GROUP);
    if(group.isValid()) {
        const GroupInfo& gi = group.value<GroupInfo>();
        QStringList names = QWoSshConf::instance()->groupNameList();
        QWoGroupInputDialog dlg(gi.name, gi.order, this);
        QPointer<QWoGroupInputDialog> dlgPtr(&dlg);
        QObject::connect(&dlg, &QWoGroupInputDialog::apply, this, [&](const QString& name, int order){
            if(gi.name != name) {
                // modify group name.
                QWoSshConf::instance()->updateGroup(gi.name, order);
                QWoSshConf::instance()->renameGroup(name, gi.name);
                QWoSshConf::instance()->renameServerGroup(name, gi.name);
            }else{
                QWoSshConf::instance()->updateGroup(name, order);
            }
            dlgPtr->done(QDialog::Accepted);
        });
        if(dlg.exec() != QDialog::Accepted){
            return;
        }
    }else{
        const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
        QWoSessionProperty dlg(this);
        if(!dlg.setSession(hi.name)) {
            return;
        }
        QObject::connect(&dlg, SIGNAL(readyToConnect(QString,int)), this, SIGNAL(readyToConnect(QString,int)));
        dlg.exec();
    }
    refreshList();
}

void QWoSessionList::onListViewItemAdd()
{
    QWoSessionProperty dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(QString,int)), this, SIGNAL(readyToConnect(QString,int)));
    dlg.exec();
    refreshList();
}

void QWoSessionList::onListViewItemCopy()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    hi.name.append(".Copy");
    if(!QWoSshConf::instance()->exists(hi.name)) {
        QWoSshConf::instance()->append(hi);
        return;
    }

    for(int i = 2; i < 100; i++) {
        QString name = QString("%1%2").arg(hi.name).arg(i);
        if(QWoSshConf::instance()->exists(name)) {
            continue;
        }
        hi.name = name;
        QWoSshConf::instance()->append(hi);
        return;
    }
}

void QWoSessionList::onListViewItemDelete()
{
    QMessageBox::StandardButton btn = QKxMessageBox::warning(this, "delete", "delete all the selective items?", QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        return ;
    }
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedIndexes();
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QVariant group = idx.data(ROLE_GROUP);
        if(group.isValid()) {
            const GroupInfo& gi = group.value<GroupInfo>();
            QWoSshConf::instance()->removeServerByGroup(gi.name);
            QWoSshConf::instance()->removeGroup(gi.name);
        }else{
            QString name = idx.data().toString();
            QWoSshConf::instance()->removeServer(name);
        }
    }
    refreshList();
}

void QWoSessionList::onListViewGroupLayout()
{
    if(m_model == m_listModel) {
        m_btnModel->setIcon(QIcon("../private/skins/black/tree.png"));
        m_proxyModel->setSourceModel(m_treeModel);
        m_model = m_treeModel;
        QWoSetting::setListModel("docker", false);
        tryToRestoreSessionsExpandState();
    }else{
        m_btnModel->setIcon(QIcon("../private/skins/black/list.png"));
        m_proxyModel->setSourceModel(m_listModel);
        m_model = m_listModel;
        QWoSetting::setListModel("docker", true);        
    }
}

bool QWoSessionList::handleListViewContextMenu(QContextMenuEvent *ev)
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedIndexes();

    QMenu menu(this);
    m_menu = &menu;

    if(idxs.isEmpty()) {
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("ReloadAll"), this, SLOT(onListViewItemReload()));
        menu.addAction(QIcon("../private/skins/black/add.png"), tr("Add"), this, SLOT(onListViewItemAdd()));
    }else if(idxs.length() == 1) {
        QModelIndex idx = idxs.at(0);
        HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("ReloadAll"), this, SLOT(onListViewItemReload()));
        menu.addAction(QIcon("../private/skins/black/add.png"), tr("Add"), this, SLOT(onListViewItemAdd()));
        menu.addAction(QIcon("../private/skins/black/ftp.png"), tr("Copy"), this, SLOT(onListViewItemCopy()));
        if(hi.type == SshWithSftp) {
            menu.addAction(QIcon("../private/skins/black/ssh2.png"), tr("SshConnect"), this, SLOT(onListViewItemOpenSsh()));
        }
        if(hi.type == SshWithSftp || hi.type == SftpOnly) {
            menu.addAction(QIcon("../private/skins/black/sftp.png"), tr("SftpConnect"), this, SLOT(onListViewItemOpenSftp()));
        }
        if(hi.type == Telnet) {
            menu.addAction(QIcon("../private/skins/black/telnet.png"), tr("TelnetConnect"), this, SLOT(onListViewItemOpenTelnet()));
        }
        if(hi.type == RLogin) {
            menu.addAction(QIcon("../private/skins/black/rlogin.png"), tr("RLoginConnect"), this, SLOT(onListViewItemOpenRLogin()));
        }
        if(hi.type == Mstsc) {
            menu.addAction(QIcon("../private/skins/black/mstsc2.png"), tr("MstscConnect"), this, SLOT(onListViewItemOpenMstsc()));
        }
        if(hi.type == Vnc) {
            menu.addAction(QIcon("../private/skins/black/vnc2.png"), tr("VncConnect"), this, SLOT(onListViewItemOpenVnc()));
        }
        if(hi.type == SerialPort) {
            menu.addAction(QIcon("../private/skins/black/serialport.png"), tr("SerialConnect"), this, SLOT(onListViewItemOpenSerialPort()));
        }
        menu.addAction(QIcon("../private/skins/black/palette.png"), tr("Edit"), this, SLOT(onListViewItemModify()));
        menu.addAction(tr("Delete"), this, SLOT(onListViewItemDelete()));
    }else{
        menu.addAction(QIcon("../private/skins/black/ftp.png"), tr("Copy"), this, SLOT(onListViewItemCopy()));
        menu.addAction(tr("Delete"), this, SLOT(onListViewItemDelete()));
    }
    {
        if(m_model == m_listModel) {
            menu.addAction(tr("Show tree mode"), this, SLOT(onListViewGroupLayout()));
        }else{
            menu.addAction(tr("Show list mode"), this, SLOT(onListViewGroupLayout()));
        }
    }
    m_menu->exec(ev->globalPos());
    return true;
}

void QWoSessionList::tryToRestoreSessionsExpandState()
{
    QString txt = m_input->text();
    if(txt.isEmpty() && m_model == m_treeModel) {
        restoreSessionsExpandState();
    }else{
        QPointer<QWoSessionList> that = this;
        QTimer::singleShot(100, this, [=](){
            if(that == nullptr) {
                return ;
            }
            onEditTextChanged(txt);
        });
    }
}

void QWoSessionList::restoreSessionsExpandState()
{
    QPointer<QWoSessionList> that = this;
    QTimer::singleShot(100, this, [=](){
        if(that == nullptr) {
            return ;
        }
        QAbstractItemModel *model = m_tree->model();
        if(model == nullptr) {
            return;
        }
        QStringList expands = QWoSetting::sessionsGroupExpand("docker");
        for(int i = 0; i < model->rowCount(); i++) {
            QModelIndex idx = model->index(i, 0);
            QVariant v = idx.data(ROLE_GROUP);
            if(!v.isValid()) {
                continue;
            }
            const GroupInfo& gi = v.value<GroupInfo>();
            if(expands.contains(gi.name)) {
                m_tree->expand(idx);
            }else{
                m_tree->collapse(idx);
            }
        }
    });
}

void QWoSessionList::saveSessionsExpandState()
{
    QPointer<QWoSessionList> that = this;
    QTimer::singleShot(100, this, [=](){
        if(that == nullptr) {
            return ;
        }
        QAbstractItemModel *model = m_tree->model();
        if(model == nullptr) {
            return;
        }
        QStringList expands;
        for(int i = 0; i < model->rowCount(); i++) {
            QModelIndex idx = model->index(i, 0);
            QVariant v = idx.data(ROLE_GROUP);
            if(!v.isValid()) {
                continue;
            }
            const GroupInfo& gi = v.value<GroupInfo>();
            if(m_tree->isExpanded(idx)) {
                expands.append(gi.name);
            }
        }
        QWoSetting::setSessionsGroupExpand("docker", expands);
    });
}

void QWoSessionList::handleFilterInputKeyEvent(QKeyEvent *ke)
{
    int key = ke->key();

    if(key == Qt::Key_Up) {
        QModelIndex idx = prev(m_tree->currentIndex());
        if(idx.isValid()) {
            m_tree->clearSelection();
            m_tree->setCurrentIndex(idx);
        }
    }else if(key == Qt::Key_Down) {
        QModelIndex idx = next(m_tree->currentIndex());
        if(idx.isValid()) {
            m_tree->clearSelection();
            m_tree->setCurrentIndex(idx);
        }
    }else if(key == Qt::Key_Right) {
        m_tree->expand(m_tree->currentIndex());
    }else if(key == Qt::Key_Left) {
        m_tree->collapse(m_tree->currentIndex());
    }else if(key == Qt::Key_Return || key == Qt::Key_Enter) {
        QModelIndex idx = m_tree->currentIndex();
        QVariant group = idx.data(ROLE_GROUP);
        if(group.isValid()) {
            if(m_tree->isExpanded(idx)) {
                m_tree->collapse(idx);
            }else{
                m_tree->expand(idx);
            }
        }
    }
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
    if(obj == m_tree) {
        if(t == QEvent::ContextMenu) {
            return handleListViewContextMenu(dynamic_cast<QContextMenuEvent*>(ev));
        }else if(t == QEvent::Enter || t == QEvent::Leave) {
            if(!m_input->text().isEmpty()){
                m_countLeft = MAX_TRY_LEFT;
            }
        }else if(t == QEvent::Show) {
            tryToRestoreSessionsExpandState();
        }
    }else if(obj == m_input) {
        if(t == QEvent::KeyPress) {
            handleFilterInputKeyEvent((QKeyEvent*)ev);
        }
    }
    return QWidget::eventFilter(obj, ev);
}

QModelIndex QWoSessionList::first()
{
    return m_proxyModel->index(0, 0);
}

QModelIndex QWoSessionList::next(const QModelIndex &idx)
{
    if(!idx.isValid()) {
        return first();
    }
    QVariant group = idx.data(ROLE_GROUP);
    if(group.isValid()) {
        // group.
        if(m_tree->isExpanded(idx)) {
            return idx.child(0, 0);
        }
    }
    QModelIndex nidx = idx.siblingAtRow(idx.row()+1);
    if(!nidx.isValid()) {
        if(!group.isValid()) {
            QModelIndex pidx = idx.parent();
            return pidx.siblingAtRow(pidx.row()+1);
        }
    }
    return nidx;
}

QModelIndex QWoSessionList::prev(const QModelIndex &idx)
{
    if(!idx.isValid()) {
        return first();
    }
    QVariant state = idx.data(ROLE_GROUP);
    if(state.isValid()) {
        // group.
        QModelIndex gidx = idx.siblingAtRow(idx.row()-1);
        if(m_tree->isExpanded(gidx)) {
            int cnt = m_proxyModel->rowCount(gidx);
            return gidx.child(cnt - 1, 0);
        }
        return gidx;
    }
    QModelIndex nidx = idx.siblingAtRow(idx.row()-1);
    if(!nidx.isValid()) {
        QVariant group = idx.data(ROLE_GROUP);
        if(!group.isValid()) {
            return idx.parent();
        }
    }
    return nidx;
}
