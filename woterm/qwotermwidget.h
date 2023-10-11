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

#include <qkxtermwidget.h>

#include <QPointer>

class QMenu;
class QWoTermMask;
class QWoPasswordInput;
class QWoTermWidgetImpl;
class QSplitter;
class QLabel;

class QWoTermWidget;
class QWoLoadingWidget;
class QKxBackgroundImageRender;
class QWoShowerWidget;

class QWoTermCreator
{
public:
    virtual QWoTermWidget *create(const QString& title, QWidget *parent) = 0;
protected:
};

template <class T>
class QWoTermCreatorT : public QWoTermCreator
{
public:
    virtual QWoTermWidget *create(const QString& title, QWidget *parent){
        return new T(title, parent);
    }
};

class QWoTermWidget : public QKxTermWidget
{
    Q_OBJECT
public:
    enum ETermType {
        ETTRemoteTarget,
        ETTLocalShell,
        ETTSerialPort,
        ETTConsole
    };

    enum ERightKeyMode {
        ERKM_NotDefined,
        ERKM_Copy,
        ERKM_CopyPaste
    };

public:
    explicit QWoTermWidget(const QString& target, int gid, ETermType ttype, QWidget *parent=nullptr);
    virtual ~QWoTermWidget();

    static QWoTermWidget *lastFocusTermWidget();
    static QList<QPointer<QWoTermWidget>> termsAll();

    void setImplementWidget(QWoShowerWidget* impl);
    QWoShowerWidget *implementWidget();

    void closeAndDelete();

    void splitWidget(const QString& target, int gid, bool vertical);
    bool attachWidget(QWoTermWidget *w, bool vertical);
    void detachWidget();

    ETermType termType() const;

    void triggerPropertyCheck();

    QString target() const;
    int gid() const;
    bool isLocalShell();

    void showLoading(bool on);
    void reloadProperty();

    bool hasHistoryFile() const;
    void outputHistoryToFile();
    void stopOutputHistoryFile();
signals:
    void aboutToClose(QCloseEvent* event);

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent* ev);
    // for close safely.
    virtual QList<QString> collectUnsafeCloseMessage() = 0;
protected:
    void initDefault();
    Q_INVOKABLE void initCustom();
protected slots:
    void onResetTermSize();
    void onCleanHistory();
    void onOutputHistoryToFile();
    void onStopOutputHistoryFile();
    void onFloatThisTab();
    void onTimeoutToSelectCopy();
private:
    void resetProperty(QVariantMap data, bool force=false);

    QWoTermWidgetImpl *findTermImpl();
    Q_INVOKABLE void addToTermImpl();
    void removeFromTermImpl();

    void onBroadcastMessage(int type, QVariant msg);
    bool handleWoEvent(QEvent *ev);
protected:
    virtual bool eventFilter(QObject *obj, QEvent *ev);
protected:
    friend class QWoTermWidgetImpl;
    QPointer<QWoShowerWidget> m_implementWidget;
    QPointer<QWoLoadingWidget> m_loading;
    QString m_target;
    int m_gid;
    bool m_bexit;
    ERightKeyMode m_rkeyMode;
    bool m_selectCopy;
    ETermType m_ttype;

    static QList<QPointer<QWoTermWidget>> m_gsTermsAll;
    static QPointer<QWoTermWidget> m_gsTermWidget;
    static QPointer<QKxBackgroundImageRender> m_bkImageRender;
};
