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

