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

#ifndef QWOPLAYBOOKCONSOLEPROXY_H
#define QWOPLAYBOOKCONSOLEPROXY_H

#include <QObject>
#include <QPointer>
#include <QJSValue>

class QWidget;
class QWoPlaybookConsole;

class QWoPlaybookConsoleProxy : public QObject
{
    Q_OBJECT
public:
    explicit QWoPlaybookConsoleProxy(const QString& name, QWidget *parent = nullptr);
    ~QWoPlaybookConsoleProxy();

    Q_INVOKABLE void log(const QJSValue& v1,
                         const QJSValue& v2=QJSValue(),
                         const QJSValue& v3=QJSValue(),
                         const QJSValue& v4=QJSValue(),
                         const QJSValue& v5=QJSValue(),
                         const QJSValue& v6=QJSValue(),
                         const QJSValue& v7=QJSValue(),
                         const QJSValue& v8=QJSValue(),
                         const QJSValue& v9=QJSValue());
    void open();
    void close();
    bool isVisible() const;
private:
    QPointer<QWidget> m_widgetParent;
    QString m_name;
    QPointer<QWoPlaybookConsole> m_console;
};

#endif // QWOPLAYBOOKCONSOLEPROXY_H
