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

#include "qwosessionttyproperty.h"
#include "ui_qwosessionttyproperty.h"
#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qwoutils.h"
#include "qkxutils.h"
#include "qwosetting.h"
#include "qkxkeytranslatormodel.h"
#include "qkxmessagebox.h"
#include "qwofontlistmodel.h"
#include "qkxpositionitem.h"
#include "qkxbuttonassist.h"
#include "qwosshconf.h"
#include "qkxcombinekeyactiondialog.h"

#include <QTabBar>
#include <QBoxLayout>
#include <QStringListModel>
#include <QTimer>
#include <QTextCodec>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QPushButton>
#include <QMouseEvent>
#include <QToolButton>
#include <QDesktopServices>
#include <QInputDialog>
#include <QButtonGroup>

#define BUTTON_REMOVE_SIZE      (10)
#define BUTTON_MARGIN           (3)

QFontCleanDelegate::QFontCleanDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
    m_btnRemove = new QToolButton(parent);
    m_btnRemove->setIcon(QIcon("../private/skins/black/close.png"));
    m_btnRemove->setIconSize(QSize(BUTTON_REMOVE_SIZE,BUTTON_REMOVE_SIZE));
    QSize sz(BUTTON_REMOVE_SIZE,BUTTON_REMOVE_SIZE);
    m_btnRemove->resize(sz);
    m_btnRemove->hide();
}

bool QFontCleanDelegate::editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx)
{
    return _editorEvent(ev, model, option, idx);
}

void QFontCleanDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const
{
    bool isCustom = idx.data(ROLE_CUSTOM_FONT).toBool();
    QStyledItemDelegate::paint(painter, option, idx);
    if(!isCustom) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPalette pal = option.palette;
    painter->setBrush(Qt::NoBrush);
    QToolButton *btn = m_btnRemove;
    QStyleOptionButton button;
    button.state |= QStyle::State_Enabled;
    if(m_event == QEvent::MouseButtonPress) {
        button.state |= QStyle::State_Sunken;
        QString family = idx.data(Qt::DisplayRole).toString();
        emit removeArrived(family);
    }else if(m_event == QEvent::MouseButtonRelease) {
    }else if(m_event == QEvent::MouseMove) {
        button.state |= QStyle::State_MouseOver;
    }
    *((QEvent::Type*)&m_event) = QEvent::None;

    QRect rt = buttonRect(option.rect);

    button.rect = rt;
    button.text = btn->text();
    button.icon = btn->icon();
    button.iconSize = btn->iconSize();
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter, btn);
    painter->restore();
}

bool QFontCleanDelegate::_editorEvent(QEvent *ev, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &idx)
{
    QMouseEvent *me = static_cast<QMouseEvent *> (ev);
    m_ptMouse = me->pos();
    QGuiApplication::restoreOverrideCursor();
    QRect rt = buttonRect(option.rect);
    QApplication::restoreOverrideCursor();
    if(!rt.contains(m_ptMouse)) {
        m_event = QEvent::None;
        return true;
    }
    QEvent::Type t = me->type();
    if(t == QEvent::MouseMove
            || t == QEvent::MouseButtonPress
            || t == QEvent::MouseButtonRelease) {
        m_event = t;
    }
    return true;
}

QRect QFontCleanDelegate::buttonRect(const QRect &itemRt) const
{
    QRect rt = m_btnRemove->rect();
    rt.adjust(-3, -3, 3, 3);
    rt.moveCenter(itemRt.center());
    rt.translate((itemRt.width() - rt.width()) / 2 - BUTTON_MARGIN, 0);
    return rt;
}

