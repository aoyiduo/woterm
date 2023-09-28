﻿/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwosessionttyproperty.h"
#include "ui_qwosessionttyproperty.h"
#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qwoutils.h"
#include "qkxutils.h"
#include "qwosetting.h"
#include "qwoshortcutmodel.h"
#include "qwoshortcutdelegate.h"
#include "qkxmessagebox.h"
#include "qwofontlistmodel.h"
#include "qkxpositionitem.h"
#include "qkxbuttonassist.h"
#include "qwosshconf.h"

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
    QObject::connect(ui->schema, SIGNAL(currentIndexChanged(const QString &)),  this, SLOT(onColorCurrentIndexChanged(const QString &)));


    QStringList kbly = QKxUtils::availableKeyLayouts();
    ui->kblayout->setModel(new QStringListModel(kbly, this));

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

    QObject::connect(m_delegate, SIGNAL(removeArrived(QString)), this, SLOT(onFontFamilyRemove(QString)));
    QObject::connect(ui->fontChooser, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onFontCurrentIndexChanged(const QString&)));
    QObject::connect(ui->fontSize, SIGNAL(valueChanged(int)), this, SLOT(onFontValueChanged(int)));
    QObject::connect(ui->blockCursor, SIGNAL(toggled(bool)), this, SLOT(onBlockCursorToggled()));
    QObject::connect(ui->underlineCursor, SIGNAL(toggled(bool)), this, SLOT(onUnderlineCursorToggled()));
    QObject::connect(ui->beamCursor, SIGNAL(toggled(bool)), this, SLOT(onBeamCursorToggled()));

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnSaveToAll, SIGNAL(clicked()), this, SLOT(onButtonSaveToAllClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

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

    QObject::connect(ui->chkTileNone, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileH, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileV, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->chkTileAll, SIGNAL(clicked()), this, SLOT(onTileButtonClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onSelectButtonClicked()));

    QObject::connect(ui->bkImgAlpha, SIGNAL(valueChanged(int)), this, SLOT(onBkImageAlphaValueChanged(int)));

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

    if(mdata.contains("colorSchema")){
        QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
        ui->schema->setCurrentText(schema);
        m_term->setColorSchema(schema);
    }
    if(mdata.contains("keyboard")) {
        QString name = mdata.value("keyboard", DEFAULT_KEY_LAYOUT).toString();
        ui->kblayout->setCurrentText(name);
    }
    if(mdata.contains("textcodec")) {
        QString name = mdata.value("textcodec", DEFAULT_TEXT_CODEC).toString();
        ui->codepage->setCurrentText(name);
        m_term->setTextCodec(name);
    }
    if(mdata.contains("fontName")) {
        QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        QString fontName = mdata.value("fontName", ft.family()).toString();
        int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();
        ui->fontSize->setValue(fontSize);
        ui->fontChooser->setCurrentText(fontName);
        m_term->setTerminalFont(fontName, fontSize);
        refleshFontPreview();
    }
    if(mdata.contains("cursorType")) {
        QString cursorType = mdata.value("cursorType", "block").toString();
        if(cursorType.isEmpty() || cursorType == "block") {
            ui->blockCursor->setChecked(true);
        }else if(cursorType == "underline") {
            ui->underlineCursor->setChecked(true);
        }else {
            ui->beamCursor->setChecked(true);
        }
    }
    if(mdata.contains("historyLength")){
        QString line = mdata.value("historyLength", QString("%1").arg(DEFAULT_HISTORY_LINE_LENGTH)).toString();
        ui->lineSize->setText(line);
    }
    if(mdata.contains("dragPaste")) {
        bool checked = mdata.value("dragPaste", false).toBool();
        ui->chkDragCopyPaste->setChecked(checked);
    }
    if(mdata.contains("rkeyPaste")) {
        bool checked = mdata.value("rkeyPaste", false).toBool();
        ui->chkRKeyCopyPaste->setChecked(checked);
    }
    if(mdata.contains("keySequence")) {
        setShortCut(mdata.value("keySequence").toMap());
    }
    if(mdata.contains("shellPath")) {
        QString path = mdata.value("shellPath").toString();
        ui->shellPath->setText(path);
    }

}

