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

#pragma once

#include "qwoglobal.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QFont>
#include <QKeySequence>

class QTreeView;

class QWoShortCutModel : public QAbstractItemModel
{
    Q_OBJECT
public:

public:
    explicit QWoShortCutModel(QTreeView *tree, QObject *parent = nullptr);
    virtual ~QWoShortCutModel();

    void setMaxColumnCount(int cnt);
    int widthColumn(const QFont& ft, int i);

    void append(int kid, QString desc, QKeySequence key);
    void append(QString title);
    QKeySequence value(int kid);
    QMap<int, QKeySequence> toMap();
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::DropActions supportedDropActions() const override;
private:
    void clear();
private:
    Q_DISABLE_COPY(QWoShortCutModel)
    struct TreeItem{
        bool isTitle;
        int kid;
        QString desc;
        QKeySequence key;
        TreeItem *parent;
        QList<TreeItem*> childs;
    };
    TreeItem *m_root;
    int m_maxColumn;
    QPointer<QTreeView> m_tree;
};