QWoSessionTTYProperty::QWoSessionTTYProperty(ETTYType type, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSessionTTYProperty)
    , m_bCustom(false)
    , m_ttyType(type)
{
    ui->setupUi(this);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    m_tabBar = new QTabBar(this);
    layout->insertWidget(0, m_tabBar);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("TTY Properties"));
    m_tabBar->addTab(tr("Pattern"));
    m_tabBar->addTab(tr("Shortcut"));
    m_tabBar->addTab(tr("Other"));
    m_tabBar->addTab(tr("Background image"));
    QObject::connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(onPageCurrentChanged(int)));
    m_tabBar->setCurrentIndex(0);
    ui->stacked->setCurrentWidget(ui->pattern);

    m_preview = new QKxTermWidget(this);
    m_term = m_preview->termItem();
    m_term->setTermName("preview");
    m_term->showTermName(false);
    m_preview->setFixedHeight(150);
    ui->previewLayout->addWidget(m_preview);
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(1000);
    setFixPreviewString();

    QStringList schemas = m_term->availableColorSchemas();
    schemas.sort();
    ui->schema->setModel(new QStringListModel(schemas, this));
    QObject::connect(ui->schema, SIGNAL(currentIndexChanged(QString)),  this, SLOT(onColorCurrentIndexChanged(QString)));

    m_keysModel = new QKxKeyTranslatorModel(QWoSetting::privatePath(), ui->keymap->font(), this);
    ui->keymap->setModel(m_keysModel);
    ui->keymap->setIconSize(QSize(32, 32));

    QStringList kbly = QKxUtils::availableKeytabs();
    ui->keytab->setModel(new QStringListModel(kbly, this));
    QObject::connect(ui->keytab, SIGNAL(currentIndexChanged(QString)), this, SLOT(onKeytabCurrentIndexChanged(QString)));
    ui->keytab->setCurrentIndex(-1);
    ui->keytab->setCurrentText(DEFAULT_KEY_TRANSLATOR);

    QList<QString> codec;
    foreach(QByteArray c, QTextCodec::availableCodecs()) {
        codec.append(c);
    }
    std::sort(codec.begin(), codec.end());
    ui->codepage->setModel(new QStringListModel(codec, this));
    ui->codepage->setEditable(false);

    ui->fontChooser->setEditable(false);
    m_delegate = new QFontCleanDelegate(ui->fontChooser);
    ui->fontChooser->setItemDelegate(m_delegate);
    QWoFontListModel *model = new QWoFontListModel(this);
    ui->fontChooser->setModel(model);

    QObject::connect(ui->keymap, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onKeymapItemDBClicked(QModelIndex)));
    QHeaderView *hdrView = ui->keymap->header();
    if(hdrView != nullptr) {
        hdrView->setStretchLastSection(true);
        hdrView->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    QObject::connect(m_delegate, SIGNAL(removeArrived(QString)), this, SLOT(onFontFamilyRemove(QString)));
    QObject::connect(ui->fontChooser, SIGNAL(currentIndexChanged(QString)), this, SLOT(onFontCurrentIndexChanged(QString)));
    QObject::connect(ui->fontSize, SIGNAL(valueChanged(int)), this, SLOT(onFontValueChanged(int)));
    QObject::connect(ui->blockCursor, SIGNAL(toggled(bool)), this, SLOT(onBlockCursorToggled()));
    QObject::connect(ui->underlineCursor, SIGNAL(toggled(bool)), this, SLOT(onUnderlineCursorToggled()));
    QObject::connect(ui->beamCursor, SIGNAL(toggled(bool)), this, SLOT(onBeamCursorToggled()));

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnSaveToAll, SIGNAL(clicked()), this, SLOT(onButtonSaveToAllClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    {
        QObject::connect(ui->chkDragCopyPaste, &QRadioButton::clicked, this, [=](){
            if(ui->chkDragCopyPaste->isChecked()) {
                ui->chkDragToInput->setChecked(false);
            }
        });
        QObject::connect(ui->chkDragToInput, &QRadioButton::clicked, this, [=](){
            if(ui->chkDragToInput->isChecked()) {
                ui->chkDragCopyPaste->setChecked(false);
            }
        });
    }

    {
        QObject::connect(ui->chkRKeyCopyPaste, &QRadioButton::clicked, this, [=](){
            if(ui->chkRKeyCopyPaste->isChecked()) {
                ui->chkRKeyCopy->setChecked(false);
            }
        });
        QObject::connect(ui->chkRKeyCopy, &QRadioButton::clicked, this, [=](){
            if(ui->chkRKeyCopy->isChecked()) {
                ui->chkRKeyCopyPaste->setChecked(false);
            }
        });
    }

    if(m_ttyType == ETTY_LocalShell) {
        ui->shellPathGroup->setVisible(true);
        ui->shellPath->setReadOnly(true);
        QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onShellPathButtonClicked()));
    }else{
        ui->shellPathGroup->setVisible(false);
    }

    {
        QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        ui->systemFont->setFont(ft);
        int pointSize = ft.pointSize();
        QString family = ft.family();
        QString label = tr("System recommand:") + QString("%1,%2").arg(family).arg(pointSize);
        ui->systemFont->setText(label);
    }

    QObject::connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onButtonImportClicked()));

    {
        ui->frame->layout()->deleteLater();
        QVBoxLayout *layout = new QVBoxLayout(ui->frame);
        m_item = new QKxPositionItem(ui->frame);
        m_item->setFixedSize(100, 100);
        QObject::connect(m_item, SIGNAL(clicked(int)), ui->chkTileNone, SLOT(click()));
        layout->addWidget(m_item);
    }
    {
        int val = QWoSetting::terminalBackgroundImageAlpha();
        ui->bkImgAlpha->setRange(30, 220);
        ui->bkImgAlpha->setValue(val);
        ui->alphaValue->setText(QString::number(val));
    }
    {
        bool smooth = QWoSetting::terminalBackgroundImageEdgeSmooth();
        ui->chkEdgeSmooth->setChecked(smooth);
    }
    {
        QString path = QWoSetting::terminalBackgroundImage();
        ui->bkImage->setText(path);

        QKxButtonAssist *btn = new QKxButtonAssist("../private/skins/black/remove.png", ui->bkImage);
        QObject::connect(btn, SIGNAL(clicked(int)), ui->bkImage, SLOT(clear()));
    }

    {
        QString pos = QWoSetting::terminalBackgroundImagePosition();
        m_item->setPositionAsString(pos);
    }

    {
        QObject::connect(ui->btnClone, SIGNAL(clicked()), this, SLOT(onCloneButtonClicked()));
        QObject::connect(ui->btnReload, SIGNAL(clicked()), this, SLOT(onReloadButtonClicked()));
        QObject::connect(ui->btnRename, SIGNAL(clicked()), this, SLOT(onRenameButtonClicked()));
        QObject::connect(ui->btnRemove, SIGNAL(clicked()), this, SLOT(onDeleteButtonClicked()));
        QObject::connect(ui->btnWrite, SIGNAL(clicked()), this, SLOT(onWriteButtonClicked()));

        QObject::connect(ui->btnKeyAdd, SIGNAL(clicked()), this, SLOT(onKeyAddButtonClicked()));
        QObject::connect(ui->btnKeyCopy, SIGNAL(clicked()), this, SLOT(onKeyCopyButtonClicked()));
        QObject::connect(ui->btnKeyModify, SIGNAL(clicked()), this, SLOT(onKeyModifyButtonClicked()));
        QObject::connect(ui->btnKeyRemove, SIGNAL(clicked()), this, SLOT(onKeyRemoveButtonClicked()));
    }

    QObject::connect(ui->chkTileNone, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileH, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileV, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileAll, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onSelectButtonClicked()));

    QObject::connect(ui->bkImgAlpha, SIGNAL(valueChanged(int)), this, SLOT(onBkImageAlphaValueChanged(int)));

    QObject::connect(ui->btnKeyHelp, &QAbstractButton::clicked, this, [=](){
        QString url = QWoSetting::isChineseLanguageFile() ? "http://cn.woterm.com" : "http://en.woterm.com";
        url += "/docs/manual/termopts/ttyopts/";
        QDesktopServices::openUrl(url);
    });
    initDefault();
    adjustSize();
}