QVariantMap QWoSessionTTYProperty::result() const
{
    return m_result;
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

void QWoSessionTTYProperty::onItemDoubleClicked(const QModelIndex &index)
{
    QAbstractItemModel *model = ui->keymap->model();
    if(index.column() == 1) {
        return;
    }
    int row = index.row();
    QModelIndex idx = model->index(row, 1, index.parent());
    QRect rt = ui->keymap->visualRect(idx);
    ui->keymap->edit(idx);
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

QVariantMap QWoSessionTTYProperty::save()
{
    QVariantMap mvar;

    QString schema = ui->schema->currentText();
    QString keyboard = ui->kblayout->currentText();
    QString textcodec = ui->codepage->currentText();
    QString fontName = ui->fontChooser->currentText();
    int fontSize = ui->fontSize->value();
    QString cursorType = ui->blockCursor->isChecked() ? "block" : (ui->beamCursor->isChecked() ? "beam": "underline");
    QString lineSize = ui->lineSize->text();
    mvar.insert("colorSchema", schema);
    mvar.insert("keyboard", keyboard);
    mvar.insert("textcodec", textcodec);
    mvar.insert("fontName", fontName);
    mvar.insert("fontSize", fontSize);
    mvar.insert("cursorType", cursorType);
    mvar.insert("historyLength", lineSize);
    mvar.insert("rkeyPaste", ui->chkRKeyCopyPaste->isChecked());
    mvar.insert("dragPaste", ui->chkDragCopyPaste->isChecked());

    QWoShortCutModel *model = qobject_cast<QWoShortCutModel*>(ui->keymap->model());
    QMap<int,QKeySequence> all = model->toMap();
    QVariantMap mdata;
    mdata.insert("SCK_Copy", all.value(QKxTermItem::SCK_Copy));
    mdata.insert("SCK_Paste", all.value(QKxTermItem::SCK_Paste));
    mdata.insert("SCK_SelectAll", all.value(QKxTermItem::SCK_SelectAll));
    mdata.insert("SCK_SelectLeft", all.value(QKxTermItem::SCK_SelectLeft));
    mdata.insert("SCK_SelectRight", all.value(QKxTermItem::SCK_SelectRight));
    mdata.insert("SCK_SelectUp", all.value(QKxTermItem::SCK_SelectUp));
    mdata.insert("SCK_SelectDown", all.value(QKxTermItem::SCK_SelectDown));
    mdata.insert("SCK_SelectHome", all.value(QKxTermItem::SCK_SelectHome));
    mdata.insert("SCK_SelectEnd", all.value(QKxTermItem::SCK_SelectEnd));

    mvar.insert("keySequence", mdata);

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

    QString keyboard = mdata.value("keyboard", DEFAULT_KEY_LAYOUT).toString();
    ui->kblayout->setCurrentText(keyboard);
    m_term->setKeyLayoutByName(keyboard);

    QString codec = mdata.value("textcodec", DEFAULT_TEXT_CODEC).toString();
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
        bool checked = mdata.value("rkeyPaste", false).toBool();
        ui->chkRKeyCopyPaste->setChecked(checked);
    }

    if(m_ttyType == ETTY_LocalShell){
        QString shellPath = mdata.value("shellPath", QWoUtils::findShellPath()).toString();
        ui->shellPath->setText(shellPath);
    }

    setShortCut(mdata.value("keySequence").toMap());
}

void QWoSessionTTYProperty::setShortCut(const QVariantMap &mdata)
{
    QWoShortCutModel *model = new QWoShortCutModel(ui->keymap, this);
    ui->keymap->setModel(model);
    model->append(tr("Edit"));
    model->append(QKxTermItem::SCK_Copy, tr("Copy"), mdata.value("SCK_Copy", m_term->defaultShortCutKey(QKxTermItem::SCK_Copy)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_Paste, tr("Paste"), mdata.value("SCK_Paste", m_term->defaultShortCutKey(QKxTermItem::SCK_Paste)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectAll, tr("SelectAll"), mdata.value("SCK_SelectAll", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectAll)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectLeft, tr("Select text from right to left"), mdata.value("SCK_SelectLeft", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectLeft)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectRight, tr("Select text from left to right"), mdata.value("SCK_SelectRight", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectRight)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectUp, tr("Select text from bottom to up"), mdata.value("SCK_SelectUp", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectUp)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectDown, tr("Select text from top to bottom"), mdata.value("SCK_SelectDown", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectDown)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectHome, tr("Select text from from line start to end"), mdata.value("SCK_SelectHome", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectHome)).value<QKeySequence>());
    model->append(QKxTermItem::SCK_SelectEnd, tr("Select text from current to line end"), mdata.value("SCK_SelectEnd", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectEnd)).value<QKeySequence>());
    ui->keymap->setColumnWidth(0, model->widthColumn(ui->keymap->font(), 0));
    ui->keymap->setItemDelegate(new QWoKeySequenceDelegate(this));
    QObject::connect(ui->keymap, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
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
    QString path = QWoSetting::fontBackupPath();
    QString fileNew = path + "/" + fi.fileName();
    if(!f.copy(fileNew)) {
        QKxMessageBox::warning(this, tr("Import errors"), tr("Failed to backup the font file.")+QString("[%1,%2]").arg(fileName).arg(f.errorString()));
        return false;
    }
    return true;
}
