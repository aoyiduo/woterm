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

#ifndef QTERMITEM_H
#define QTERMITEM_H

#include "qvtedef.h"
#include <QWidget>
#include <QPointer>

class QKxTermItem;
class QAbstractSlider;
class QKxSearch;

class QTERM_EXPORT QKxTermWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QKxTermWidget(QWidget *parent = nullptr);
    ~QKxTermWidget();
    QKxTermItem *termItem();
    QWidget *findBar();
    void setFindBarVisible(bool on);

    QString termName() const;
    void setTermName(const QString& name);

    void sendInput(const QByteArray& cmd);

    void setFindShortCut(QKeySequence key);
signals:
    void sendData(const QByteArray& buf);
    void activePathArrived(const QString& path);
protected:
    void resizeEvent(QResizeEvent *ev);
    bool eventFilter(QObject *watched, QEvent *event);
private slots:
    void onTermScrollValueChanged(int lines, int position);
    void onScrollValueChanged(int position);

protected:
    QPointer<QKxTermItem> m_term;
    QPointer<QKxSearch> m_find;
    QPointer<QAbstractSlider> m_vscroll;
    QKeySequence m_keyFind;
};

#endif // QTERM_H