QWoSessionTTYProperty::~QWoSessionTTYProperty()
{
    delete ui;
}

void QWoSessionTTYProperty::setCustom(const QVariantMap &_mdata)
{
    QVariantMap global = QWoSetting::ttyDefault();
    QVariantMap mdata = HostInfo::merge(global, _mdata);
    m_bCustom = true;

    {
        QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
        ui->schema->setCurrentText(schema);
        m_term->setColorSchema(schema);
    }
    {
        QString name = mdata.value("keyTranslator", DEFAULT_KEY_TRANSLATOR).toString();
        ui->keytab->setCurrentText(name);
    }
    {
        QString name = mdata.value("textCodec", DEFAULT_TEXT_CODEC).toString();
        ui->codepage->setCurrentText(name);
        m_term->setTextCodec(name);
    }
    {
        QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        QString fontName = mdata.value("fontName", ft.family()).toString();
        int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();
        ui->fontSize->setValue(fontSize);
        ui->fontChooser->setCurrentText(fontName);
        m_term->setTerminalFont(fontName, fontSize);
        refleshFontPreview();
    }
    {
        QString cursorType = mdata.value("cursorType", "block").toString();
        if(cursorType.isEmpty() || cursorType == "block") {
            ui->blockCursor->setChecked(true);
        }else if(cursorType == "underline") {
            ui->underlineCursor->setChecked(true);
        }else {
            ui->beamCursor->setChecked(true);
        }
    }
    {
        QString line = mdata.value("historyLength", QString("%1").arg(DEFAULT_HISTORY_LINE_LENGTH)).toString();
        ui->lineSize->setText(line);
    }
    {
        bool checked = mdata.value("dragPaste", false).toBool();
        ui->chkDragCopyPaste->setChecked(checked);
    }
    {
        bool checked = mdata.value("dragInput", false).toBool();
        ui->chkDragToInput->setChecked(checked);
    }
    {
        bool checked = mdata.value("selectCopy", false).toBool();
        ui->chkSelectToCopy->setChecked(checked);
    }
    {
        bool checked = mdata.value("rkeyCopy", false).toBool();
        ui->chkRKeyCopy->setChecked(checked);
    }
    {
        bool checked = mdata.value("rkeyPaste", false).toBool();
        ui->chkRKeyCopyPaste->setChecked(checked);
    }

    {
        QString path = mdata.value("shellPath").toString();
        ui->shellPath->setText(path);
    }

    {
        QString name = mdata.value("keyTranslator", DEFAULT_KEY_TRANSLATOR).toString();
        if(!m_keysModel->load(name)) {
            reloadKeytabsLater(name);
        }
    }
}

