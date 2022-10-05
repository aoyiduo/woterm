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

#ifndef QWOSERIALINPUT_H
#define QWOSERIALINPUT_H

#include <QWidget>

namespace Ui {
class QWoSerialInput;
}

class QWoSerialInput : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSerialInput(const QString& target, QWidget *parent = 0);
    ~QWoSerialInput();
    bool isTextMode();
    bool isAutoNewLine();
    void reset();
signals:
    void connectReady(const QString& name);
    void disconnect();
    void sendText(const QString& txt);
    void moreReady();

private slots:
    void onBtnRefleshClicked();
    void onBtnConnectCliecked();
    void onBtnDisconnectCliecked();
    void onBtnSendClicked();
private:
    Ui::QWoSerialInput *ui;
};

#endif // QWOSERIALINPUT_H
