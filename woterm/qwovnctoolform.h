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

#ifndef QWOVNCTOOLFORM_H
#define QWOVNCTOOLFORM_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class QWoVncToolForm;
}

class QMenu;
class QWoVncWidget;
class QWoShowerWidget;

class QWoVncToolForm : public QWidget
{
    Q_OBJECT

public:
    explicit QWoVncToolForm(QWoVncWidget *vnc, QWidget *parent = 0);
    ~QWoVncToolForm();

    QSize sizeHint() const;
signals:
    void enter();
    void leave();
    void nextScreenRatio();
    void nextScreenMode();
    void blackScreen();
    void lockScreen();
    void qualityLevel(int lv);
    void sizeChanged(const QSize& sz);
    void ftpClicked();
private slots:    
    void onQualityMenuAboutToShow();
    void onQualityLevelSelect(QAction *chk);
    void onQualityMenuActived();
    void onExitFullScreen();
    void onExitRemoteDesktop();
    void onReturnTabWindow();
    void onFullScreen();
    void onMinimizeWindow();
    void onScreenCountChanged(int cnt);
    void onMessageSupport(int cnt);
    void onCloseWindow();
    void onAudioClicked();

    void onAudioStateArrived(int state);
    void onShowerFloatChanged(QWoShowerWidget *shower, bool bFloat);
private:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    bool eventFilter(QObject *w, QEvent *e);
    void resizeEvent(QResizeEvent *e);
private:
    Q_INVOKABLE void updateStatus();
    Q_INVOKABLE void resetLayout();
private:
    Ui::QWoVncToolForm *ui;
    QPointer<QWoVncWidget> m_vnc;
    QPointer<QMenu> m_qlvMenu;
    QPointer<QAction> m_qlv0;
    QPointer<QAction> m_qlv1;
    QPointer<QAction> m_qlv2;
    QPointer<QAction> m_qlv3;
    QPointer<QAction> m_qlv4;
    QPointer<QAction> m_qlv5;
    QPointer<QAction> m_qlv7;
    QPointer<QAction> m_qlv8;
    QPointer<QAction> m_qlv9;
    QPointer<QAction> m_qlv10;
    QPointer<QAction> m_qlv11;
};

#endif // QWOVNCTOOLFORM_H
