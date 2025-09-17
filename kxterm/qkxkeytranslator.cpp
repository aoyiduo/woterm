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

#include "qkxkeytranslator.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>


QKxKeyTranslator::QKxKeyTranslator(QObject *parent)
    : QObject(parent)
{

}

QString QKxKeyTranslator::name() const
{
    return m_name;
}

bool QKxKeyTranslator::load(const QString &file)
{
    m_name.clear();
    m_keys.clear();

    QList<KeyInfo> all;
    if(!load(file, all)) {
        return false;
    }
    for(auto it = all.begin(); it != all.end(); it++) {
        const KeyInfo& ki = *it;

        QList<KeyInfo> lski = m_keys.take(ki.key);
        lski.insert(ki.key, ki);
        m_keys.insert(ki.key, lski);
    }
    QFileInfo fi(file);
    m_name = fi.fileName();
    return !m_keys.isEmpty();
}

bool QKxKeyTranslator::match(int key, Qt::KeyboardModifiers modifiers, VTModes modes, EOperation &op, QByteArray &out)
{
    //skip keypad effect.
    //modifiers &= ~Qt::KeypadModifier;
    //modifiers &= ~Qt::MetaModifier;
    QList<KeyInfo> kis = m_keys.value(key);
    if(kis.isEmpty()) {
        return false;
    }
    QMap<int, KeyInfo> hit;
    for(int i = 0; i < kis.length(); i++) {
        KeyInfo ki = kis.at(i);
        if((ki.modes & modes) == ki.modes) {
            if(modifierMatch(ki.modifies, modifiers)) {
                if(!ki.out.isEmpty() || !hit.contains(ki.modifies)){
                    // make sure the ansi control sequence has more proirity.
                    hit.insert(ki.modifies, ki);
                }
            }
        }
    }
    if(hit.isEmpty()) {
        return false;
    }
    KeyInfo& ki = hit.last();
    op = ki.op;
    out = ki.out;
    return true;
}

QKeySequence QKxKeyTranslator::shortcut(EOperation op, VTModes modes)
{
    for(auto it = m_keys.begin(); it != m_keys.end(); it++) {
        QList<KeyInfo> kis = m_keys.value(it.key());
        if(kis.isEmpty()) {
            return false;
        }
        for(int i = 0; i < kis.length(); i++) {
            KeyInfo ki = kis.at(i);
            if((ki.modes & modes) == ki.modes) {
                if(ki.op == op) {
                    int key = ki.key;
                    if(ki.modifies & MK_CTRL_YES) {
                        key += Qt::CTRL;
                    }
                    if(ki.modifies & MK_SHIFT_YES) {
                        key += Qt::SHIFT;
                    }
                    if(ki.modifies & MK_ALT_YES) {
                        key += Qt::ALT;
                    }
                    if(ki.modifies & MK_META_YES) {
                        key += Qt::META;
                    }
                    return QKeySequence(key);
                }
            }
        }
    }
    return QKeySequence();
}

QString QKxKeyTranslator::operationToName(EOperation op)
{
    switch (op) {
    case ECopy:
        return "copy";
    case EPaste:
        return "paste";
    case ESelectLineUp:
        return "selectLineUp";
    case ESelectLineDown:
        return "selectLineDown";
    case ESelectLineLeft:
        return "selectLineLeft";
    case ESelectLineRight:
        return "selectLineRight";
    case ESelectLineHome:
        return "selectLineHome";
    case ESelectLineEnd:
        return "selectLineEnd";
    case ESelectAll:
        return "selectAll";
    case EScrollLineUp:
        return "scrollLineUp";
    case EScrollPageUp:
        return "scrollPageUp";
    case EScrollUpToTop:
        return "scrollUpToTop";
    case EScrollLineDown:
        return "scrollLineDown";
    case EScrollPageDown:
        return "scrollPageDown";
    case EScrollDownToBottom:
        return "scrollDownToBottom";
    case EFind:
        return "find";
    default:
        return "";
    }
}

QKxKeyTranslator::EOperation QKxKeyTranslator::operationFromName(const QString &code)
{
    if(code == "scrollLineUp") {
        return EScrollLineUp;
    }else if(code == "scrollPageUp") {
        return EScrollPageUp;
    }else if(code == "scrollUpToTop") {
        return EScrollUpToTop;
    }else if(code == "scrollLineDown") {
        return EScrollLineDown;
    }else if(code == "scrollPageDown") {
        return EScrollPageDown;
    }else if(code == "scrollDownToBottom") {
        return EScrollDownToBottom;
    }else if(code == "selectLineUp") {
        return ESelectLineUp;
    }else if(code == "selectLineDown") {
        return ESelectLineDown;
    }else if(code == "selectLineLeft") {
        return ESelectLineLeft;
    }else if(code == "selectLineRight") {
        return ESelectLineRight;
    }else if(code == "selectLineHome") {
        return ESelectLineHome;
    }else if(code == "selectLineEnd") {
        return ESelectLineEnd;
    }else if(code == "selectAll") {
        return ESelectAll;
    }else if(code == "copy") {
        return ECopy;
    }else if(code == "paste") {
        return EPaste;
    }else if(code == "find") {
        return EFind;
    }
    return ENotDefined;
}

