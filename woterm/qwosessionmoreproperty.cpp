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

#include "qwosessionmoreproperty.h"
#include "ui_qwosessionmoreproperty.h"
#include "qkxtermitem.h"
#include "qkxtermwidget.h"
#include "qwosshconf.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qkxutils.h"
#include "qwoshortcutmodel.h"
#include "qwoshortcutdelegate.h"

#include <QTimer>
#include <QDir>
#include <QStringListModel>
#include <QTranslator>
#include <QKeySequence>
#include <QVariant>
#include <QTextCodec>

QWoSessionMoreProperty::QWoSessionMoreProperty(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSessionMoreProperty),
    m_bCustom(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("SesstionProperty"));
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);

    {
        QStandardItemModel *model = new QStandardItemModel(this);
        QStandardItem *terminal = new QStandardItem(tr("Terminal"));
        QStandardItem *shortCut = new QStandardItem(tr("ShortCut"));
        terminal->appendRow(shortCut);
        model->appendRow(terminal);
        QStandardItem *appearance = new QStandardItem(tr("Rdp/Mstsc"));
        model->appendRow(appearance);
        QStandardItem *vnc = new QStandardItem(tr("Vnc"));
        model->appendRow(vnc);        
        QStandardItem *lang = new QStandardItem(tr("Language"));
        model->appendRow(lang);
        //QStandardItem *fileTransfre = new QStandardItem(tr("FileTransfer"));
        //model.appendRow(fileTransfre);
        ui->type->setModel(model);
        ui->type->expandAll();
        ui->type->setMaximumWidth(200);
    }

    m_preview = new QKxTermWidget(this);
    m_term = m_preview->termItem();
    m_term->setTermName("preview");
    ui->previewLayout->addWidget(m_preview);
    QObject::connect(ui->type, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onTreeItemClicked(const QModelIndex&)));
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(1000);
    setFixPreviewString();    

    QStringList rfbs;
    rfbs << "RFB_33" << "RFB_37" << "RFB_38";
    ui->proto->setModel(new QStringListModel(rfbs, this));
    ui->proto->setCurrentText("RFB_38");
    QStringList fmts;
    fmts << "RGB32_888" << "RGB16_565" << "RGB15_555" << "RGB8_332" << "RGB8_Map";
    ui->pixel->setModel(new QStringListModel(fmts, this));
    ui->pixel->setCurrentText("RGB16_565");

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

    QStringList families = QKxUtils::availableFontFamilies();
    ui->fontChooser->setModel(new QStringListModel(families, this));
    ui->fontChooser->setEditable(false);

    m_langs = QWoSetting::allLanguages();
    ui->langChooser->setModel(new QStringListModel(m_langs.keys(), this));
    QObject::connect(ui->langChooser, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLangCurrentIndexChanged(const QString&)));
    QObject::connect(ui->fontChooser, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onFontCurrentIndexChanged(const QString&)));
    QObject::connect(ui->fontSize, SIGNAL(valueChanged(int)), this, SLOT(onFontValueChanged(int)));
    QObject::connect(ui->blockCursor, SIGNAL(toggled(bool)), this, SLOT(onBlockCursorToggled()));
    QObject::connect(ui->underlineCursor, SIGNAL(toggled(bool)), this, SLOT(onUnderlineCursorToggled()));
    QObject::connect(ui->beamCursor, SIGNAL(toggled(bool)), this, SLOT(onBeamCursorToggled()));

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    ui->lineSize->setValidator(new QIntValidator(10, 65535));
    ui->fixWidth->setValidator(new QIntValidator(1024, 1920));
    ui->fixHeight->setValidator(new QIntValidator(768, 1080));
    ui->fixWidth->setText("1024");
    ui->fixHeight->setText("768");
    ui->fixRadio->setChecked(false);
    ui->deskRadio->setChecked(true);
    initDefault();
    initShortCut();
}

QWoSessionMoreProperty::~QWoSessionMoreProperty()
{
    delete ui;
}

