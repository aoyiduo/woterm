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

#include "qwosessionmanage.h"
#include "ui_qwosessionmanage.h"
#include "qwohostsimplelist.h"
#include "qkxtermwidget.h"
#include "qwosessionproperty.h"

#include "qwosetting.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qwotreeview.h"
#include "qwoidentify.h"

#include <QFileDialog>
#include <QMenu>
#include <QDebug>
#include <QIntValidator>
#include <QStringListModel>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QContextMenuEvent>

QWoSessionManage::QWoSessionManage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSessionManage)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle(tr("Session List"));

    m_model = QWoHostListModel::instance();
    m_model->setMaxColumnCount(4);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    QVBoxLayout *layout = new QVBoxLayout(ui->frame);
    ui->frame->setLayout(layout);
    ui->frame->setFrameShape(QFrame::NoFrame);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_tree = new QWoTreeView(ui->frame);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    layout->addWidget(m_tree);
    m_tree->installEventFilter(this);

    m_tree->setModel(m_proxyModel);
    int w = m_model->widthColumn(m_tree->font(), 0);
    if(w < 100) {
        w = 100;
    }
    m_tree->setColumnWidth(0, w+30);
    m_tree->setColumnWidth(1, 200);
    m_tree->setIconSize(QSize(24,24));

    QObject::connect(ui->filter, SIGNAL(textChanged(const QString&)), this, SLOT(onEditTextChanged(const QString&)));
    QObject::connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(onOpenReady()));
    QObject::connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(onDeleteReady()));
    QObject::connect(ui->btnModify, SIGNAL(clicked()), this, SLOT(onModifyReady()));
    QObject::connect(ui->btnNew, SIGNAL(clicked()), this, SLOT(onNewReady()));
    QObject::connect(m_tree, SIGNAL(itemChanged(const QModelIndex&)), this, SLOT(onTreeItemSelected(const QModelIndex&)));
    QObject::connect(m_tree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onTreeItemDoubleClicked(const QModelIndex&)));

    ui->filter->clear();
    ui->filter->setPlaceholderText(tr("Enter keyword to search target quickly"));
}

QWoSessionManage::~QWoSessionManage()
{
    delete ui;
}

void QWoSessionManage::onEditTextChanged(const QString &txt)
{
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
}

void QWoSessionManage::onSshConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_SSH);
    close();
}

void QWoSessionManage::onSftpConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_SFTP);
    close();
}

void QWoSessionManage::onTelnetConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_TELNET);
    close();
}

void QWoSessionManage::onRLoginConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_RLOGIN);
    close();
}

void QWoSessionManage::onMstscConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_MSTSC);
    close();
}

void QWoSessionManage::onVncConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_VNC);
    close();
}

void QWoSessionManage::onSerialPortConnectReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
        break;
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets.at(0), EOT_SERIALPORT);
    close();
}

void QWoSessionManage::onOpenReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    if(idxs.isEmpty()) {
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    QModelIndex idx = idxs.at(0);
    onTreeItemDoubleClicked(idx);
}


void QWoSessionManage::onDeleteReady()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    if(idxs.isEmpty()) {
        QMessageBox::warning(this, "delete", "no selection?", QMessageBox::Ok|QMessageBox::No);
        return;
    }
    QMessageBox::StandardButton btn = QMessageBox::warning(this, "delete", "delete all the selective items?", QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        return ;
    }
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        QWoSshConf::instance()->remove(name);
    }
    refreshList();
}

void QWoSessionManage::onModifyReady()
{
    QModelIndex idx = m_tree->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("Modify"), tr("No Selection"));
        return;
    }
    QVariant host = idx.data(ROLE_HOSTINFO);
    if(!host.isValid()) {
        return;
    }
    HostInfo hi = host.value<HostInfo>();
    QWoSessionProperty dlg(this);
    dlg.setSession(hi.name);
    dlg.exec();
    refreshList();
}

void QWoSessionManage::onNewReady()
{
    QWoSessionProperty dlg(this);
    dlg.exec();
    refreshList();
}