QByteArray QKxKeyTranslator::stringToAnsiSequence(const QString &code)
{
    QByteArray out;
    QByteArray left = code.toLatin1();
    while(left.length() > 0) {
        char ch = left.at(0);
        if(ch != '\\') {
            out.append(ch);
            left.remove(0, 1);
            continue;
        }
        ch = left.at(1);
        switch (ch) {
        case 'E':
            out.append(27);
            left.remove(0, 2);
            break;
        case 'b':
            out.append(8);
            left.remove(0, 2);
            break;
        case 'f':
            out.append(12);
            left.remove(0, 2);
            break;
        case 't':
            out.append(9);
            left.remove(0, 2);
            break;
        case 'r':
            out.append(13);
            left.remove(0, 2);
            break;
        case 'n':
            out.append(10);
            left.remove(0, 2);
            break;
        case 'x':
            left.remove(0, 2);
            QByteArray hex = QByteArray::fromHex(left.left(2));
            if(hex.length() > 0) {
                out.append(hex.at(0));
            }
            left.remove(0, 2);
            break;
        }
    }
    return out;
}

bool QKxKeyTranslator::load(const QString &file, QList<KeyInfo> &all)
{
    QFile f(file);
    if(!f.open(QFile::ReadOnly|QFile::Text)) {
        return false;
    }

    //static const QRegularExpression rgxTitle(R"(keyboard\s+\"(.*)\")");
    static const QRegularExpression rgxKey(R"(key\s+(.+?)\s*:\s*(\"(.*)\"|\w+))");

    while(!f.atEnd()){
        QString line = f.readLine();
        if(line.isEmpty()) {
            continue;
        }
        line = removeComment(line);
        QRegularExpressionMatch key = rgxKey.match(line);
        if(key.hasMatch()) {
            QString k = key.captured(1);
            QString w = key.captured(2);
            QString v = key.captured(3);
            //QStringView vt = key.capturedView(3);
            //qDebug() << "QStringView" << vt;
            if(v.isEmpty()) {
                // shortcut map to
                parseOperation(k, w, all);
            }else{
                // key map to
                parseString(k, v, all);
            }
        }
    }
    return !all.isEmpty();
}

bool QKxKeyTranslator::save(const QString &filePath, const QString &fileTemplate, const QList<KeyInfo> &keys)
{
    QFile fread(fileTemplate);
    if(!fread.open(QFile::ReadOnly)) {
        return false;
    }
    int cLength = maxMaxConditionLength(keys);
    QByteArray tempSave = fread.readAll();
    QByteArrayList lineAll;
    for(auto it = keys.begin(); it != keys.end(); it++) {
        const QKxKeyTranslator::KeyInfo& ki = *it;
        QByteArray action = ki.out.isEmpty() ? ki.action.toUtf8() : ("\"" + ki.action.toUtf8() + "\"");
        QByteArray condition = ki.condition.toUtf8();
        condition = condition.append(cLength+20, QChar::Space);
        condition.resize(cLength+10);
        QByteArray line = "key " + condition + " : " + action;
        lineAll.append(line);
    }
    fread.close();

    QFile fsave(filePath);
    if(!fsave.open(QFile::WriteOnly)) {
        qInfo() << "save failure:" << fsave.errorString();
        return false;
    }
    QFileInfo fi(fsave);
    QByteArray content = lineAll.join("\r\n");
    tempSave = tempSave.replace("{{name}}", fi.baseName().toUtf8());
    QByteArray keysAll = tempSave.replace("{{key}}", content);
    fsave.write(keysAll);
    return true;
}

int QKxKeyTranslator::maxMaxConditionLength(const QList<KeyInfo> &keys)
{
    int count = 0;
    for(auto it = keys.begin(); it != keys.end(); it++) {
        const QKxKeyTranslator::KeyInfo& ki = *it;
        if(ki.condition.length() > count) {
            count = ki.condition.length();
        }
    }
    return count;
}

QMap<QKxKeyTranslator::EOperation, QString> QKxKeyTranslator::operationDescriptions()
{
    static QMap<QKxKeyTranslator::EOperation, QString> descs;
    if(descs.isEmpty()) {
        descs.insert(ECopy, tr("Copy"));
        descs.insert(EPaste, tr("Paste"));
        descs.insert(EFind, tr("Find"));
        descs.insert(ESelectLineUp, tr("Select to the previous row"));
        descs.insert(ESelectLineDown, tr("Select to the next row"));
        descs.insert(ESelectLineLeft, tr("Select to the left of the row"));
        descs.insert(ESelectLineRight, tr("Select to the right of the row"));
        descs.insert(ESelectLineHome, tr("Select to the beginning of row"));
        descs.insert(ESelectLineEnd, tr("Select to the end of row"));
        descs.insert(ESelectAll, tr("Select all"));
        descs.insert(EScrollLineUp, tr("Scroll up one row"));
        descs.insert(EScrollPageUp, tr("Scroll up one page"));
        descs.insert(EScrollUpToTop, tr("Scroll to the top"));
        descs.insert(EScrollLineDown, tr("Scroll down one row"));
        descs.insert(EScrollPageDown, tr("Scroll down one page"));
        descs.insert(EScrollDownToBottom, tr("Scroll down to bottom"));
    }
    return descs;
}

