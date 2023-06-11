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

#include "qwounioninputitem.h"
#include "ui_qwounioninputitem.h"
#include "qkxbuttonassist.h"

#include "qwounioninputconfiguredialog.h"
#include "qwosetting.h"

QWoUnionInputItem::QWoUnionInputItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoUnionInputItem)

{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);
    QObject::connect(ui->btnInput, SIGNAL(clicked()), this, SLOT(onEditFinished()));
    QObject::connect(ui->input, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
    QObject::connect(ui->btnSetting, SIGNAL(clicked()), this, SLOT(onSettingsButtonClicked()));
    QKxButtonAssist *btn = new QKxButtonAssist("../private/skins/black/remove.png", ui->input);
    QObject::connect(btn, SIGNAL(clicked(int)), ui->input, SLOT(clear()));
    adjustSize();
    m_isEnterAsRN = QWoSetting::value("UnionInput/EnterAsRN", false).toBool();
}

QWoUnionInputItem::~QWoUnionInputItem()
{
    delete ui;
}

void QWoUnionInputItem::onEditFinished()
{
    QString cmd = ui->input->text();
    cmd += m_isEnterAsRN ? "\r\n" : "\n";
    ui->input->clear();
    emit inputArrived(cmd);
}

void QWoUnionInputItem::onSettingsButtonClicked()
{
    QWoUnionInputConfigureDialog dlg(m_isEnterAsRN, this);
    if(dlg.exec() == QDialog::Accepted) {
        m_isEnterAsRN = dlg.isEnterRN();
        QWoSetting::setValue("UnionInput/EnterAsRN", m_isEnterAsRN);
    }
}
