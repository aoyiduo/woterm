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

#ifndef QMORECENTHISTORYMODEL_H
#define QMORECENTHISTORYMODEL_H

#include <QAbstractListModel>

class QMoRecentHistoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ length NOTIFY lengthChanged)

public:
    explicit QMoRecentHistoryModel(QObject *parent = nullptr);
    Q_INVOKABLE void update(const QString& name, int type);
    Q_INVOKABLE void remove(const QString& name);
    Q_INVOKABLE void clear();
    int length() const;
signals:
    void lengthChanged();
protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
private:
    void save();
    Q_INVOKABLE void init();
private:
    struct RecentHistoryData {
        QString name;
        int timeLast;
        int type;
    };
    QList<RecentHistoryData> m_recents;
};



#endif // QMORECENTHISTORYMODEL_H
