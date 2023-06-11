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

#pragma once

#include "qwowidget.h"

#include <QPointer>

class QMenu;

class QWoShowerWidget : public QWoWidget
{
    Q_OBJECT
public:
    explicit QWoShowerWidget(const QString& target, QWidget *parent=nullptr);
    inline QString targetName() const {
        return m_target;
    }
protected:
    inline int increaseId() {
        return m_id++;
    }
protected:
    virtual bool handleTabMouseEvent(QMouseEvent *ev) = 0;
    virtual void handleTabContextMenu(QMenu *menu) = 0;
    virtual QMap<QString, QString> collectUnsafeCloseMessage() = 0;    
    virtual bool handleCustomProperties();
    virtual bool isRemoteSession();
protected:
    friend class QWoShower;
    int m_id;
    QString m_target;
};