void QWoSessionMoreProperty::setCustom(EHostType type, const QString &prop)
{
    m_bCustom = true;
    if(!prop.isEmpty()) {
        QVariantMap mdata = QWoUtils::qBase64ToVariant(prop).toMap();
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
            QString fontName = mdata.value("fontName", DEFAULT_FONT_FAMILY).toString();
            int fontSize = mdata.value("fontSize", DEFAULT_FONT_SIZE).toInt();
            ui->fontSize->setValue(fontSize);
            ui->fontChooser->setCurrentText(fontName);
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
        if(mdata.contains("desktopType")) {
            QString deskType = mdata.value("desktopType", "desktop").toString();
            if(deskType == "desktop") {
                ui->deskRadio->setChecked(true);
                ui->fixRadio->setChecked(false);
            }else{
                ui->deskRadio->setChecked(false);
                ui->fixRadio->setChecked(true);
                QString width = mdata.value("desktopWidth", "1024").toString();
                QString height = mdata.value("desktopHeight", "768").toString();
                ui->fixWidth->setText(width);
                ui->fixHeight->setText(height);
            }
        }
        if(mdata.contains("vncProto")) {
            {QString v = mdata.value("vncProto", "RFB_38").toString();
            if(v == "RFB_33" || v == "RFB_37" || v == "RFB_38") {
                ui->proto->setCurrentText(v);
            }else {
                ui->proto->setCurrentText("RFB_38");
            }}
            {QString v = mdata.value("vncPixel", "RGB16_565").toString();
            if(v == "RGB32_888" || v == "RGB16_565"
                    || v == "RGB15_555" || v == "RGB8_332"
                    || v == "RGB8_Map") {
                ui->pixel->setCurrentText(v);
            }else {
                ui->pixel->setCurrentText("RGB16_565");
            }}
            ui->h264->setChecked(mdata.value("vncH264", true).toBool());
            ui->jpeg->setChecked(mdata.value("vncJPEG", true).toBool());
            ui->zrle3->setChecked(mdata.value("vncZRLE3", true).toBool());
            ui->trle3->setChecked(mdata.value("vncTRLE3", true).toBool());
            ui->zrle2->setChecked(mdata.value("vncZRLE2", true).toBool());
            ui->trle2->setChecked(mdata.value("vncTRLE2", true).toBool());
            ui->zrle->setChecked(mdata.value("vncZRLE", true).toBool());
            ui->trle->setChecked(mdata.value("vncTRLE", true).toBool());
            ui->hextile->setChecked(mdata.value("vncHextile", true).toBool());
            ui->rre->setChecked(mdata.value("vncRRE", true).toBool());
            ui->copyrect->setChecked(mdata.value("vncCopyRect", true).toBool());
            ui->raw->setChecked(/*mdata.value("vncRaw", true).toBool()*/true);
            ui->deskresize->setChecked(mdata.value("vncDeskResize", true).toBool());
            ui->viewonly->setChecked(mdata.value("vncViewOnly", false).toBool());
        }
    }
    ui->type->hide();
    if(type < Mstsc) {
        ui->stacked->setCurrentWidget(ui->terminal);
    }else if(type < Vnc){
        ui->stacked->setCurrentWidget(ui->mstsc);
    }else {
        ui->stacked->setCurrentWidget(ui->vnc);
    }
}

QString QWoSessionMoreProperty::result() const
{
    return m_result;
}

QString QWoSessionMoreProperty::language() const
{
    return m_lang;
}

void QWoSessionMoreProperty::onTimeout()
{
    setFixPreviewString();
}

void QWoSessionMoreProperty::setFixPreviewString()
{
    m_term->clearAll();
    QByteArray seqTxt;
    seqTxt.append("\033[31mRed \033[32mGreen \033[33mYellow \033[34mBlue");
    seqTxt.append("\r\n\033[35mMagenta \033[36mCyan \033[37mWhite \033[39mDefault");
    seqTxt.append("\r\n\033[40mBlack \033[41mRed \033[42mGreen \033[43mYellow \033[44mBlue");
    seqTxt.append("\r\n\033[45mMagenta \033[46mCyan \033[47mWhite \033[49mDefault");
    m_term->parse(seqTxt);
}

