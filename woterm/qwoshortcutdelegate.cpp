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

#include "qwoshortcutdelegate.h"

#include <QKeySequenceEdit>

//! [0]
QWoKeySequenceDelegate::QWoKeySequenceDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}


QWidget *QWoKeySequenceDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QKeySequenceEdit *editor = new QKeySequenceEdit(parent);
    return editor;
}


void QWoKeySequenceDelegate::setEditorData(QWidget *w, const QModelIndex &index) const
{
    QKeySequence value = index.model()->data(index, Qt::EditRole).value<QKeySequence>();
    QKeySequenceEdit *editor = static_cast<QKeySequenceEdit*>(w);
    editor->setKeySequence(value);
}


void QWoKeySequenceDelegate::setModelData(QWidget *w, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QKeySequenceEdit *editor = static_cast<QKeySequenceEdit*>(w);
    QKeySequence ks = editor->keySequence();
    model->setData(index, ks, Qt::EditRole);
}


void QWoKeySequenceDelegate::updateEditorGeometry(QWidget *w, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    w->setFocus();
    w->setGeometry(option.rect);
}