QVariantMap QWoSessionTTYProperty::result() const
{
    return m_result;
}

void QWoSessionTTYProperty::onCloneButtonClicked()
{
    QString name = ui->keytab->currentText();
    QString path = QKxUtils::keytabPath(name);
    QFileInfo fi(path);
    if(!fi.exists()) {
        QKxMessageBox::information(this, tr("File information"), tr("File is not exist."));
        return;
    }
    int tryLeft = 3;
    QString fileName = name;
    QStringList keytabsAll = QKxUtils::availableKeytabs();
    while(tryLeft-- > 0) {
        fileName = QInputDialog::getText(this, tr("File name"), tr("Input a file name to save."), QLineEdit::Normal, fileName);
        if(fileName.isEmpty()) {
            return;
        }
        if(keytabsAll.contains(fileName)) {
            QKxMessageBox::information(this, tr("File information"), tr("A file with the same name already exists，Please input a new one."));
            continue;
        }
        QString pathCustom = QWoSetting::customKeytabPath();
        pathCustom = pathCustom+"/"+fileName;
        if(!fileName.endsWith(".keytab")) {
            pathCustom += ".keytab";
        }
        if(QFile::copy(path, pathCustom)) {
            reloadKeytabsLater(fileName);
            return;
        }
        QKxMessageBox::information(this, tr("File information"), tr("Failed to create file. Please check if the file name is a valid file name."));
    }
}

void QWoSessionTTYProperty::onReloadButtonClicked()
{
    reloadKeytabsLater();
}

void QWoSessionTTYProperty::onRenameButtonClicked()
{
    QString name = ui->keytab->currentText();
    if(isPrivateKeytab(name)) {
        QKxMessageBox::information(this, tr("Rename information"), tr("Internal private directory does not support this operation."));
        return;
    }
    QString path = QKxUtils::keytabPath(name);
    QFileInfo fi(path);
    QString fileName = QInputDialog::getText(this, tr("File information"), tr("Input a new file name"), QLineEdit::Normal, name);
    if(fileName.isEmpty()) {
        return;
    }
    QStringList all = QKxUtils::availableKeytabs();
    if(all.contains(fileName)) {
        QKxMessageBox::information(this, tr("Rename information"), tr("The name already exist, please input a new one."));
        return;
    }
    QString pathNew = fi.absolutePath();
    pathNew += "/" + fileName;
    if(!pathNew.endsWith(".keytab")) {
        pathNew += ".keytab";
    }
    if(!QFile::rename(fi.absoluteFilePath(), pathNew)) {
        QKxMessageBox::information(this, tr("Rename information"), tr("Failed to rename file")+":"+name);
        return;
    }
    reloadKeytabsLater(fileName);
}

void QWoSessionTTYProperty::onDeleteButtonClicked()
{
    QString name = ui->keytab->currentText();
    if(isPrivateKeytab(name)) {
        QKxMessageBox::information(this, tr("Delete information"), tr("Internal private directory does not support this operation."));
        return;
    }
    QString path = QKxUtils::keytabPath(name);
    if(QFile::remove(path)) {
        reloadKeytabsLater();
    }
}

void QWoSessionTTYProperty::onWriteButtonClicked()
{
    QString name = ui->keytab->currentText();
    if(isPrivateKeytab(name)) {
        QKxMessageBox::information(this, tr("Operation information"), tr("Modifying internal private files is not supported. Please clone a file before editing."));
        return;
    }
    m_keysModel->save(":/woterm/resource/keytab.template");
}

