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

#include "qwowidget.h"

#include <QPointer>

class QMenu;

class QWoShowerWidget : public QWoWidget
{
    Q_OBJECT
public:
    enum EShowerType {
        eTerm,
        eMstsc,
        eRdp,
        eSftp,
        eSerialport, //
        eVnc,
        ePlaybook
    };

    enum ESessionState {
        eUnknow,
        eDisconnected,
        eOtherDisconnected, // current focus is good.
        eAllConnected
    };

    enum EHistoryFileState {
        eNever,
        eNoFile,
        eOtherNoFile, // current has history but other no history.
        eAllHasFiles
    };

public:
    explicit QWoShowerWidget(const QString& target, EShowerType type, QWidget *parent=nullptr);
    inline QString targetName() const {
        return m_target;
    }

    inline EShowerType showerType() {
        return m_typeShower;
    }

    QWidget *lastFocusWidget();
    void setLastFocusWidget(QWidget *w);

    virtual ESessionState sessionState(){
        return eUnknow;
    };
    virtual void stopSession(){

    };
    virtual void reconnectSession(bool all) {

    }

    virtual EHistoryFileState historyFileState() {
        return eNever;
    }

    virtual void outputHistoryToFile() {

    }

    virtual void stopOutputHistoryToFile(bool all) {

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

protected slots:
    void onShowFullScreen();
protected:
    friend class QWoShower;
    int m_id;
    QString m_target;
    EShowerType m_typeShower;
    QPointer<QWidget> m_lastFocusWidget;
};
