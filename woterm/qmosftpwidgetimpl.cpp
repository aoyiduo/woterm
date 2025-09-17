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

#include "qmosftpwidgetimpl.h"

#include "qmoapplication.h"
#include "qwosshconf.h"
#include "qwosortfilterproxymodel.h"
#include "qwoutils.h"
#include "qwosetting.h"
#include "qwoidentify.h"
#include "qmomessageboxassist.h"
#include "qmosftpremotemodel.h"
#include "qmosftpassist.h"
#include "qmosftptransferassist.h"
#include "qmosftpqueuemodel.h"
#include "qmoquickassist.h"

#include <QBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QCoreApplication>

QMoSftpWidgetImpl::QMoSftpWidgetImpl(const QString& target, QWidget *parent)
    : QMoShowWidget(target, parent)
    , m_gid(QWoUtils::gid())
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    m_quick = new QQuickWidget(this);
    m_quick->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlEngine *engine = m_quick->engine();
    QWoUtils::injectJS(engine, QStringList() << ":/woterm/js/async.js" << ":/woterm/js/utils.js" );

    layout->addWidget(m_quick);

    m_model = new QMoSftpRemoteModel(this);
    m_sftp = new QMoSftpAssist(target, m_gid, m_model, this);
    m_sftpTransfer = new QMoSftpTransferAssist(target, m_gid, engine, this);

    m_qmlContext = m_quick->rootContext();
    m_qmlContext->setContextProperty("quick", new QMoQuickAssist(m_quick));
    m_qmlContext->setContextProperty("gTitle", target);
    m_qmlContext->setContextProperty("gApp", QMoApplication::instance());
    m_qmlContext->setContextProperty("gUtils", new QWoUtils(this));    
    m_qmlContext->setContextProperty("gMessageBox", new QMoMessageBoxAssist(this));
    m_qmlContext->setContextProperty("gSetting", new QWoSetting(this));
    m_qmlContext->setContextProperty("gSshConf", QWoSshConf::instance());
    m_qmlContext->setContextProperty("gIdentify", new QWoIdentify(this));
    m_qmlContext->setContextProperty("gSftpTransfer", m_sftpTransfer);
    m_qmlContext->setContextProperty("gSftpTransferModel", m_sftpTransfer->taskModel());
    m_qmlContext->setContextProperty("gSftpModel", m_model);
    m_qmlContext->setContextProperty("gSftp", m_sftp);

    QObject::connect(m_sftp, SIGNAL(closeArrived()), this, SLOT(onWindowCloseArrived()));
    QObject::connect(engine, SIGNAL(quit()), this, SLOT(onWindowCloseArrived()));

    m_quick->setSource(QUrl(QStringLiteral("qrc:/woterm/qml/sftp.qml")));
}

QMoSftpWidgetImpl::~QMoSftpWidgetImpl()
{

}

void QMoSftpWidgetImpl::onWindowCloseArrived()
{
    close();
}