void QWoSessionTTYProperty::onKeyAddButtonClicked()
{
    if(!switchToModify()) {
        return;
    }
    QKxCombineKeyActionDialog dlg(this);
    QObject::connect(&dlg, &QKxCombineKeyActionDialog::messageArrived, this, [=](const QString& title, const QString& msg){
        QKxMessageBox::information(this, title, msg);
    });
    if(dlg.exec() == QDialog::Accepted+1) {
        QKxKeyTranslator::KeyInfo ki = dlg.result();
        QModelIndex idx = m_keysModel->add(ki);
        ui->keymap->setCurrentIndex(idx);
        ui->keymap->scrollTo(idx);
    }
}

void QWoSessionTTYProperty::onKeyCopyButtonClicked()
{
    if(!switchToModify()) {
        return;
    }

    QModelIndex idx = ui->keymap->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("No selection to modify"));
        return;
    }
    QKxKeyTranslator::KeyInfo ki;
    if(!m_keysModel->data(idx.row(), ki)) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("No selection to modify"));
        return;
    }
    QModelIndex idx2 = m_keysModel->add(ki);
    ui->keymap->setCurrentIndex(idx2);
}

void QWoSessionTTYProperty::onKeyModifyButtonClicked()
{
    if(!switchToModify()) {
        return;
    }
    QModelIndex idx = ui->keymap->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("No selection to modify"));
        return;
    }
    onKeymapItemDBClicked(idx);
}

void QWoSessionTTYProperty::onKeyRemoveButtonClicked()
{
    if(!switchToModify()) {
        return;
    }
    QModelIndex idx = ui->keymap->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("No selection to modify"));
        return;
    }
    m_keysModel->remove(idx);
    if(idx.row() >= m_keysModel->count()) {
        idx = idx.siblingAtRow(idx.row()-1);
    }
    ui->keymap->setCurrentIndex(idx);
    ui->keymap->scrollTo(idx);
}

void QWoSessionTTYProperty::onKeymapItemDBClicked(const QModelIndex &idx)
{
    if(!switchToModify()) {
        return;
    }
    QKxCombineKeyActionDialog dlg(this);
    QObject::connect(&dlg, &QKxCombineKeyActionDialog::messageArrived, this, [=](const QString& title, const QString& msg){
        QKxMessageBox::information(this, title, msg);
    });
    QKxKeyTranslator::KeyInfo ki;
    if(!m_keysModel->data(idx.row(), ki)) {
        QKxMessageBox::information(this, tr("Parameter error"), tr("No selection to modify"));
        return;
    }
    dlg.init(ki);
    if(dlg.exec() == QDialog::Accepted+1) {
        QKxKeyTranslator::KeyInfo ki = dlg.result();
        QModelIndex idx2 = m_keysModel->modify(idx, ki);
        ui->keymap->setCurrentIndex(idx2);
        ui->keymap->scrollTo(idx2);
    }
}

void QWoSessionTTYProperty::onPageCurrentChanged(int idx)
{
    switch (idx) {
    case 0:
        ui->stacked->setCurrentWidget(ui->pattern);
        break;
    case 1:
        ui->stacked->setCurrentWidget(ui->shortcut);
        break;
    case 2:
        ui->stacked->setCurrentWidget(ui->other);
        break;
    case 3:
        ui->stacked->setCurrentWidget(ui->bkimg);
        break;
    }
}

void QWoSessionTTYProperty::onColorCurrentIndexChanged(const QString &txt)
{
    m_term->setColorSchema(txt);
}

void QWoSessionTTYProperty::onKeytabCurrentIndexChanged(const QString &txt)
{
    if(txt.isEmpty()) {
        return;
    }


    QString path = QDir::cleanPath(QKxUtils::keytabPath(txt));
    QString pathPrv = QDir::cleanPath(QWoSetting::privatePath());
    bool isPrv = path.startsWith(pathPrv);
    ui->btnRemove->setVisible(!isPrv);
    ui->btnRename->setVisible(!isPrv);
    ui->btnWrite->setVisible(!isPrv);

    //ui->btnKeyAdd->setEnabled(!isPrv);
    //ui->btnKeyCopy->setEnabled(!isPrv);
    //ui->btnKeyModify->setEnabled(!isPrv);
    //ui->btnKeyRemove->setEnabled(!isPrv);

    saveShortcut();
    if(!m_keysModel->load(txt)) {
        QKxMessageBox::information(this, tr("Shortcut information"), tr("Failed to load keytab file")+":"+txt);
        reloadKeytabsLater(DEFAULT_KEY_TRANSLATOR);
    }
}

void QWoSessionTTYProperty::onFontCurrentIndexChanged(const QString &family)
{
    QStringList styles = QKxUtils::familyStyles(family);
    refleshFontPreview();
}

void QWoSessionTTYProperty::onStyleCurrentIndexChanged(const QString &style)
{
    refleshFontPreview();
}

