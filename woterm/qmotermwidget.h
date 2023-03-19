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

#ifndef QMOTERMWIDGET_H
#define QMOTERMWIDGET_H

#include <qkxtermwidget.h>
#include <QPointer>

class QWoLoadingWidget;
class QMoTermWidget : public QKxTermWidget
{
    Q_OBJECT
public:
    enum ETermType {
        ETTRemoteTarget,
        ETTLocalShell,
        ETTSerialPort
    };
public:
    explicit QMoTermWidget(const QString& target, ETermType ttype, QWidget *parent=nullptr);
    virtual ~QMoTermWidget();
    void showLoading(bool on);
    void reloadProperty();
signals:
    void aboutToClose(QCloseEvent* event);

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
    void onTouchPointClicked();
private:
    void resetProperty(QVariantMap data);
protected:
    QString m_target;
    ETermType m_ttype;
    QPointer<QWoLoadingWidget> m_loading;
    bool m_rkeyPaste;
    QString m_historyFile;

};

#endif // QMOTERMWIDGET_H
