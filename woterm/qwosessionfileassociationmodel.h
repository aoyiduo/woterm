/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWOSESSIONFILEASSOCIATIONMODEL_H
#define QWOSESSIONFILEASSOCIATIONMODEL_H

#include "qwoglobal.h"

#include <QPointer>
#include <QAbstractListModel>
#include <QList>

struct FileAssociation {
    QString type;
    QString application;
    QString parameter;
    int priority;

    FileAssociation() {
        priority = 0;
    }
};
Q_DECLARE_METATYPE(FileAssociation)

class QWoSessionFileAssociationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoSessionFileAssociationModel(QObject *parent = nullptr);

    static QWoSessionFileAssociationModel *instance();

    bool autoAddDefaultEditor() const;
    void setAutoAddDefaultEditor(bool on);

    bool findEditor(const QString& file, FileAssociation *pFileAssoc);
    Q_INVOKABLE void add(const QString &typs, const QString& app, const QString& parameter, int proirity);
    Q_INVOKABLE void update(int idx, const QString &typs, const QString& app, const QString& parameter, int proirity);
    Q_INVOKABLE void remove(int idx);
    Q_INVOKABLE void clear();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
private:
    void save();
    Q_INVOKABLE void init();
    void sort();
    bool tryToAddDefaultEditor();
private:
    QList<FileAssociation> m_all;
};

#endif // QWOSESSIONFILEASSOCIATIONMODEL_H
