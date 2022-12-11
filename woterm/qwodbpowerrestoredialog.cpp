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

#include "qwodbpowerrestoredialog.h"
#include "ui_qwodbpowerrestoredialog.h"

#include "qwodbsftpdetaildialog.h"
#include "qwosetting.h"
#include "qkxmessagebox.h"
#include "qwosshconf.h"
#include "qwodbsftpdownsync.h"
#include "qwodbservermergemodel.h"
#include "qwodbgroupmergemodel.h"
#include "qwodbidentitymergemodel.h"
#include "qwodbsftpdownlistdialog.h"
#include "qkxcipher.h"
#include "qwoutils.h"

#include <QStringListModel>
#include <QTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QPushButton>

#include <SQLiteCpp/SQLiteCpp.h>
#include <QMouseEvent>

#define DB_UNCRYPT_NAME     ("woterm_restore_decryption.db")
QDBMergeActionDelegate::QDBMergeActionDelegate(QTableView *tblView, QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_parent(parent)
    , m_tblView(tblView) {
    m_btnAdd = new QPushButton(QIcon(":/woterm/resource/skin/add.png"), tr("Add"), parent);
    m_btnRemove = new QPushButton(QIcon(":/woterm/resource/skin/close.png"), tr("Remove"), parent);
    m_btnReplace = new QPushButton(QIcon(":/woterm/resource/skin/ftp.png"), tr("Replace"), parent);
    m_btnAdd->setObjectName("restoreButton");
    m_btnRemove->setObjectName("restoreButton");
    m_btnReplace->setObjectName("restoreButton");
    QSize sz(50,20);
    m_btnAdd->resize(sz);
    m_btnRemove->resize(sz);
    m_btnReplace->resize(sz);
    m_btnAdd->hide();
    m_btnRemove->hide();
    m_btnReplace->hide();
}

// make sure return true, or else it could not update.
bool QDBMergeActionDelegate::editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx)
{
    _editorEvent(ev, model, option, idx);
    return true;
}

bool QDBMergeActionDelegate::_editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx) {
    QMouseEvent *me = static_cast<QMouseEvent *> (ev);
    m_ptMouse = me->pos();
    QGuiApplication::restoreOverrideCursor();
    m_event = QEvent::None;
    QRect itemRt = option.rect;
    QString label = idx.data(Qt::DisplayRole).toString();
    QPushButton *btn = nullptr;
    QEvent::Type t = me->type();
    if(label == "replace") {
        btn = m_btnReplace;
    }else if(label == "add") {
        btn = m_btnAdd;
    }else if(label == "remove") {
        btn = m_btnRemove;
    }else{
        if(t == QEvent::MouseButtonPress) {
            m_tblView->setCurrentIndex(idx);
        }
        return false;
    }
    QRect rt = btn->rect();
    rt.adjust(-3, -3, 3, 3);
    rt.moveCenter(itemRt.center());
    QApplication::restoreOverrideCursor();
    if(!rt.contains(m_ptMouse)) {
        if(t == QEvent::MouseButtonPress) {
            m_tblView->setCurrentIndex(idx);
        }
        return false;
    }

    if(t == QEvent::MouseMove
            || t == QEvent::MouseButtonPress
            || t == QEvent::MouseButtonRelease) {
        // QApplication::setOverrideCursor(Qt::PointingHandCursor);
        m_event = t;
    }
    return true;
}

void QDBMergeActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const{
     QString label = idx.data(Qt::DisplayRole).toString();
    if(label.isEmpty()) {
        QStyledItemDelegate::paint(painter, option, idx);
    }else{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        QPalette pal = option.palette;
        painter->setBrush(Qt::NoBrush);
        if (option.state & QStyle::State_Selected){
            painter->fillRect(option.rect, pal.highlight());
            painter->setPen(Qt::white);
        }else{
            painter->setPen(Qt::black);
        }
        QStyleOptionButton button;
        button.state |= QStyle::State_Enabled;
        QPushButton *btn = nullptr;
        if(label == "replace") {
            btn = m_btnReplace;
        }else if(label == "add") {
            btn = m_btnAdd;
        }else if(label == "remove") {
            btn = m_btnRemove;
        }
        if(m_event == QEvent::MouseButtonPress) {
            button.state |= QStyle::State_Sunken;
        }else if(m_event == QEvent::MouseButtonRelease) {
            if(btn == m_btnAdd) {
                emit actionArrived(DB_MERGE_ACTION_ADD, idx);
            }else if(btn == m_btnRemove) {
                emit actionArrived(DB_MERGE_ACTION_REMOVE, idx);
            }else if(btn == m_btnReplace) {
                emit actionArrived(DB_MERGE_ACTION_REPLACE, idx);
            }

        }else if(m_event == QEvent::MouseMove) {
            button.state |= QStyle::State_MouseOver;
        }
        *((QEvent::Type*)&m_event) = QEvent::None;
        if(btn) {
            QRect itemRt = option.rect;
            QRect rt = btn->rect();
            rt.adjust(-3, -3, 3, 3);
            rt.moveCenter(itemRt.center());
            button.rect = rt;
            button.text = btn->text();
            button.icon = btn->icon();
            button.iconSize = btn->iconSize();
            QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter, btn);
        }
        painter->restore();
    }
}


QWoDBPowerRestoreDialog::QWoDBPowerRestoreDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoDBPowerRestoreDialog)
    , m_sftpMode(false)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    m_pathTemp = QWoSetting::tempPath();

    setWindowTitle(tr("Database restore"));

    ui->btnApply->setEnabled(false);
    ui->backupType->setModel(new QStringListModel(QStringList() << tr("sftp server") << tr("local file"), this));
    QObject::connect(ui->backupType, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentBackupIndexChanged()));
    onCurrentBackupIndexChanged();
    QStringList crypts;
    crypts.append("AES-CBC-256");
    crypts.append("AES-CTR-256");
    crypts.append("AES-GCM-256");
    crypts.append("DES-CBC");
    crypts.append("DES-ECB");
    crypts.append("DES-OFB64");
    crypts.append("RC4");
    crypts.append("Blowfish");
    ui->cryptType->setModel(new QStringListModel(crypts, ui->cryptType));
    ui->cryptType2->setModel(new QStringListModel(crypts, ui->cryptType2));
    ui->sftpServer->setReadOnly(true);

    QStringList items;
    items.append(tr("Session list"));
    items.append(tr("Group list"));
    items.append(tr("Identity file list"));
    ui->projectBox->setModel(new QStringListModel(items, ui->projectBox));
    QObject::connect(ui->projectBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentProjectIndexChanged()));

    QObject::connect(ui->btnSftpDetail, SIGNAL(clicked()), this, SLOT(onSftpDetailButtonClicked()));

    QString lastFile = QWoSetting::value("DBBackup/lastLocalFile").toString();
    ui->pathLocal->setText(lastFile);
    QString cryptType = QWoSetting::value("DBBackup/lastCryptType").toString();
    if(cryptType.isEmpty()) {
        ui->cryptType->setCurrentIndex(0);
        ui->cryptType2->setCurrentIndex(0);
    }else{
        ui->cryptType->setCurrentText(cryptType);
        ui->cryptType2->setCurrentText(cryptType);
    }
    QString cryptKey = QWoSetting::value("DBBackup/lastCryptKey").toString();
    ui->cryptKey->setText(cryptKey);
    ui->cryptKey2->setText(cryptKey);

    resetSftpUrl();

    QObject::connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(onMergeWidgetShow()));
    QObject::connect(ui->btnLast, SIGNAL(clicked()), this, SLOT(onTypeWidgetShow()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnExit, SIGNAL(clicked()), this, SLOT(close()));

    ui->localFilePath->setReadOnly(true);
    ui->sftpFilePath->setReadOnly(true);
    QObject::connect(ui->btnBackupList, SIGNAL(clicked()), this, SLOT(onBackupListClicked()));
    QObject::connect(ui->btnDecryptAgain, SIGNAL(clicked()), this, SLOT(onDecryptAgainClicked()));
    ui->mergeWidget->hide();
    ui->typeWidget->show();
    adjustSize();

    QObject::connect(ui->chkDiff, SIGNAL(clicked()), this, SLOT(onGroupModeClicked()));
    QObject::connect(ui->chkFull, SIGNAL(clicked()), this, SLOT(onGroupModeClicked()));
    QObject::connect(ui->chkReplace, SIGNAL(clicked()), this, SLOT(onGroupModeClicked()));


    m_modelServer = new QWoDBServerMergeModel(this);
    m_modelGroup = new QWoDBGroupMergeModel(this);
    m_modelIdentity = new QWoDBIdentityMergeModel(this);

    ui->tblMerge->setTabletTracking(true);
    QWidget *viewPort = ui->tblMerge->viewport();
    if(viewPort) {
        viewPort->setAttribute(Qt::WA_Hover,true);
        viewPort->setTabletTracking(true);
        viewPort->setMouseTracking(true);
    }
    QFont ft = ui->tblMerge->font();
    m_modelServer->setFont(ft);
    m_modelGroup->setFont(ui->tblMerge->font());
    m_modelIdentity->setFont(ui->tblMerge->font());

    ui->tblMerge->setModel(m_modelServer);
    ui->tblMerge->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblMerge->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblMerge->setSelectionMode(QAbstractItemView::SingleSelection);
    QDBMergeActionDelegate *delegate = new QDBMergeActionDelegate(ui->tblMerge, ui->tblMerge);
    QObject::connect(delegate, SIGNAL(actionArrived(int,QModelIndex)), this, SLOT(onActionArrived(int,QModelIndex)));
    ui->tblMerge->setItemDelegateForColumn(2, delegate);
    QHeaderView *hdrView = ui->tblMerge->horizontalHeader();
    if(hdrView != nullptr) {
        hdrView->setStretchLastSection(true);
        //hdrView->setSectionResizeMode(QHeaderView::Stretch);
    }
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onButtonApplyClicked()));
    setMinimumWidth(600);
    ui->tblMerge->setMinimumHeight(350);
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QWoDBPowerRestoreDialog::~QWoDBPowerRestoreDialog()
{
    delete ui;
}