void QWoSessionTTYProperty::onFontValueChanged(int v)
{
    refleshFontPreview();
}

void QWoSessionTTYProperty::onBkImageAlphaValueChanged(int v)
{
    ui->alphaValue->setText(QString::number(v));
}

void QWoSessionTTYProperty::onBlockCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_Block);
}

void QWoSessionTTYProperty::onUnderlineCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_Underline);
}

void QWoSessionTTYProperty::onBeamCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_IBeam);
}

void QWoSessionTTYProperty::onTimeout()
{
    setFixPreviewString();
}

void QWoSessionTTYProperty::onButtonSaveClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    done(QDialog::Accepted);
}

void QWoSessionTTYProperty::onButtonSaveToAllClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    QWoSetting::setTtyDefault(m_result);
    QWoSshConf::instance()->removeProperties(SshWithSftp);
    m_result = QVariantMap();
    done(QDialog::Accepted+1);
}

void QWoSessionTTYProperty::onShellPathButtonClicked()
{
#ifdef Q_OS_WIN
    QString sufix = tr("Program Files (*.exe)");
#else
    QString sufix;
#endif
    QString path = ui->shellPath->text();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Program Files"), path, sufix);
    if(!fileName.isEmpty()) {
        fileName = QDir::toNativeSeparators(fileName);
        ui->shellPath->setText(fileName);
    }
}

void QWoSessionTTYProperty::onButtonImportClicked()
{
    QString pathLast = QWoSetting::value("fontImport/lastPath", QDir::homePath()).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Font file"), pathLast);
    if(fileNames.isEmpty()) {
        return;
    }
    QFileInfo fi(fileNames.first());
    pathLast = fi.absolutePath();
    QWoSetting::setValue("fontImport/lastPath", pathLast);
    QWoFontListModel *model = qobject_cast<QWoFontListModel*>(ui->fontChooser->model());
    QStringList all = model->stringList();
    for(int i = 0; i < fileNames.length(); i++) {
        QString fileName = fileNames.at(i);
        importFont(all, fileName);
    }
    model->reload();
    QStringList families = model->stringList();
    QString txtSel = ui->fontChooser->currentText();
    if(families.contains(txtSel)) {
        ui->fontChooser->setCurrentText(txtSel);
    }else{
        ui->fontChooser->setCurrentIndex(0);
    }
}

void QWoSessionTTYProperty::onFontFamilyRemove(const QString &family)
{
    QStringList fileErrs;
    QKxUtils::removeCustomFontFamily(family, fileErrs);

    QWoFontListModel *model = qobject_cast<QWoFontListModel*>(ui->fontChooser->model());
    QMetaObject::invokeMethod(model, "reload", Qt::QueuedConnection);
}

void QWoSessionTTYProperty::onTileButtonClicked()
{
    QRadioButton *btn = qobject_cast<QRadioButton*>(sender());
    if(ui->chkTileNone == btn) {
        m_item->setTileMode(0);
    }else if(ui->chkTileH == btn){
        m_item->setTileMode(1);
    }else if(ui->chkTileV == btn) {
        m_item->setTileMode(2);
    }else if(ui->chkTileAll == btn) {
        m_item->setTileMode(3);
    }
}

void QWoSessionTTYProperty::onSelectButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Image file"), QDir::homePath(), "Images (*.png *.jpg)");
    if(file.isEmpty()) {
        return;
    }
    QImage img(file);
    if(img.isNull() || img.width() < 128 || img.height() < 128) {
        QKxMessageBox::information(this, tr("Image information"), tr("Image size cannot be smaller than 128*128"));
        return;
    }

    ui->bkImage->setText(file);
}

void QWoSessionTTYProperty::saveShortcut(bool force)
{
    if(m_keysModel->hasModified()) {
        if(force) {
            m_keysModel->save(":/woterm/resource/keytab.template");
            return;
        }
        int code = QKxMessageBox::information(this, tr("Modify information"), tr("The shortcut file[%1] has been modified, do you need to save it?").arg(m_keysModel->name()), QMessageBox::Yes|QMessageBox::No);
        if(code == QMessageBox::Yes) {
            m_keysModel->save(":/woterm/resource/keytab.template");
        }
    }
}

