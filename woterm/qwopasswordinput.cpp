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

#include "qwopasswordinput.h"
#include "ui_qwopasswordinput.h"

#include <QPainter>
#include <QMessageBox>
#include <QMouseEvent>

QWoPasswordInput::QWoPasswordInput(QWidget *parent)
    : QWoWidget(parent)
    , ui(new Ui::QWoPasswordInput)
    , m_dragPosition(0, 0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QObject::connect(ui->visible, SIGNAL(clicked(bool)), this, SLOT(onPasswordVisible(bool)));
    QObject::connect(ui->btnFinish, SIGNAL(clicked()), this, SLOT(onClose()));
    QObject::connect(ui->password, SIGNAL(returnPressed()), this, SLOT(onClose()));

    ui->password->setFocusPolicy(Qt::StrongFocus);
    ui->password->setFocus();
    QFont ft = ui->title->font();
    ft.setBold(true);
    ui->title->setFont(ft);
    ui->title->setAlignment(Qt::AlignHCenter);
}

QWoPasswordInput::~QWoPasswordInput()
{
    delete ui;
}

void QWoPasswordInput::reset(const QString &title, const QString &prompt, bool echo)
{
    ui->title->setText(title);
    ui->tip->setText(prompt);
    ui->save->setChecked(false);
    ui->save->setVisible(!echo);
    ui->visible->setVisible(!echo);
    ui->visible->setChecked(echo);
    ui->password->clear();
    ui->password->setEchoMode(echo ? QLineEdit::Normal : QLineEdit::Password);
}

void QWoPasswordInput::onPasswordVisible(bool checked)
{
    ui->password->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void QWoPasswordInput::onClose()
{
    QString pass = ui->password->text();
    if(pass.isEmpty()) {
        QMessageBox::StandardButton btn = QMessageBox::warning(this, tr("Tip"), tr("The Password is Empty, continue to finish?"), QMessageBox::Ok|QMessageBox::No);
        if(btn == QMessageBox::No) {
            return ;
        }
    }
    hide();
    emit result(pass, ui->save->isChecked());
}

void QWoPasswordInput::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void QWoPasswordInput::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
