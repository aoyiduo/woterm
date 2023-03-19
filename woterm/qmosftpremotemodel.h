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

#ifndef QMOSFTPREMOTEMODEL_H
#define QMOSFTPREMOTEMODEL_H

#include "qwoglobal.h"

#include <QAbstractListModel>
#include <QList>
#include <QIcon>

class QMoSftpRemoteModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QMoSftpRemoteModel(QObject *parent = nullptr);
    virtual ~QMoSftpRemoteModel();
    Q_INVOKABLE QString fileName(int row) const;
    Q_INVOKABLE QVariantMap fileInfo(int row) const;
    Q_INVOKABLE QVariantList fileInfosFromSelection() const;
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
public:
    QString path() const;
    bool exist(const QString& fileName);
signals:
    void pathChanged(const QString& path);
private slots:
    void onDirOpen(const QString& path, const QVariantList& data, const QVariantMap& userData);
protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
private:
    QVariantMap toMap(const FileInfo& fi) const;
private:
    Q_DISABLE_COPY(QMoSftpRemoteModel)
    QList<FileInfo> m_fileInfos;
    QString m_path;
};

#endif // QMOSFTPREMOTEMODEL_H