void QWoSessionManage::onImportReady()
{
#if 0
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import File"));
    if(fileName.isEmpty()) {
        return;
    }

    QWoSshConf conf(fileName, this);
    conf.refresh();
    QList<HostInfo> hosts = conf.hostList();
    if(hosts.isEmpty()) {
        QMessageBox::warning(this, tr("warning"), tr("it's not a ssh config format: %1").arg(fileName));
        return;
    }
    QMap<QString, IdentifyInfo> all = QWoIdentify::all();
    QMap<QString, QString> needsImport;
    QMap<QString, QString> path2figure;
    for(int i = 0; i < hosts.length(); i++) {
        HostInfo hi = hosts.at(i);
        if(!hi.identityFile.isEmpty()) {
            QDir dir;
            QString path = hi.identityFile;
            if(path.startsWith('~')) {
                path=QDir::homePath() + "/" + path.remove(0, 1);
            }
            path = QDir::cleanPath(path);
            if(!QFile::exists(path)) {
                QMessageBox::warning(this, tr("warning"), tr("failed to find the identify file list in config file for bad path: %1").arg(path));
                return;
            }
            IdentifyInfo info;
            if(!QWoIdentify::infomation(path, &info)) {
                QMessageBox::warning(this, tr("warning"), tr("bad identify file: %1").arg(path));
                return;
            }
            if(!all.contains(info.fingureprint)){
                needsImport.insert(info.path, info.path);
            }
            path2figure.insert(info.path, info.fingureprint);
        }
    }
    if(!needsImport.isEmpty()) {
        QString items = needsImport.keys().join("\r\n");
        QMessageBox::warning(this, tr("warning"), tr("The config file contain the follow identify's files, Please export them before do this action.")+QString("%1").arg(items));
        return;
    }
    //qDebug() << his;    
    QWoSshConf *gconf = QWoSshConf::instance();    
    for(int i = 0; i < hosts.length(); i++) {
        HostInfo hi = hosts.at(i);
        if(!hi.identityFile.isEmpty()) {
            QString path = hi.identityFile;
            if(path.startsWith('~')) {
                path=QDir::homePath() + "/" + path.remove(0, 1);
            }
            path = QDir::cleanPath(path);
            IdentifyInfo info = all.value(path2figure.value(path));
            hi.identityFile = "woterm:" + QWoUtils::nameToPath(info.name);
        }
        if(!gconf->append(hi)) {
            QString name = hi.name;            
            for(int j = 1; j < 100; j++){
                hi.name = QString("%1-%2").arg(name).arg(j);
                if(gconf->append(hi)) {
                    break;
                }
            }
        }
    }
    refreshList();
#endif
}

void QWoSessionManage::onTreeViewOpenInSamePage()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    if(idxs.length() > 6) {
        QMessageBox::information(this, tr("Info"), tr("can't open over 6 session in same page."));
        return;
    }
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets, true);
    close();
}

void QWoSessionManage::onTreeViewOpenInDifferentPage()
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QStringList targets;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QString name = idx.data().toString();
        targets.append(name);
    }
    if(targets.isEmpty()){
        QMessageBox::warning(this, tr("Info"), tr("no selection"));
        return;
    }
    emit readyToConnect(targets, false);
    close();
}

void QWoSessionManage::onTreeItemSelected(const QModelIndex &idx)
{
    if(!idx.isValid()) {
        return;
    }
    HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
}

void QWoSessionManage::onTreeItemDoubleClicked(const QModelIndex &idx)
{
    if(!idx.isValid()) {
        return;
    }
    HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    switch(hi.type) {
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
    close();
}

void QWoSessionManage::refreshList()
{
    QWoSshConf::instance()->refresh();
    m_model->refreshList();
}

bool QWoSessionManage::handleTreeViewContextMenu(QContextMenuEvent *ev)
{
    QItemSelectionModel *model = m_tree->selectionModel();
    QModelIndexList idxs = model->selectedRows();

    QMenu menu(this);

    if(idxs.isEmpty()) {
        menu.addAction(QIcon(":/woterm/resource/skin/add.png"), tr("Add"), this, SLOT(onNewReady()));
    } else if(idxs.length() == 1) {
        QModelIndex idx = idxs.at(0);
        HostInfo hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
        menu.addAction(QIcon(":/woterm/resource/skin/add.png"), tr("Add"), this, SLOT(onNewReady()));
        if(hi.type == SshWithSftp) {
            menu.addAction(QIcon(":/woterm/resource/skin/ssh2.png"), tr("SshConnect"), this, SLOT(onSshConnectReady()));
        }
        if(hi.type == SshWithSftp || hi.type == SftpOnly) {
            menu.addAction(QIcon(":/woterm/resource/skin/sftp.png"), tr("SftpConnect"), this, SLOT(onSftpConnectReady()));
        }
        if(hi.type == Telnet) {
            menu.addAction(QIcon(":/woterm/resource/skin/telnet.png"), tr("TelnetConnect"), this, SLOT(onTelnetConnectReady()));
        }
        if(hi.type == RLogin) {
            menu.addAction(QIcon(":/woterm/resource/skin/rlogin.png"), tr("RLoginConnect"), this, SLOT(onRLoginConnectReady()));
        }
        if(hi.type == Mstsc) {
            menu.addAction(QIcon(":/woterm/resource/skin/mstsc2.png"), tr("MstscConnect"), this, SLOT(onMstscConnectReady()));
        }
        if(hi.type == Vnc) {
            menu.addAction(QIcon(":/woterm/resource/skin/vnc2.png"), tr("VncConnect"), this, SLOT(onVncConnectReady()));
        }
        if(hi.type == SerialPort) {
            menu.addAction(QIcon(":/woterm/resource/skin/serialport.png"), tr("SerialConnect"), this, SLOT(onSerialPortConnectReady()));
        }
        menu.addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyReady()));
        menu.addAction(tr("Delete"), this, SLOT(onDeleteReady()));
    }else{
        menu.addAction(tr("Delete"), this, SLOT(onDeleteReady()));
    }
    menu.exec(ev->globalPos());
    return true;
}

bool QWoSessionManage::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type t = ev->type();
    if(obj == m_tree) {
        if(t == QEvent::ContextMenu) {
            return handleTreeViewContextMenu(dynamic_cast<QContextMenuEvent*>(ev));
        }
    }
    return QDialog::eventFilter(obj, ev);
}