void QWoSessionMoreProperty::onTreeItemClicked(const QModelIndex &idx)
{
    QString name = idx.data().toString();
    if(name == tr("Terminal")){
        ui->stacked->setCurrentWidget(ui->terminal);
    }else if(name == tr("Rdp/Mstsc")){
        ui->stacked->setCurrentWidget(ui->mstsc);
    }else if(name == tr("ShortCut")){
        ui->stacked->setCurrentWidget(ui->shortcut);
    }else if(name == tr("Language")){
        ui->stacked->setCurrentWidget(ui->lang);
    }else{
        ui->stacked->setCurrentWidget(ui->vnc);
    }
}

void QWoSessionMoreProperty::onColorCurrentIndexChanged(const QString &txt)
{
    m_term->setColorSchema(txt);
}

void QWoSessionMoreProperty::onLangCurrentIndexChanged(const QString &lang)
{
    QString path = m_langs.value(lang);
    if(path.isEmpty()) {
       return ;
    }
    m_lang = m_langs.value(lang);
}

void QWoSessionMoreProperty::onFontCurrentIndexChanged(const QString &family)
{
    QFont f = m_term->font();
    f.setFamily(family);
    m_term->setFont(f);
}

void QWoSessionMoreProperty::onFontValueChanged(int v)
{
    QFont f = m_term->font();
    f.setPointSize(v);
    m_term->setFont(f);
}

void QWoSessionMoreProperty::onBlockCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_Block);
}

void QWoSessionMoreProperty::onUnderlineCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_Underline);
}

void QWoSessionMoreProperty::onBeamCursorToggled()
{
    m_term->setCursorType(QKxTermItem::CT_IBeam);
}

void QWoSessionMoreProperty::onButtonSaveClicked()
{
    QVariantMap mvar;
    QWidget *w = ui->stacked->currentWidget();
    if(w == ui->terminal || !m_bCustom) {
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
    }

    if(w == ui->mstsc || !m_bCustom) {
        if(ui->fixRadio->isChecked()) {
            QString width = ui->fixWidth->text();
            QString height = ui->fixHeight->text();
            mvar.insert("desktopType", "fix");
            mvar.insert("desktopWidth", width);
            mvar.insert("desktopHeight", height);
        }else{
            mvar.insert("desktopType", "desktop");
        }
    }

    if(w == ui->vnc || !m_bCustom) {
        QString proto = ui->proto->currentText();
        mvar.insert("vncProto", proto);
        QString pixel = ui->pixel->currentText();
        mvar.insert("vncPixel", pixel);
        bool zrle = ui->zrle->isChecked();
        mvar.insert("vncZRLE", zrle);
        bool trle = ui->trle->isChecked();
        mvar.insert("vncTRLE", trle);
        bool zrle2 = ui->zrle2->isChecked();
        mvar.insert("vncZRLE2", zrle2);
        bool trle2 = ui->trle2->isChecked();
        mvar.insert("vncTRLE2", trle2);
        bool zrle3 = ui->zrle3->isChecked();
        mvar.insert("vncZRLE3", zrle3);
        bool trle3 = ui->trle3->isChecked();
        mvar.insert("vncTRLE3", trle3);
        bool h264 = ui->h264->isChecked();
        mvar.insert("vncH264", h264);
        bool jpeg = ui->jpeg->isChecked();
        mvar.insert("vncJPEG", jpeg);
        bool hextile = ui->hextile->isChecked();
        mvar.insert("vncHextile", hextile);
        bool copyrect = ui->copyrect->isChecked();
        mvar.insert("vncCopyRect", copyrect);
        bool rre = ui->rre->isChecked();
        mvar.insert("vncRRE", rre);
        bool raw = ui->rre->isChecked();
        mvar.insert("vncRaw", raw);
        bool deskresize = ui->deskresize->isChecked();
        mvar.insert("vncDeskResize", deskresize);
        bool viewonly = ui->viewonly->isChecked();
        mvar.insert("vncViewOnly", viewonly);
    }
    m_result = QWoUtils::qVariantToBase64(mvar);
    if(!m_bCustom) {
        QWoSetting::setValue("property/default", m_result);
        QWoShortCutModel *model = (QWoShortCutModel*)ui->keymap->model();
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
        QWoSetting::setValue("property/shortcut", QWoUtils::qVariantToBase64(mdata));
    }
    close();
}

