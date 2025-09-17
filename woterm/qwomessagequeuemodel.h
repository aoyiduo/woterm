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

#ifndef QWOMESSAGEQUEUEMODEL_H
#define QWOMESSAGEQUEUEMODEL_H

#include <QAbstractListModel>
#include <QList>

class QWoMessageQueueModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoMessageQueueModel(QObject *parent = nullptr);

    void warning(const QString& title, const QString& content);
signals:

private:
protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    enum EMessageType {
        EMTWarning = 0,
        EMTInformation = 1
    };
    struct MessageData {
        EMessageType type;
        QString title;
        QString content;
    };

    QList<MessageData> m_queue;
};

#endif // QWOMESSAGEQUEUEMODEL_H
