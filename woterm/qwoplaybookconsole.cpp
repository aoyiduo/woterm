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

#include "qwoplaybookconsole.h"
#include "ui_qwoplaybookconsole.h"

#include "qkxmessagebox.h"

#include <QTimer>
#include <QTextBlock>
#include <QTextDocument>
#include <QDebug>
#include <QIntValidator>

#define MAX_BLOCK       (10000)
#define MIN_BLOCK       (5)

QWoPlaybookConsole::QWoPlaybookConsole(const QString& name, QWidget *parent)
    : QWidget(parent, Qt::Tool)
    , ui(new Ui::QWoPlaybookConsole)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("Console")+QString(":%1").arg(name));
    ui->textEdit->setReadOnly(true);
    QObject::connect(ui->btnClear, SIGNAL(clicked()), ui->textEdit, SLOT(clear()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

    setMaximumBlockCount(1000);
    ui->maxBlock->setText(QString::number(1000));
    QObject::connect(ui->maxBlock, SIGNAL(editingFinished()), this, SLOT(onMaxBlockArrived()));
}

QWoPlaybookConsole::~QWoPlaybookConsole()
{
    delete ui;
}

void QWoPlaybookConsole::append(const QString &msg)
{
    QTextCursor tc = ui->textEdit->textCursor();
    tc.movePosition(QTextCursor::End);
    tc.insertBlock();
    tc.insertText(msg);
}

int QWoPlaybookConsole::maximumBlockCount() const
{
    QTextDocument *doc = ui->textEdit->document();
    return doc->maximumBlockCount();
}

void QWoPlaybookConsole::setMaximumBlockCount(int cnt)
{
    QTextDocument *doc = ui->textEdit->document();
    doc->setMaximumBlockCount(cnt);
}

void QWoPlaybookConsole::onMaxBlockArrived()
{
    QString cnt = ui->maxBlock->text();
    int icnt = cnt.toInt();
    if(icnt < MIN_BLOCK || icnt > MAX_BLOCK) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("The number value should be range between %1 and %2 .").arg(MIN_BLOCK).arg(MAX_BLOCK));
        return;
    }
    setMaximumBlockCount(icnt);
}

void QWoPlaybookConsole::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);
}

void QWoPlaybookConsole::hideEvent(QHideEvent *ev)
{
    QWidget::hideEvent(ev);
}
