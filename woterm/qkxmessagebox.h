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

#ifndef QKXMESSAGEBOX_H
#define QKXMESSAGEBOX_H


#include <QMessageBox>
#include <QPointer>

class QKxMessageBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit QKxMessageBox(QWidget *parent = nullptr);
    explicit QKxMessageBox(Icon icon, const QString &title, const QString &text,
                StandardButtons buttons = NoButton, QWidget *parent = nullptr,
                Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    virtual ~QKxMessageBox();

    static void message(QWidget *parent, const QString& title, const QString& content, bool warning = true);

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButton button0, QMessageBox::StandardButton button1 = NoButton);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &text,
                                            QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton question(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButtons buttons = StandardButtons(Yes | No), QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title, const QString &text,
                                               QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
private:
    static void prettyButtonText(QMessageBox *dlg, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton);
private:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent* e);

private slots:
    void onAdjustSize();
    void onNextMessage();
private:
    void message(const QString& title, const QString& content, bool warning = true);
    Q_INVOKABLE void showMessage(const QString& title, const QString& content, bool isWarning = true);
private:
    struct MessageBoxData {
        bool isWarning;
        QString title;
        QString content;
    };
    QPointer<QPushButton> m_btnNext;
    QList<MessageBoxData> m_queue;
};

#endif // QKXMESSAGEBOX_H
