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

#include "qkxpositionitem.h"

#include <QResizeEvent>
#include <QtGlobal>
#include <QDebug>
#include <QPainter>
#include <QIcon>
#include <QMenu>
#include <QProcess>
#include <QVBoxLayout>
#include <QTimer>

QKxPositionItem::QKxPositionItem(QWidget *parent)
    : QWidget(parent)
{
    m_dots.append(0);
}

QKxPositionItem::~QKxPositionItem()
{
}

void QKxPositionItem::setTileMode(int mode)
{
    if(m_dots.isEmpty()) {
        return;
    }
    int idx = m_dots.at(0);
    int y = idx / 3;
    int x = idx % 3;
    m_dots.clear();
    if(mode == 1) {
        m_dots.append(y * 3);
        m_dots.append(y * 3 + 1);
        m_dots.append(y * 3 + 2);
    }else if(mode == 2) {
        m_dots.append(x);
        m_dots.append(3 + x);
        m_dots.append(6 + x);
    }else if(mode == 3) {
        for(int i = 0; i < 9; i++) {
            m_dots.append(i);
        }
    }else{
        m_dots.append(idx);
    }
    update();
}

void QKxPositionItem::setPositionAsString(const QString &pos)
{
    m_dots.clear();
    for(int i = 0; i < pos.length(); i++) {
        QChar c = pos.at(i);
        if(c == '1') {
            m_dots.append(i);
        }
    }
    update();
}

QString QKxPositionItem::postionAsString() const
{
    QString tmp("000000000");
    for(int i = 0; i < m_dots.length(); i++) {
        int v = m_dots.at(i);
        if(v < 0 || v >= tmp.length()) {
            continue;
        }
        tmp[v] = '1';
    }
    return tmp;
}

void QKxPositionItem::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.save();
    QRect rt(0, 0, width(), height());
    int bw = width() / 3;
    int bh = height() / 3;
    QRect brt = QRect(0, 0, bw, bh);
    for(int y = 0; y < 3; y++) {
        for(int x = 0; x < 3; x++) {
            QRect rt = brt.translated(bw * x, bh * y);
            p.drawRect(rt);
            int idx = y * 3 + x;
            for(int i = 0; i < m_dots.length(); i++) {
                if(idx == m_dots.at(i)) {
                    rt.adjust(bw / 3, bw / 3, -bw / 3, -bw / 3);
                    p.drawEllipse(rt);
                }
            }
        }
    }
    p.restore();
}

void QKxPositionItem::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    QPoint pt = event->pos();
    updateIndex(pt);
}

void QKxPositionItem::updateIndex(const QPoint &pt)
{
    int bw = width() / 3;
    int bh = height() / 3;
    QRect brt = QRect(0, 0, bw, bh);
    for(int y = 0; y < 3; y++) {
        for(int x = 0; x < 3; x++) {
            QRect rt = brt.translated(bw * x, bh * y);
            if(rt.contains(pt)) {
                m_dots.clear();
                int idx = y * 3 + x;
                m_dots.append(idx);
                update();
                emit clicked(idx);
                return;
            }
        }
    }
}
