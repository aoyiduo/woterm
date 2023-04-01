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

#include "qmomenu.h"
#include "ui_qmomenu.h"

#include "qmomainwindow.h"
#include "qmomenulistmodel.h"

#include <QShowEvent>
#include <QIcon>

QMoMenu::QMoMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QMoMenu)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground);
    ui->menuArea->setObjectName("menuArea");
    QSize sz = QMoMainWindow::instance()->size();
    ui->menuArea->setMinimumWidth(sz.width() / 2);
    m_model = new QMoMenuListModel(this);
    ui->menuList->setModel(m_model);
    m_model->update(MID_Back, tr("Back"), QIcon(":/woterm/resource/skin/back.png"));
    m_model->update(MID_Cancel, tr("Cancel"), QIcon(":/woterm/resource/skin/close.png"));
    QObject::connect(ui->menuList, SIGNAL(clicked(QModelIndex)), this, SLOT(onCurrentItemClicked(QModelIndex)));
}

QMoMenu::~QMoMenu()
{
    delete ui;
}

void QMoMenu::addItem(int mid, const QString &text, const QString &icon)
{
    Q_ASSERT(mid >= MID_User);
    m_model->update(mid, text, QIcon(icon));
}

void QMoMenu::removeItem(int mid)
{
    m_model->remove(mid);
}

void QMoMenu::onCurrentItemClicked(const QModelIndex &idx)
{
    QVariant vid = idx.data(ROLE_MENUID);
    if(!vid.isValid()) {
        return;
    }
    int id = vid.toInt();
    close();
    emit clicked(id);
}

void QMoMenu::showEvent(QShowEvent *e)
{
    int h = m_model->itemsHeight();
    ui->menuList->setFixedHeight(h);
    ui->menuArea->setMaximumHeight(h + 20);
    adjustSize();
}
