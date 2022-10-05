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
private:
private:
    void closeEvent(QCloseEvent *event);
private slots:
    void onSetQualityLevel(int lv);
    void onBlackScreen();
    void onLockScreen();
    void onNetScreenRatioRequest();
    void onNextScreenModeRequest();

    void onSessionReconnect();
    void onForceToCloseThisSession();
    void onFinished();
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onErrorArrived(const QByteArray& buf);
    void onPasswordResult(const QByteArray& passwd, bool ok);
    void onPasswordInputResult(const QString& pass, bool isSave);
private:
    QVariantMap config(const QString& props);
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    bool eventFilter(QObject *w, QEvent *e);
private:
    const QString m_target;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QWoTermMask> m_mask;
    QPointer<QWoPasswordInput> m_passInput;
    bool m_savePassword;
    QString m_passwdLast;
    bool m_isPasswdOk;
};
