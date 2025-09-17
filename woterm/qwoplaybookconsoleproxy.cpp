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

#include "qwoplaybookconsoleproxy.h"
#include "qwoplaybookconsole.h"

QWoPlaybookConsoleProxy::QWoPlaybookConsoleProxy(const QString& name, QWidget *parent)
    : QObject(parent)
    , m_widgetParent(parent)
    , m_name(name)
{

}

QWoPlaybookConsoleProxy::~QWoPlaybookConsoleProxy()
{
    if(m_console) {
        m_console->close();
        m_console->deleteLater();
    }
}

void QWoPlaybookConsoleProxy::log(const QJSValue &v1, const QJSValue &v2, const QJSValue &v3, const QJSValue &v4, const QJSValue &v5, const QJSValue &v6, const QJSValue &v7, const QJSValue &v8, const QJSValue &v9)
{
    if(m_console == nullptr) {
        return;
    }
    QJSValueList all = {v1, v2, v3, v4, v5, v6, v7, v8, v9};
    QStringList oks;
    for(auto it = all.begin(); it != all.end(); it++) {
        QJSValue v = *it;
        if(v.isUndefined()) {
            continue;
        }
        oks.append(v.toString());
    }
    QString msg = oks.join(' ');
    m_console->append(msg);
}

void QWoPlaybookConsoleProxy::open()
{
    if(m_console == nullptr) {
        m_console = new QWoPlaybookConsole(m_name, m_widgetParent);
        m_console->resize(800, 600);
    }
    m_console->show();
}

void QWoPlaybookConsoleProxy::close()
{
    if(m_console) {
        m_console->close();
    }
}

bool QWoPlaybookConsoleProxy::isVisible() const
{
    if(m_console == nullptr) {
        return false;
    }
    return m_console->isVisible();
}
