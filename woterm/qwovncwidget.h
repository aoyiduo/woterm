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

#include "qkxvncwidget.h"
#include <QPointer>


class QWoLoadingWidget;
class QWoTermMask;
class QWoVnc;
class QWoPasswordInput;

class QWoVncWidget : public QKxVncWidget
{
    Q_OBJECT
public:
    explicit QWoVncWidget(const QString& target, QWidget *parent=nullptr);
    ~QWoVncWidget();

    Q_INVOKABLE void reconnect();
signals:
    void aboutToClose(QCloseEvent* event);
    void forceToClose();
private:
    void closeEvent(QCloseEvent *event);
private slots:
    void onSetQualityLevel(int lv);
    void onBlackScreen();
    void onLockScreen();
    void onNetScreenRatioRequest();
    void onNextScreenModeRequest();
    void onForceToCloseSession();

    void onSessionReconnect();
    void onFinished();
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onErrorArrived(const QByteArray& buf);
    void onPasswordResult(const QByteArray& passwd, bool ok);
    void onPasswordInputResult(const QString& pass, bool isSave);
    void onAdjustPosition();
private:
    QVariantMap config(const QVariantMap& props);
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    bool eventFilter(QObject *w, QEvent *e);
private:
    const QString m_target;
    QPointer<QWidget> m_parent;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QWoTermMask> m_mask;
    QPointer<QWoPasswordInput> m_passInput;
    bool m_savePassword;
    QString m_passwdLast;
    bool m_isPasswdOk;
};
