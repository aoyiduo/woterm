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

QWoPasswordInput::QWoPasswordInput(QWidget *parent) :
    QWoWidget(parent),
    ui(new Ui::QWoPasswordInput)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground, true);
    ui->setupUi(this);

    QObject::connect(ui->visible, SIGNAL(clicked(bool)), this, SLOT(onPasswordVisible(bool)));
    QObject::connect(ui->btnFinish, SIGNAL(clicked()), this, SLOT(onClose()));
    QObject::connect(ui->password, SIGNAL(returnPressed()), this, SLOT(onClose()));

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    ui->password->setFocusPolicy(Qt::StrongFocus);
    ui->password->setFocus();
    QFont ft = ui->title->font();
    ft.setBold(true);
    ui->title->setFont(ft);
    ui->title->setAlignment(Qt::AlignHCenter);
    ui->inputArea->setContentsMargins(5, 5, 5, 5);
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

void QWoPasswordInput::paintEvent(QPaintEvent *paint)
{
    QPainter p(this);
    p.setBrush(QColor(128,128,128,128));
    p.drawRect(0,0, width(), height());
}

void QWoPasswordInput::mousePressEvent(QMouseEvent *ev)
{
    ev->accept();
}

void QWoPasswordInput::contextMenuEvent(QContextMenuEvent *ev)
{
    ev->accept();
}
