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

#ifndef QMOMAINWINDOW_H
#define QMOMAINWINDOW_H

#include <QWidget>
#include <QPointer>
#include <QMessageBox>

namespace Ui {
class QMoMainWindow;
}

class QMoUtils;
class QQmlContext;
class QMoShowWidget;
class QMoMessageBoxAssist;
class QMoRecentHistoryModel;

class QMoMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QMoMainWindow(QWidget *parent = nullptr);
    ~QMoMainWindow();

    static QMoMainWindow * instance();
    QSize size() const;

    Q_INVOKABLE void openLocalShell();
    Q_INVOKABLE bool openSsh(const QString& target);
    Q_INVOKABLE bool openSftp(const QString& target);
    Q_INVOKABLE bool openTelnet(const QString& target);
    Q_INVOKABLE bool openRLogin(const QString& target);
    Q_INVOKABLE bool openMstsc(const QString& target);
    Q_INVOKABLE bool openVnc(const QString& target);
    Q_INVOKABLE bool openRecent(const QString& target, int type);
    Q_INVOKABLE bool openSerialPort();
signals:
    void readyUpgradeVersion();
private slots:
    void onWindowCloseArrived();
    void onInitLater();
    void onAppStart();
    void onVersionCheck(int code, const QByteArray &body);
private:
    virtual void closeEvent(QCloseEvent* ev);
    virtual void resizeEvent(QResizeEvent* ev);
private:
    Ui::QMoMainWindow *ui;
    QPointer<QQmlContext> m_qmlContext;
    QPointer<QMoRecentHistoryModel> m_recentAccess;
    QPointer<QMoShowWidget> m_show;
    bool m_closeHasConfirm;
};

#endif // QMOMAINWINDOW_H
