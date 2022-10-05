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

#ifndef QWOSFTPTRANSFERWIDGET_H
#define QWOSFTPTRANSFERWIDGET_H

#include <QWidget>

namespace Ui {
class QWoSftpTransferWidget;
}

class QWoSftpTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSftpTransferWidget(QWidget *parent = 0);
    ~QWoSftpTransferWidget();
    void setTip(const QString& tip);
    void progress(int v);
signals:
    void abort();
private slots:
    void onAbort();

protected:
    virtual void showEvent(QShowEvent *event);
private:
    Ui::QWoSftpTransferWidget *ui;
};

#endif // QWOSFTPTRANSFERWIDGET_H