void QWoSessionMoreProperty::onItemDoubleClicked(const QModelIndex &index)
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

void QWoSessionMoreProperty::initDefault()
{
    QString val = QWoSetting::value("property/default").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();

    QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
    ui->schema->setCurrentText(schema);
    m_term->setColorSchema(schema);

    QString keyboard = mdata.value("keyboard", DEFAULT_KEY_LAYOUT).toString();
    ui->kblayout->setCurrentText(keyboard);
    m_term->setKeyLayoutByName(keyboard);

    QString codec = mdata.value("textcodec", DEFAULT_TEXT_CODEC).toString();
    ui->codepage->setCurrentText(codec);
    m_term->setTextCodec(codec);

    QString fontName = mdata.value("fontName", DEFAULT_FONT_FAMILY).toString();
    int fontSize = mdata.value("fontSize", DEFAULT_FONT_SIZE).toInt();
    ui->fontSize->setValue(fontSize);
    ui->fontChooser->setCurrentText(fontName);

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

    QString deskType = mdata.value("desktopType", "desktop").toString();
    if(deskType == "desktop") {
        ui->deskRadio->setChecked(true);
        ui->fixRadio->setChecked(false);
    }else{
        ui->deskRadio->setChecked(false);
        ui->fixRadio->setChecked(true);
        QString width = mdata.value("desktopWidth", "1024").toString();
        QString height = mdata.value("desktopHeight", "768").toString();
        ui->fixWidth->setText(width);
        ui->fixHeight->setText(height);
    }

    {QString v = mdata.value("vncProto", "RFB_38").toString();
    if(v == "RFB_33" || v == "RFB_37" || v == "RFB_38") {
        ui->proto->setCurrentText(v);
    }else {
        ui->proto->setCurrentText("RFB_38");
    }}
    {QString v = mdata.value("vncPixel", "RGB16_565").toString();
    if(v == "RGB32_888" || v == "RGB16_565"
            || v == "RGB15_555" || v == "RGB8_332"
            || v == "RGB8_Map") {
        ui->pixel->setCurrentText(v);
    }else {
        ui->pixel->setCurrentText("RGB16_565");
    }}

    ui->h264->setChecked(mdata.value("vncH264", true).toBool());
    ui->jpeg->setChecked(mdata.value("vncJPEG", true).toBool());
    ui->zrle3->setChecked(mdata.value("vncZRLE3", true).toBool());
    ui->trle3->setChecked(mdata.value("vncTRLE3", true).toBool());
    ui->zrle2->setChecked(mdata.value("vncZRLE2", true).toBool());
    ui->trle2->setChecked(mdata.value("vncTRLE2", true).toBool());
    ui->zrle->setChecked(mdata.value("vncZRLE", true).toBool());
    ui->trle->setChecked(mdata.value("vncTRLE", true).toBool());
    ui->hextile->setChecked(mdata.value("vncHextile", true).toBool());
    ui->rre->setChecked(mdata.value("vncRRE", true).toBool());
    ui->copyrect->setChecked(mdata.value("vncCopyRect", true).toBool());
    ui->raw->setChecked(/*mdata.value("vncRaw", true).toBool()*/true);
    ui->deskresize->setChecked(mdata.value("vncDeskResize", true).toBool());
    ui->viewonly->setChecked(mdata.value("vncViewOnly", false).toBool());

    QString lang = QWoSetting::languageFile();
    if(m_langs.values().contains(lang)) {
        ui->langChooser->setCurrentText(QWoSetting::languageName(lang));
    }
}

void QWoSessionMoreProperty::initShortCut()
{
    QString val = QWoSetting::value("property/shortcut").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
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
