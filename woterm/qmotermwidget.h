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

#ifndef QMOTERMWIDGET_H
#define QMOTERMWIDGET_H

#include <qkxtermwidget.h>
#include <QPointer>

class QWoLoadingWidget;
class QKxTouchPoint;
class QMoTermWidget : public QKxTermWidget
{
    Q_OBJECT
public:
    enum ETermType {
        ETTRemoteTarget,
        ETTLocalShell,
        ETTSerialPort
    };
    enum ERightKeyMode {
        ERKM_NotDefined,
        ERKM_Copy,
        ERKM_CopyPaste
    };
public:
    explicit QMoTermWidget(const QString& target, ETermType ttype, QWidget *parent=nullptr);
    virtual ~QMoTermWidget();
    void showLoading(bool on);
    void reloadProperty();
signals:
    void aboutToClose(QCloseEvent* event);
    void touchPointClicked();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
protected:
    QString target() const;
    void initDefault();
    void initCustom();
protected slots:
    void onResetTermSize();
    void onCleanHistory();
    void onOutputHistoryToFile();
    void onStopOutputHistoryFile();
private:
    void resetProperty(QVariantMap data);
    Q_INVOKABLE void showTouchPoint(bool show, bool async);
    Q_INVOKABLE void resetTouchPointPosition();
protected:
    QString m_target;
    ETermType m_ttype;
    QPointer<QWoLoadingWidget> m_loading;
    ERightKeyMode m_rkeyMode;
    QString m_historyFile;
    QPointer<QKxTouchPoint> m_touchPoint;
};

#endif // QMOTERMWIDGET_H
