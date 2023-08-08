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

#ifndef QWOSERIALTERMWIDGET_H
#define QWOSERIALTERMWIDGET_H

#include "qwoshowerwidget.h"
#include "qwotermwidget.h"
#include <QPointer>

class QSplitter;
class QWoSerialInput;
class QMenu;
class QTabBar;
class QWoCommandLineInput;
class QSerialPort;
class QWoModem;

class QWoSerialTermWidget : public QWoTermWidget
{
    Q_OBJECT
public:
    explicit QWoSerialTermWidget(const QString& target, QWidget* parent=nullptr);
    ~QWoSerialTermWidget();
protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual QList<QString> collectUnsafeCloseMessage();
private slots:
    void onCopyToClipboard();
    void onModifyThisSession();
    void onCleanThisSession();
    void onOpenInteractiveMode();
    void onCloseInteractiveMode();
    void onShowFindBar();

    void onZmodemSend(bool local=true);
    void onZmodemRecv(bool local=true);
    void onZmodemAbort();
    void onZmodemDataArrived(const QByteArray& buf);
    void onZmodemStatusArrived(const QByteArray& buf);
    void onZmodemFinished();
private:
    int isZmodemCommand(const QByteArray &data);
    bool checkZmodemInstall();
private:
    bool eventFilter(QObject *watched, QEvent *event);
private:
    QPointer<QMenu> m_menu;    
    QPointer<QWoModem> m_modem;
};

#endif
