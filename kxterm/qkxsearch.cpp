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

#include "qkxsearch.h"
#include "ui_qkxsearch.h"

#include "qkxtermitem.h"

QKxSearch::QKxSearch(QKxTermItem *term, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QKxSearch),
    m_term(term)
{
    ui->setupUi(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QObject::connect(ui->findPrev, SIGNAL(clicked()), this, SLOT(onFindPrev()));
    QObject::connect(ui->findNext, SIGNAL(clicked()), this, SLOT(onFindNext()));
    QObject::connect(ui->findAll, SIGNAL(clicked()), this, SLOT(onFindAll()));
    QObject::connect(ui->close, SIGNAL(clicked()), this, SLOT(onClose()));
    QObject::connect(ui->key, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    QObject::connect(ui->key, SIGNAL(returnPressed()), this, SLOT(onFindNext()));
}

QKxSearch::~QKxSearch()
{
    delete ui;
}

void QKxSearch::onClose()
{
    m_term->clearSelection();
    setVisible(false);
}

void QKxSearch::onFindPrev()
{
    m_term->findPrev(ui->ascase->isChecked(), ui->regular->isChecked());
}

void QKxSearch::onFindNext()
{
    m_term->findNext(ui->ascase->isChecked(), ui->regular->isChecked());
}

void QKxSearch::onFindAll()
{
    m_term->findAll(ui->ascase->isChecked(), ui->regular->isChecked());
}

void QKxSearch::onTextChanged(const QString &txt)
{
    m_term->find(txt, ui->ascase->isChecked(), ui->regular->isChecked());
}
