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

#ifndef QMOMENULISTMODEL_H
#define QMOMENULISTMODEL_H

#include "qwoglobal.h"
#include <QPointer>
#include <QAbstractListModel>
#include <QIcon>
#include <QList>

class QTimer;
class QMoMenuListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QMoMenuListModel(QObject *parent = nullptr);

    void append(int id, const QString& name, const QIcon& ico = QIcon());
    int itemCount() const;
    int itemsHeight() const;
protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private slots:
    void onLaterReset();
private:
    struct MenuData {
        int id;
        QIcon icon;
        QString name;
    };

    QList<MenuData> m_items;
    QPointer<QTimer> m_timer;
};

#endif // QMOMENULISTMODEL_H