void QWoDBPowerRestoreDialog::onCurrentBackupIndexChanged()
{
    int idx = ui->backupType->currentIndex();
    if(idx == 0) {
        // sftp server.
        ui->sftpArea->show();
        ui->localArea->hide();
    }else{
        ui->sftpArea->hide();
        ui->localArea->show();
    }
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
}

void QWoDBPowerRestoreDialog::onCurrentProjectIndexChanged()
{
    showMergeResult();
}

void QWoDBPowerRestoreDialog::onAdjustLayout()
{
    adjustSize();
}

void QWoDBPowerRestoreDialog::onSftpDetailButtonClicked()
{
    QWoDbSftpDetailDialog dlg(this);
    if(dlg.exec() != (QDialog::Accepted + 1)) {
        resetSftpUrl();
    }
}

void QWoDBPowerRestoreDialog::onDecryptAgainClicked()
{
    m_cryptKey = ui->cryptKey2->text();
    m_cryptType = ui->cryptType2->currentText();
    QString fileName = ui->sftpFilePath->text();
    if(decryptFile(fileName, DB_UNCRYPT_NAME)) {
        QString uncryptFile = m_pathTemp + "/" + DB_UNCRYPT_NAME;
        runMerge(uncryptFile);
    }
}

void QWoDBPowerRestoreDialog::onButtonApplyClicked()
{
    if(ui->chkReplace->isChecked()) {
        if(m_sftpMode) {
            QString dbFile = m_pathTemp + "/" + DB_UNCRYPT_NAME;
            QWoSshConf::instance()->restore(dbFile);
        }else{
            QString dbFile = ui->pathLocal->text();
            QWoSshConf::instance()->restore(dbFile);
        }
    }else{
        m_modelGroup->apply();
        m_modelIdentity->apply();
        m_modelServer->apply();
    }
    done(QDialog::Accepted);
}

