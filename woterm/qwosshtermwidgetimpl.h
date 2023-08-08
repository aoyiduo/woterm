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

#include "qwotermwidgetimpl.h"

#include <QPointer>

class QSplitter;
class QWoSshTermWidget;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QWoSftpWidget;

class QWoSshTermWidgetImpl : public QWoTermWidgetImpl
{
    Q_OBJECT
public:
    explicit QWoSshTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoSshTermWidgetImpl();
protected:
    virtual QWoTermWidget *createTermWidget(const QString& target, int gid, QWidget *parent);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual ESessionState sessionState();
    virtual void stopSession();
    virtual void reconnectSession(bool all);
    virtual EHistoryFileState historyFileState();
    virtual void outputHistoryToFile();
    virtual void stopOutputHistoryToFile(bool all);
private slots:
    void onSftpAssistOpen();
    void onNewSftpSession();
    void onNewSshSession();
    void onActivePathArrived(const QString& path);
private:
    QPointer<QWoSftpWidget> m_sftp;
};
