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

#include "qkxfilterlistview.h"
#include "ui_qkxfilterlistview.h"
#include "qwohostlistmodel.h"
#include "qwosortfilterproxymodel.h"
#include "qwoutils.h"
#include "qwosshconf.h"

#include <QLineEdit>
#include <QDebug>
#include <QListView>
#include <QMouseEvent>
#include <QDateTime>
#include <QToolButton>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QGraphicsColorizeEffect>


#define BUTTON_REMOVE_SIZE      (13)
#define BUTTON_MARGIN           (3)

QSessionButtonActionDelegate::QSessionButtonActionDelegate(QListView *lstView, QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_listView(lstView)
{
    m_btnSftp = new QToolButton(parent);
    m_btnSftp->setIcon(QIcon("../private/skins/black/sftp.png"));
    m_btnSftp->setIconSize(QSize(BUTTON_REMOVE_SIZE,BUTTON_REMOVE_SIZE));

    QSize sz(BUTTON_REMOVE_SIZE,BUTTON_REMOVE_SIZE);
    m_btnSftp->resize(sz);
    m_btnSftp->hide();
}

bool QSessionButtonActionDelegate::editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx)
{
    _editorEvent(ev, model, option, idx);
    return true;
}

void QSessionButtonActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const
{
    QString label = idx.data(Qt::DisplayRole).toString();
    QStyledItemDelegate::paint(painter, option, idx);


    if (!(option.state & QStyle::State_Selected)){
        return;
    }

    const HostInfo& hi = QWoSshConf::instance()->find(label);
    if(!hi.isValid()) {
        return;
    }

    if(hi.type != SshWithSftp) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPalette pal = option.palette;
    painter->setBrush(Qt::NoBrush);
    QToolButton *btn = m_btnSftp;
    QStyleOptionButton button;
    button.state |= QStyle::State_Enabled;
    if(m_event == QEvent::MouseButtonPress) {
        button.state |= QStyle::State_Sunken;
    }else if(m_event == QEvent::MouseButtonRelease) {
    }else if(m_event == QEvent::MouseMove) {
        button.state |= QStyle::State_MouseOver;
    }
    *((QEvent::Type*)&m_event) = QEvent::None;

    QRect rt = buttonRect(option);
    painter->setBrush(QColor(Qt::white));
    painter->setPen(option.palette.highlight().color());
    painter->drawRoundRect(rt, 16, 16);
    button.rect = rt;
    button.text = btn->text();
    button.icon = btn->icon();
    button.iconSize = btn->iconSize();
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter, btn);
    painter->restore();
}

bool QSessionButtonActionDelegate::_editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx)
{
    QMouseEvent *me = static_cast<QMouseEvent *> (ev);
    m_ptMouse = me->pos();
    QGuiApplication::restoreOverrideCursor();
    QRect rt = buttonRect(option);
    QApplication::restoreOverrideCursor();
    if(!rt.contains(m_ptMouse)) {
        m_event = QEvent::None;
        return true;
    }
    QEvent::Type t = me->type();
    if(t == QEvent::MouseMove
            || t == QEvent::MouseButtonRelease) {
        m_event = t;
    }else if(t == QEvent::MouseButtonPress) {
        m_event = t;
        emit sftpArrived(idx);
    }
    return true;
}

QRect QSessionButtonActionDelegate::buttonRect(const QStyleOptionViewItem &option) const
{
    QRect rt = m_btnSftp->rect();
    rt.adjust(-3, -3, 3, 3);
    QRect itemRt = option.rect;
    rt.moveCenter(itemRt.center());
    rt.translate((itemRt.right() - rt.width()) / 2, 0);
    return rt;
}