void QWoDBPowerRestoreDialog::onMergeWidgetShow()
{
    m_sftpMode = ui->backupType->currentIndex() == 0;
    if(m_sftpMode) {
        QString server = ui->sftpServer->text();
        if(server.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("the server address should not be empty."));
            return;
        }
        QString key = ui->cryptKey->text();
        if(key.isEmpty()) {
            QKxMessageBox::information(this, tr("Parameter error"), tr("the crypt key should not be empty."));
            return;
        }
        m_cryptType = ui->cryptType->currentText();
        m_cryptKey = ui->cryptKey->text();
        ui->cryptType2->setCurrentText(m_cryptType);
        ui->cryptKey2->setText(m_cryptKey);
        ui->decryptArea->hide();
        ui->localFileArea->hide();
        ui->sftpFileArea->show();
        if(m_sync == nullptr){
            m_sync = new QWoDBSftpDownSync(this);
            QObject::connect(m_sync, SIGNAL(infoArrived(int,int,QString)), this, SLOT(onSyncInfoArrived(int,int,QString)));
            QObject::connect(m_sync, SIGNAL(fetchFinished(int,QString)), this, SLOT(onSyncFetchFinished(int,QString)));
        }
        m_sync->fetchLatest();
    }else{
        ui->decryptArea->hide();
        ui->localFileArea->show();
        ui->sftpFileArea->hide();
        ui->localFilePath->setText(ui->pathLocal->text());
        QMetaObject::invokeMethod(this, "fetchLocal", Qt::QueuedConnection);
    }
    ui->mergeWidget->show();
    ui->typeWidget->hide();
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
}

void QWoDBPowerRestoreDialog::onTypeWidgetShow()
{
    ui->mergeWidget->hide();
    ui->typeWidget->show();
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
}

void QWoDBPowerRestoreDialog::onBackupListClicked()
{
    if(m_sync) {
        QString fileName = QWoDBSftpDownListDialog::result(m_sync, this);
        if(fileName.isEmpty()) {
            return;
        }
        ui->sftpFilePath->setText(fileName);
        m_sync->fetch(fileName);
    }
}

void QWoDBPowerRestoreDialog::onGroupModeClicked()
{
    showMergeResult();
}

void QWoDBPowerRestoreDialog::onActionArrived(int action, const QModelIndex &idx)
{
    m_modelCurrent->runAction(action, idx);
}

#define DB_SFTP_DOWNLOAD_CHECK_VERSION        (1)
#define DB_SFTP_DOWNLOAD_DOWNLOAD_FILE        (2)

void QWoDBPowerRestoreDialog::onSyncInfoArrived(int action, int err, const QString &errDesc)
{
    ui->infoMsg->setText(errDesc);
}

void QWoDBPowerRestoreDialog::onSyncFetchFinished(int err, const QString &fileName)
{
    m_modelServer->clear();
    m_modelGroup->clear();
    m_modelIdentity->clear();
    if(err == 0) {
        ui->sftpFilePath->setText(fileName);
        if(decryptFile(fileName, DB_UNCRYPT_NAME)) {
            QString uncryptFile = m_pathTemp + "/" + DB_UNCRYPT_NAME;
            runMerge(uncryptFile);
        }
    }else{
        ui->sftpFilePath->setText("");
    }
}

void QWoDBPowerRestoreDialog::fetchLocal()
{
    QString dbFile = ui->pathLocal->text();
    if(!QWoSshConf::databaseValid(dbFile)) {
        QKxMessageBox::information(this, tr("Error"), tr("Failed to open file:%1").arg(dbFile));
        return;
    }
    runMerge(dbFile);
}

void QWoDBPowerRestoreDialog::resetSftpUrl()
{
    QVariantMap dm = QWoSetting::value("DBBackup/sftpDetail").toMap();
    QString host = dm.value("host").toString();
    QString name = dm.value("name").toString();
    QString path = dm.value("path", "~/woterm_db_backup").toString();
    QString port = dm.value("port", 22).toString();
    QString url = QString("sftp://%1@%2:%3?port=%4").arg(name, host, path, port);
    ui->sftpServer->setText(url);
}

