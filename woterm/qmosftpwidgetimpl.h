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

#ifndef QMOSFTPWIDGETIMPL_H
#define QMOSFTPWIDGETIMPL_H

#include "qmoshowwidget.h"

class QQuickWidget;
class QQmlContext;
class QMoSftpAssist;
class QMoSftpRemoteModel;
class QMoSftpTransferAssist;

class QMoSftpWidgetImpl : public QMoShowWidget
{
    Q_OBJECT
public:
    explicit QMoSftpWidgetImpl(const QString& target, QWidget *parent = nullptr);
    virtual ~QMoSftpWidgetImpl();

signals:

private slots:
    void onWindowCloseArrived();
private:
    int m_gid;
    QPointer<QQuickWidget> m_quick;
    QPointer<QQmlContext> m_qmlContext;
    QPointer<QMoSftpAssist> m_sftp;
    QPointer<QMoSftpRemoteModel> m_model;
    QPointer<QMoSftpTransferAssist> m_sftpTransfer;
};

#endif // QMOSFTPWIDGETIMPL_H
