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

#include "qwopasswordinput.h"
#include "ui_qwopasswordinput.h"

#include "qkxmessagebox.h"

#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

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
    adjustSize();
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
        QMessageBox::StandardButton btn = QKxMessageBox::warning(this, tr("Tip"), tr("The Password is Empty, continue to finish?"), QMessageBox::Ok|QMessageBox::No);
        if(btn == QMessageBox::No) {
            return ;
        }
    }
    hide();
    emit result(pass, ui->save->isChecked());
}

void QWoPasswordInput::showEvent(QShowEvent *e)
{
    QWoWidget::showEvent(e);
    if(m_timer == nullptr) {
        m_timer = new QTimer(this);
        QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(raise()));
    }
    m_timer->start(100);
}

void QWoPasswordInput::hideEvent(QHideEvent *event)
{
    QWoWidget::hideEvent(event);
    m_timer->stop();
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