QVariantMap QWoSessionTTYProperty::save()
{
    QVariantMap mvar;

    saveShortcut();

    QString schema = ui->schema->currentText();
    QString keytab = ui->keytab->currentText();
    QString textcodec = ui->codepage->currentText();
    QString fontName = ui->fontChooser->currentText();
    int fontSize = ui->fontSize->value();
    QString cursorType = ui->blockCursor->isChecked() ? "block" : (ui->beamCursor->isChecked() ? "beam": "underline");
    QString lineSize = ui->lineSize->text();
    mvar.insert("colorSchema", schema);
    mvar.insert("keyTranslator", keytab);
    mvar.insert("textcodec", textcodec);
    mvar.insert("fontName", fontName);
    mvar.insert("fontSize", fontSize);
    mvar.insert("cursorType", cursorType);
    mvar.insert("historyLength", lineSize);
    mvar.insert("rkeyCopy", ui->chkRKeyCopy->isChecked());
    mvar.insert("rkeyPaste", ui->chkRKeyCopyPaste->isChecked());
    mvar.insert("dragPaste", ui->chkDragCopyPaste->isChecked());
    mvar.insert("dragInput", ui->chkDragToInput->isChecked());
    mvar.insert("selectCopy", ui->chkSelectToCopy->isChecked());

    if(m_ttyType == ETTY_LocalShell) {
        mvar.insert("shellPath", ui->shellPath->text());
    }

    if(!m_bCustom) {
        QWoSetting::setTtyDefault(mvar);
    }


    /*set background image*/
    QString path = ui->bkImage->text();
    QWoSetting::setTerminalBackgroundImage(path);
    QWoSetting::setTerminalBackgroundImageAlpha(ui->bkImgAlpha->value());
    QWoSetting::setTerminalBackgroundImageEdgeSmooth(ui->chkEdgeSmooth->isChecked());
    QWoSetting::setTerminalBackgroundImagePosition(m_item->postionAsString());

    return mvar;
}

void QWoSessionTTYProperty::initDefault()
{
    QVariantMap mdata = QWoSetting::ttyDefault();

    QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
    ui->schema->setCurrentText(schema);
    m_term->setColorSchema(schema);

    QString keytab = mdata.value("keyTranslator", DEFAULT_KEY_TRANSLATOR).toString();
    ui->keytab->setCurrentText(keytab);
    m_term->setKeyTranslatorByName(keytab);

    QString codec = mdata.value("textCodec", DEFAULT_TEXT_CODEC).toString();
    ui->codepage->setCurrentText(codec);
    m_term->setTextCodec(codec);

    QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString fontName = mdata.value("fontName", ft.family()).toString();
    ui->fontChooser->setCurrentText(fontName);
    int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();
    ui->fontSize->setValue(fontSize);
    m_term->setTerminalFont(fontName, fontSize);
    refleshFontPreview();

    QString cursorType = mdata.value("cursorType", "block").toString();
    if(cursorType == "block") {
        ui->blockCursor->setChecked(true);
    }else if(cursorType == "underline") {
        ui->underlineCursor->setChecked(true);
    }else {
        ui->beamCursor->setChecked(true);
    }
    QString line = mdata.value("historyLength", QString("%1").arg(DEFAULT_HISTORY_LINE_LENGTH)).toString();
    ui->lineSize->setText(line);
    {
        bool checked = mdata.value("dragPaste", false).toBool();
        ui->chkDragCopyPaste->setChecked(checked);
    }
    {
        bool checked = mdata.value("dragInput", false).toBool();
        ui->chkDragToInput->setChecked(checked);
    }    
    {
        bool checked = mdata.value("rkeyCopy", false).toBool();
        ui->chkRKeyCopy->setChecked(checked);
    }
    {
        bool checked = mdata.value("rkeyPaste", false).toBool();
        ui->chkRKeyCopyPaste->setChecked(checked);
    }
    {
        bool checked = mdata.value("selectCopy", false).toBool();
        ui->chkSelectToCopy->setChecked(checked);
    }

    if(m_ttyType == ETTY_LocalShell){
        QString shellPath = mdata.value("shellPath", QWoUtils::findShellPath()).toString();
        ui->shellPath->setText(shellPath);
    }

    {
        QString name = mdata.value("keyTranslator", DEFAULT_KEY_TRANSLATOR).toString();
        if(!m_keysModel->load(name)) {
            reloadKeytabsLater(name);
        }
    }
}

void QWoSessionTTYProperty::setFixPreviewString()
{
    m_term->clearAll();    
    m_term->parseTest();
}