QString QKxKeyTranslator::removeComment(const QString &_line)
{
    QString line = _line;
    bool inQuotes = false;
    int pos = -1;
    for (int i = line.length() - 1; i >= 0; i--) {
        QChar ch = line[i];
        if (ch == '\"') {
            inQuotes = !inQuotes;
        } else if (ch == '#' && !inQuotes) {
            pos = i;
        }
    }

    if (pos != -1) {
        line = line.left(pos);
    }
    line = line.simplified();
    return line;
}

void QKxKeyTranslator::parseOperation(const QString &key, const QString &code, QList<KeyInfo>& all)
{
    KeyInfo ki;
    if(!parseKey(key, ki)) {
        return;
    }
    ki.op = operationFromName(code);
    if(ki.op == ENotDefined) {
        return;
    }
    ki.action = code;
    all.append(ki);
}

void QKxKeyTranslator::parseString(const QString &key, const QString &code, QList<KeyInfo>& all)
{
    KeyInfo ki;
    if(!parseKey(key, ki)) {
        return;
    }
    QByteArray out = stringToAnsiSequence(code);
    if(out.isEmpty()) {
        return;
    }
    ki.action = code;
    ki.out = out;
    all.append(ki);
}

bool QKxKeyTranslator::parseKey(const QString &key, KeyInfo &ki)
{
    QStringList tokens = tokenArray(key);
    //qDebug() << key << tokens;
    QString code = tokens.takeFirst();
    QKeySequence ks = QKeySequence::fromString(code);
    ki.key = ks[0];
    ModifyKeys modifies = 0;
    VTModes modes = 0;
    while(!tokens.isEmpty()) {
        QString t1 = tokens.takeFirst().toLower();
        bool wanted = t1.at(0) == '+';
        QString t = t1.mid(1);
        if(t == "shift") {
            modifies |= wanted ? MK_SHIFT_YES : MK_SHIFT_NO;
        }else if(t == "alt" || t == "option") {
            modifies |= wanted ? MK_ALT_YES : MK_ALT_NO;
        }else if(t == "ctrl" || t == "control") {
            modifies |= wanted ? MK_CTRL_YES : MK_CTRL_NO;
        }else if(t == "meta" || t == "command") {
            modifies |= wanted ? MK_META_YES : MK_META_NO;
        }else if(t == "anymod" || t == "anymodifier") {
            modifies |= wanted ? MK_ANY_YES : MK_ANY_NO;
        }else if(t == "keypad") {
            modifies |= wanted ? MK_KEYPAD_YES : MK_KEYPAD_NO;
        }else if(t == "appcukeys" || t == "appcursorkeys") {
            modes |= wanted ? VM_APPCUKEY_YES : VM_APPCUKEY_NO;
        }else if(t == "ansi") {
            modes |= wanted ? VM_ANSI_YES : VM_ANSI_NO;
        }else if(t == "newline") {
            modes |= wanted ? VM_NEWLINE_YES : VM_NEWLINE_NO;
        }else if(t == "appscreen") {
            modes |= wanted ? VM_APPSCREEN_YES : VM_APPSCREEN_NO;
        }else if(t == "appkeypad") {
            modes |= wanted ? VM_APPKEYPAD_YES : VM_APPKEYPAD_NO;
        }
    }
    ki.condition = key;
    ki.modifies = modifies;
    ki.modes = modes;
    return true;
}

QStringList QKxKeyTranslator::tokenArray(const QString &keys)
{
    QStringList all;
    QString part;
    for(int i = 0; i < keys.length(); i++) {
        QChar ch = keys.at(i);
        if(ch == QChar::Space || ch == '-' || ch == '+') {
            part = part.simplified();
            if(!part.isEmpty()) {
                all.append(part);
            }
            part.clear();
        }
        part.append(ch);
    }
    if(!part.isEmpty()) {
        all.append(part);
    }
    return all;
}

bool QKxKeyTranslator::modifierMatch(ModifyKeys mk, Qt::KeyboardModifiers modifier)
{
    ModifyKeys flags;
    int anyModify = Qt::ControlModifier|Qt::ShiftModifier|Qt::AltModifier|Qt::MetaModifier;
    flags |= modifier & Qt::ControlModifier ? MK_CTRL_YES : MK_CTRL_NO;
    flags |= modifier & Qt::ShiftModifier ? MK_SHIFT_YES : MK_SHIFT_NO;
    flags |= modifier & Qt::AltModifier ? MK_ALT_YES : MK_ALT_NO;
    flags |= modifier & Qt::MetaModifier ? MK_META_YES : MK_META_NO;
    flags |= modifier & anyModify ? MK_ANY_YES : MK_ANY_NO;

    return (mk & flags) == mk;
}
