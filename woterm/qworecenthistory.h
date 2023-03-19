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

#ifndef QWORECENTHISTORY_H
#define QWORECENTHISTORY_H

#include <QObject>

class QMenu;
class QWoRecentHistory : public QObject
{
    Q_OBJECT
public:
    explicit QWoRecentHistory(QObject *parent = nullptr);
    void update(const QString& name, int type);
    void buildMenu(QMenu *menu);

signals:
    void readyToConnect(const QString&, int);
private slots:
    void onClearAll();
    void onMenuTriggered();
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

#endif // QWORECENTHISTORY_H