QKxFilterListView::QKxFilterListView(QLineEdit *input, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QKxFilterListView)
    , m_input(input)
    , m_filterEnable(true)
{
    setAttribute(Qt::WA_StyledBackground);
    ui->setupUi(this);
    input->installEventFilter(this);
    QStyledItemDelegate *delegate = new QSessionButtonActionDelegate(ui->listView, ui->listView);
    QObject::connect(delegate, SIGNAL(sftpArrived(QModelIndex)), this, SLOT(onDelegateSftpArrived(QModelIndex)));
    ui->listView->setItemDelegate(delegate);
    m_grab = ui->listView->viewport();
    m_grab->installEventFilter(this);
    m_grab->setMouseTracking(true);
#if 0
    int mh = ui->btnSSH->height();
    ui->btnSSH->setMinimumHeight(mh);
    ui->btnSFTP->setMinimumHeight(mh);
    ui->btnTelnet->setMinimumHeight(mh);
    ui->btnVNC->setMinimumHeight(mh);
    ui->btnAdd->setMinimumHeight(mh);
    ui->btnSSH->setObjectName("kxFilterList");
    ui->btnSFTP->setObjectName("kxFilterList");
    ui->btnTelnet->setObjectName("kxFilterList");
    ui->btnVNC->setObjectName("kxFilterList");
    ui->btnAdd->setObjectName("kxFilterList");
#else
    QWoUtils::setLayoutVisible(ui->panel, false);
#endif

    m_filter = new QWoSortFilterProxyModel(1, this);
    m_filter->setSourceModel(QWoHostListModel::instance());
    m_filter->setRecursiveFilteringEnabled(true);
    ui->listView->setModel(m_filter);
    ui->listView->setMinimumHeight(0);
    setMinimumHeight(0);
    setContentsMargins(0, 0, 0, 0);
    QObject::connect(input, SIGNAL(textChanged(QString)), this, SLOT(onEditTextChanged(QString)));
    adjustSize();
    hide();

    QObject::connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onListItemClicked(QModelIndex)));
}

QKxFilterListView::~QKxFilterListView()
{
    delete ui;
}

void QKxFilterListView::enableFilter(bool on)
{
    m_filterEnable = on;
}

QListView *QKxFilterListView::listView()
{
    return ui->listView;
}

void QKxFilterListView::onEditTextChanged(const QString &txt)
{
    if(!m_filterEnable) {
        return;
    }
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
    m_filter->setFilterRegExp(regex);
    m_filter->setFilterRole(ROLE_REFILTER);
    int cnt = m_filter->rowCount();
    if(cnt > 0) {
        QModelIndex idx = m_filter->index(0, 0);
        ui->listView->setCurrentIndex(idx);
    }
    QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
}

void QKxFilterListView::onListItemClicked(const QModelIndex &index)
{
    if(!m_sftpFirstResponse.isEmpty()) {
        return;
    }
    const HostInfo& hi = index.data(ROLE_HOSTINFO).value<HostInfo>();
    if(hi.type == SshWithSftp) {
        emit itemClicked(hi.name, EOT_SSH);
    }else if(hi.type == SftpOnly) {
        emit itemClicked(hi.name, EOT_SFTP);
    }else if(hi.type == Telnet) {
        emit itemClicked(hi.name, EOT_TELNET);
    }else if(hi.type == RLogin) {
        emit itemClicked(hi.name, EOT_RLOGIN);
    }else if(hi.type == Mstsc) {
        emit itemClicked(hi.name, EOT_MSTSC);
    }else if(hi.type == Vnc) {
        emit itemClicked(hi.name, EOT_VNC);
    }
    hide();
}

void QKxFilterListView::onDelegateSftpArrived(const QModelIndex &idx)
{
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    if(!hi.isValid()) {
        return;
    }
    m_sftpFirstResponse = hi.name;
    emit itemClicked(hi.name, EOT_SFTP);
    //qDebug() << "onDelegateSftpArrived" << idx.data();
    hide();
}

