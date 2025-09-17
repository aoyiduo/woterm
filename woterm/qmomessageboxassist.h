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

#ifndef QMOMESSAGEBOXASSIST_H
#define QMOMESSAGEBOXASSIST_H

#include <QMessageBox>
#include <QPointer>

class QMoMessageBoxAssist : public QObject
{
    Q_OBJECT
public:
    explicit QMoMessageBoxAssist(QWidget *parent = nullptr);
    Q_INVOKABLE int critical(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int information(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int question(const QString &title, const QString &text, int buttons = int(QMessageBox::Yes | QMessageBox::No), int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int warning(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);
private:
    QPointer<QWidget> m_dlgParent;
};

#endif // QMOMESSAGEBOXASSIT_H