void QWoSessionTTYProperty::refleshFontPreview()
{
    QString family = ui->fontChooser->currentText();
    int pt = ui->fontSize->value();
    int pointSize = QKxUtils::suggestFontSize(family, pt);
    QFontInfo fi = m_term->setTerminalFont(family, pointSize);
    if(family != fi.family() || pt != pointSize) {
        QString txt = QString("%1: %2(%3,%4),%5(%6,%7)").arg(tr("Input does not match actual")).arg(tr("input")).arg(family).arg(pt).arg(tr("output")).arg(fi.family()).arg(fi.pointSize());
        qDebug() << txt;
        ui->fontInfo->setVisible(true);
        ui->fontInfo->setText(txt);
    }else{
        ui->fontInfo->setVisible(false);
    }
}

bool QWoSessionTTYProperty::isPrivateKeytab(const QString &name)
{
    QString path = QDir::cleanPath(QKxUtils::keytabPath(name));
    QString pathPrv = QDir::cleanPath(QWoSetting::privatePath());
    return path.startsWith(pathPrv);
}

bool QWoSessionTTYProperty::importFont(const QStringList& allFamilies, const QString &fileName)
{
    QFileInfo fi(fileName);
    int fid = QFontDatabase::addApplicationFont(fileName);
    if(fid < 0) {
        QKxMessageBox::warning(this, tr("Import errors"), tr("The font format is not supported.")+QString("[%1]").arg(fileName));
        return false;
    }
    QStringList families = QFontDatabase::applicationFontFamilies(fid);
    QFontDatabase fdb;
    for(auto it = families.begin(); it != families.end(); ) {
        QString name = *it;
        if(allFamilies.contains(name)) {
            it = families.erase(it);
            continue;
        }
        if(!fdb.isSmoothlyScalable(name)) {
            it = families.erase(it);
            continue;
        }
        if(!fdb.isFixedPitch(name)) {
            it = families.erase(it);
        }else{
            it++;
        }
    }
    QFontDatabase::removeApplicationFont(fid);
    if(families.isEmpty()) {
        QKxMessageBox::warning(this, tr("Import errors"), tr("The font to be imported does not meet the requirements for terminal use.")+QString("[%1]").arg(fileName));
        return false;
    }
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly)) {
        QKxMessageBox::warning(this, tr("Import errors"), tr("Failed to open the font file.")+QString("[%1,%2]").arg(fileName).arg(f.errorString()));
        return false;
    }
    QString path = QWoSetting::customFontPath();
    QString fileNew = path + "/" + fi.fileName();
    if(!f.copy(fileNew)) {
        QKxMessageBox::warning(this, tr("Import errors"), tr("Failed to backup the font file.")+QString("[%1,%2]").arg(fileName).arg(f.errorString()));
        return false;
    }
    return true;
}

void QWoSessionTTYProperty::reloadKeytabsLater(const QString& name)
{
    QString nameNow = ui->keytab->currentText();
    QKxUtils::cleanupKeytabs();
    QStringList all = QKxUtils::availableKeytabs();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->keytab->model());
    model->setStringList(all);

    QString target = name;
    if(name.isEmpty() || !all.contains(name)) {
        target = nameNow;
    }
    if(target.isEmpty() || !all.contains(target)) {
        target = DEFAULT_KEY_TRANSLATOR;
    }
    ui->keytab->setCurrentText(target);
}

bool QWoSessionTTYProperty::switchToModify()
{
    QString name = ui->keytab->currentText();
    if(isPrivateKeytab(name)) {
        QStringList keytabsAll = QKxUtils::availableKeytabs();
        for(int i = 0; i < 3; i++) {
            QStringList tips;
            tips.append(tr("The current profile does not support modification, a new profile will be created."));
            tips.append(tr("Please input a profile name."));
            QString fileName = QInputDialog::getText(this, tr("File name"), tips.join("\r\n"), QLineEdit::Normal, name);
            if(fileName.isEmpty()) {
                return false;
            }
            if(keytabsAll.contains(fileName)) {
                if(isPrivateKeytab(fileName)) {
                    QKxMessageBox::information(this, tr("File information"), tr("Modifying internal private files is not supported. Please input a new one."));
                    continue;
                }
                if(QKxMessageBox::information(this, tr("File information"), tr("Found a profile with the same name. Do you want to switch to that profile?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
                    return false;
                }
                ui->keytab->setCurrentText(fileName);
                reloadKeytabsLater(fileName);
                return true;
            }else{
                QString pathCustom = QWoSetting::customKeytabPath();
                pathCustom = pathCustom+"/"+fileName;
                if(!fileName.endsWith(".keytab")) {
                    pathCustom += ".keytab";
                }
                QString path = QDir::cleanPath(QKxUtils::keytabPath(name));
                if(!QFile::copy(path, pathCustom)) {
                    return false;
                }
                reloadKeytabsLater(fileName);
                return true;
            }
        }
    }
    return true;
}