bool QKxFilterListView::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type type = ev->type();
    if(obj == m_input) {
        if(type == QEvent::FocusIn) {
        }else if(type == QEvent::FocusOut){
            hide();
        }
    }else if(obj == m_grab) {
        if(type == QEvent::Show) {
            handleShowEvent(static_cast<QShowEvent*>(ev));
        }else if(type == QEvent::Hide) {
            handleHideEvent(static_cast<QHideEvent*>(ev));
        }else if(type == QEvent::MouseButtonPress) {
            handleMousePressEvent(static_cast<QMouseEvent*>(ev));
        }else if(type == QEvent::MouseMove) {
            handleMouseMoveEvent(static_cast<QMouseEvent*>(ev));
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QKxFilterListView::resetPosition()
{
    QString keys = m_input->text();
    int minHeight = 0;
#if 0
    if(keys.contains(QChar::Space) || keys.isEmpty()) {
        ui->btnSFTP->hide();
        ui->btnSSH->hide();
        ui->btnTelnet->hide();
        ui->btnVNC->hide();
        minHeight = 0;
    }else if(keys.endsWith(":22")) {
        ui->btnSFTP->show();
        ui->btnSSH->show();
        ui->btnTelnet->hide();
        ui->btnVNC->hide();
        minHeight = ui->btnSSH->height();
    }else if(keys.endsWith(":23")) {
        ui->btnSFTP->hide();
        ui->btnSSH->hide();
        ui->btnTelnet->show();
        ui->btnVNC->hide();
        minHeight = ui->btnTelnet->height();
    }else if(keys.contains(":59")) {
        ui->btnSFTP->hide();
        ui->btnSSH->hide();
        ui->btnTelnet->hide();
        ui->btnVNC->show();
        minHeight = ui->btnVNC->height();
    }else{
        ui->btnSFTP->show();
        ui->btnSSH->show();
        ui->btnTelnet->show();
        ui->btnVNC->show();
        minHeight = ui->btnVNC->height();
    }
#endif
    QWidget *parent = parentWidget();
    QRect rtInput = m_input->rect();
    QPoint pt = m_input->mapTo(parent, rtInput.bottomLeft());
    int mh = minimizeHeight(minHeight);
    QRect rt = QRect(0, 0, rtInput.width(), mh);
    qDebug() << "resetPosition" << rt;
    rt.moveTo(pt+QPoint(0,1));
    setGeometry(rt);
    setFixedHeight(mh);
    show();
    raise();
}

int QKxFilterListView::minimizeHeight(int mh)
{
    int count = m_filter->rowCount();
    ui->listView->setVisible(count > 0);
    if(count <= 0) {
        return mh;
    }
    QModelIndex idx = m_filter->index(0, 0);
    QVariant vsz = m_filter->data(idx, Qt::SizeHintRole);
    int itemHeight = 25;
    if(vsz.isValid()) {
        itemHeight = vsz.toSize().height();
    }
    int cnt = qMin<int>(count, 10);
    int cntHeight = cnt * itemHeight;
    QMargins m = ui->listView->contentsMargins();
    int height_actual = cntHeight + ui->listView->lineWidth() * 2 + m.top() + m.bottom()  + mh;
    //qDebug() << "cnt" << cnt << "count" << count << itemHeight << cntHeight << mh << m.top() << m.bottom() << height_actual;
    return height_actual;
}

void QKxFilterListView::handleShowEvent(QShowEvent *ev)
{
    m_sftpFirstResponse.clear();
    QWidget::showEvent(ev);
    m_grab->grabMouse();
    //qDebug() << "handleShowEvent" << rect() << QDateTime::currentMSecsSinceEpoch();
}

void QKxFilterListView::handleHideEvent(QHideEvent *ev)
{
    QWidget::hideEvent(ev);
    m_grab->releaseMouse();
    //qDebug() << "handleHideEvent" << rect() << QDateTime::currentMSecsSinceEpoch();
}

void QKxFilterListView::handleMouseMoveEvent(QMouseEvent *ev)
{
    QWidget::mouseMoveEvent(ev);
    //qDebug() << "handleMouseMoveEvent" << rect() << QDateTime::currentMSecsSinceEpoch();
}

void QKxFilterListView::handleMousePressEvent(QMouseEvent *ev)
{
    QWidget::mousePressEvent(ev);
    QPoint pt = ev->pos();
    QRect rt = m_grab->rect();
    //qDebug() << "handleMousePressEvent" << rt << pt << rt.contains(pt);
    m_grab->releaseMouse();
    if(!rt.contains(pt)) {
        hide();
    }
}