bool QWoDBPowerRestoreDialog::decryptFile(const QString &fileName, const QString& fileNameDst)
{
    QString path = m_pathTemp + "/" + fileName;
    QFile lf(path);
    if(!lf.open(QIODevice::ReadOnly)) {
        QKxMessageBox::information(this, tr("Decryption error"), tr("Failed to open file:%1").arg(fileName));
        return false;
    }
    QByteArray all = lf.readAll();
    lf.close();
    QByteArray out;
    if(!decrypt(all, m_cryptType.toUtf8(), m_cryptKey.toUtf8(), out)) {
        QKxMessageBox::information(this, tr("Decryption error"), tr("Failed to decrypt the backup file:%1.").arg(fileName));
        if(!ui->decryptArea->isVisible()){
            ui->decryptArea->show();
            QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
        }
        return false;
    }
    qDebug() << "ready to do more";
    QString fileDecrypt = m_pathTemp + "/" + fileNameDst;
    QFile::remove(fileDecrypt);
    QFile df(fileDecrypt);
    if(!df.open(QIODevice::WriteOnly)) {
        QKxMessageBox::information(this, tr("Decryption error"), tr("Failed to write decrypt result to file:%1").arg(fileDecrypt));
        return false;
    }
    df.write(out);
    df.close();

    if(ui->decryptArea->isVisible()){
        ui->decryptArea->hide();
        QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
    }
    return true;
}

bool QWoDBPowerRestoreDialog::decrypt(const QByteArray& in, const QByteArray &type, const QByteArray &key, QByteArray &out)
{
    if(type == "AES-CBC-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesCbcEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "AES-CTR-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesCtrEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "AES-GCM-256") {
        QByteArray pass = QKxCipher::makeBytes(key, 32);
        QByteArray ivec = QKxCipher::makeBytes(key, 16);
        if(!QKxCipher::aesGcmEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "DES-CBC") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::tripleDesCbcEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "DES-ECB") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        if(!QKxCipher::tripleDesEcbEncrypt(in, out, pass, false)) {
            return false;
        }
    }else if(type == "DES-OFB64") {
        QByteArray pass = QKxCipher::makeBytes(key, 24);
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::tripleDesOfb64Encrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }else if(type == "RC4") {
        QByteArray pass = key;
        if(!QKxCipher::rc4Encrypt(in, out, pass, false)) {
            return false;
        }
    }else if(type == "Blowfish") {
        QByteArray pass = key;
        QByteArray ivec = QKxCipher::makeBytes(key, 8);
        if(!QKxCipher::blowfishEcbEncrypt(in, out, pass, ivec, false)) {
            return false;
        }
    }
    // strncmp(header, "SQLite format 3\000", 16)
    QByteArray header = out.left(16);
    if(!header.startsWith("SQLite format 3")) {
        return false;
    }
    return true;
}


bool QWoDBPowerRestoreDialog::runMerge(const QString &dbFile)
{
    m_modelGroup->clear();
    m_modelIdentity->clear();
    m_modelServer->clear();
    ui->btnApply->setEnabled(false);

    m_local.clear();
    m_remote.clear();
    QString file = QWoSetting::sshServerDbPath();
    if(!dbToMap(file, m_local)) {
        return false;
    }
    if(!dbToMap(dbFile, m_remote)) {
        return false;
    }
    QList<QString> names = m_remote.keys();
    for(auto it = names.begin(); it != names.end(); it++) {
        QString name = *it;
        QList<QVariantMap> local = m_local.value(name);
        QList<QVariantMap> remote = m_remote.value(name);
        if(name == "servers") {
            MergeInfo mi = runMergeList(local, remote);
            m_modelServer->setData(mi);
        }else if(name == "groups") {
            MergeInfo mi = runMergeList(local, remote);
            m_modelGroup->setData(mi);
        }else if(name == "identities") {
            MergeInfo mi = runMergeList(local, remote);
            m_modelIdentity->setData(mi);
        }
    }
    showMergeResult();
    ui->btnApply->setEnabled(true);
    return true;
}

MergeInfo QWoDBPowerRestoreDialog::runMergeList(const QList<QVariantMap> &_local, const QList<QVariantMap> &_remote) const
{
    QList<QVariantMap> local = _local;
    QList<QVariantMap> remote = _remote;
    QList<QVariantMap> same, replace, lhave, rhave;
    for(auto lt = local.begin(); lt != local.end(); lt++) {
        QVariantMap lm = *lt;
        QVariantMap item;
        item.insert("local", lm);
        QString name = lm.value("name").toString();
        bool hasFind = false;
        bool isSame = true;
        for(auto rt = remote.begin(); rt != remote.end(); rt++) {
            QVariantMap rm = *rt;
            QString nameHit = rm.value("name").toString();
            if(nameHit == name) {
                if(name == "kxtry.portmap8") {
                    qDebug() << lm << rm;
                }
                item.insert("remote", rm);
                isSame = lm == rm;
                item.insert("isSame", isSame);
                remote.erase(rt);
                hasFind = true;
                break;
            }
        }
        if(hasFind) {
            if(isSame) {
                same.append(item);
            }else{
                replace.append(item);
            }
        }else{
            lhave.append(item);
        }
    }
    for(auto rt = remote.begin(); rt != remote.end(); rt++) {
        QVariantMap rm = *rt;
        QVariantMap item;
        item.insert("remote", rm);
        rhave.append(item);
    }
    MergeInfo mi;
    mi.same = same;
    mi.lhave = lhave;
    mi.rhave = rhave;
    mi.replace = replace;
    return mi;
}

