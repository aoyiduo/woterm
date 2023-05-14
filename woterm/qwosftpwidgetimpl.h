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

#include "qwoshowerwidget.h"

#include <QPointer>

class QSplitter;
class QWoSftpWidget;
class QMenu;
class QTabBar;
class QWoCommandLineInput;

class QWoSftpWidgetImpl : public QWoShowerWidget
{
    Q_OBJECT
public:
    explicit QWoSftpWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoSftpWidgetImpl();
signals:
    void aboutToClose(QCloseEvent* event);
protected:
    virtual QMap<QString, QString> collectUnsafeCloseMessage();
    virtual void closeEvent(QCloseEvent *event);
    virtual bool handleTabMouseEvent(QMouseEvent* ev);
    virtual void handleTabContextMenu(QMenu *menu);
    virtual bool handleCustomProperties();
    virtual void updateEnable(bool on);
private slots:
    void onDestroyReady();
    void onNewSshSession();
    void onNewSftpSession();
private:
    void setTabText(const QString& title);
private:
    friend class QWoSftpWidget;
    const int m_gid;
    const QPointer<QTabBar> m_tab;
    QPointer<QSplitter> m_root;
    QPointer<QWoSftpWidget> m_sftp;
    QPointer<QMenu> m_menu;
};
