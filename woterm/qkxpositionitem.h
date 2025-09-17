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

#ifndef QKXPOSITIONITEM_H
#define QKXPOSITIONITEM_H

#include <QWidget>

class QKxPositionItem : public QWidget
{
    Q_OBJECT

public:
    explicit QKxPositionItem(QWidget *parent = nullptr);
    ~QKxPositionItem();

    // 0: no tile.
    // 1: tile horization.
    // 2: tile vertical.
    // 3: tile all
    void setTileMode(int mode);
    void setPositionAsString(const QString& pos);
    QString postionAsString() const;
signals:
    void clicked(int idx);
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    void updateIndex(const QPoint& pt);
private:
    QList<int> m_dots;
};

#endif // QKXPOSITIONITEM_H