void QWoDBPowerRestoreDialog::showMergeResult()
{
    int idx = ui->projectBox->currentIndex();
    bool isReplace = ui->chkReplace->isChecked();
    ui->mergeArea->setVisible(!isReplace);
    if(!isReplace) {
        bool isFull = ui->chkFull->isChecked();
        QString msg;
        if(idx == 0) {
            // Session list
            msg = m_modelServer->reset(isFull);
            m_modelCurrent = m_modelServer;
            ui->tblMerge->setModel(m_modelServer);
        }else if(idx == 1) {
            // Group list
            msg = m_modelGroup->reset(isFull);
            m_modelCurrent = m_modelGroup;
            ui->tblMerge->setModel(m_modelGroup);
        }else if(idx == 2) {
            // Identity file list
            msg = m_modelIdentity->reset(isFull);
            m_modelCurrent = m_modelIdentity;
            ui->tblMerge->setModel(m_modelIdentity);
        }
        QMetaObject::invokeMethod(ui->tblMerge, "resizeRowsToContents", Qt::QueuedConnection);
        QMetaObject::invokeMethod(ui->tblMerge, "resizeColumnsToContents", Qt::QueuedConnection);

        QKxMessageBox::information(this, tr("Merge information"), msg);
    }
    QMetaObject::invokeMethod(this, "onAdjustLayout", Qt::QueuedConnection);
}

// Only relevant information affecting the connection network is synchronized,
// and configuration information is not synchronized.
#define PASSWORD_ENCRYPT    ("WoTerm@2022-11-6")
QList<QVariantMap> fetchList(SQLite::Database& db, const QString& tblName) {
    QList<QVariantMap> all;
    QString sql = QString("select * from %1").arg(tblName);
    SQLite::Statement query(db, sql.toUtf8());
    while(query.executeStep()) {
        QVariantMap dm;
        for(int i = 0; i < query.getColumnCount(); i++) {
            QString name = query.getColumnName(i);
            if(name == "id"
                    ||name == "ct"
                    ||name == "dt"
                    || name == "syncFlag"
                    || name == "delFlag"
                    || name == "version"
                    || name == "property") {
                continue;
            }
            SQLite::Column col = query.getColumn(i);
            if(col.isInteger()) {
                dm.insert(name, col.getInt());
            }else if(col.isText()) {
                QByteArray txt = col.getText();
                if(txt.isEmpty()) {
                    continue;
                }
                if(name == "loginPassword") {
                    if(txt == "WoTerm:") {
                        continue;
                    }
                    txt = QWoUtils::aesEncrypt(txt, PASSWORD_ENCRYPT);
                }
                dm.insert(name, txt);
            }
        }
        all.append(dm);
    }
    return all;
}

bool QWoDBPowerRestoreDialog::dbToMap(const QString &dbFile, QMap<QString, QList<QVariantMap> > &all)
{
    try{
        SQLite::Database db(dbFile.toUtf8(), SQLite::OPEN_READONLY);
        QList<QVariantMap> groups = fetchList(db, "groups");
        QList<QVariantMap> servers = fetchList(db, "servers");
        QList<QVariantMap> identities = fetchList(db, "identities");
        all.insert("groups", groups);
        all.insert("servers", servers);
        all.insert("identities", identities);
    }catch(std::exception& e) {
        QByteArray err = e.what();
        qDebug() << "dbToMap" << err;
        return false;
    }
    return true;
}

void QWoDBPowerRestoreDialog::init()
{

}
