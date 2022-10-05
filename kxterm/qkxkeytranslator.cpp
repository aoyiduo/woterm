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

#include "qkxkeytranslator.h"

#include <QFile>
#include <QDebug>


static QRegExp rgxTitle("keyboard\\s+\"(.*)\"");
static QRegExp rgxKey("key\\s+([\\w\\+\\s\\-\\*\\.]+)\\s*:\\s*(\"(.*)\"|\\w+)");

QKxKeyTranslator::QKxKeyTranslator(QObject *parent)
    : QObject(parent)
{

}

QString QKxKeyTranslator::path() const
{
    return m_path;
}

QString QKxKeyTranslator::name() const
{
    return m_title;
}

void QKxKeyTranslator::setName(const QString &name)
{
    m_title = name;
}

bool QKxKeyTranslator::load(const QString &file)
{
    QFile f(file);
    if(!f.open(QFile::ReadOnly|QFile::Text)) {
        return false;
    }

    while(!f.atEnd()){
        QString line = f.readLine().simplified();
        if(line.isEmpty()) {
            continue;
        }
        if(rgxTitle.exactMatch(line)) {
            m_title = rgxTitle.capturedTexts().at(1);
        } else if(rgxKey.exactMatch(line)) {
            QStringList lsz = rgxKey.capturedTexts();
            if(!lsz.last().isEmpty()) {
                parseOutput(lsz.at(1).simplified(), lsz.at(3).simplified());
            }
        }
    }
    return !m_keys.isEmpty();
}

QByteArray QKxKeyTranslator::match(int key, Qt::KeyboardModifiers modifiers, VTModes states)
{
    //skip keypad effect.
    modifiers &= ~Qt::KeypadModifier;
    modifiers &= ~Qt::MetaModifier;
    QList<KeyInfo> kis = m_keys.value(key);
    if(kis.isEmpty()) {
        return QByteArray();
    }
    QMap<int, KeyInfo> hit;
    for(int i = 0; i < kis.length(); i++) {
        KeyInfo ki = kis.at(i);
        if((ki.mode & states) == ki.mode) {
            if(modifierEqual(ki.modify, modifiers)) {
                hit.insert(ki.modify, ki);
            }
        }
    }
    if(hit.isEmpty()) {
        return QByteArray();
    }
    return hit.last().out;
}

bool QKxKeyTranslator::parseKey(const QString &key, KeyInfo &ki)
{
    QStringList tokens = tokenArray(key);
    qDebug() << key << tokens;
    QString code = tokens.takeFirst();
    QKeySequence ks = QKeySequence::fromString(code);
    ki.key = ks[0];
    ModifyKeys modify = 0;
    VTModes mode = 0;
    while(!tokens.isEmpty()) {
        QString t1 = tokens.takeFirst().toLower();
        bool wanted = t1.at(0) == '+';
        QString t = t1.mid(1);
        if(t == "shift") {
            modify |= wanted ? MK_SHIFT_YES : MK_SHIFT_NO;
        }else if(t == "alt") {
            modify |= wanted ? MK_ALT_YES : MK_ALT_NO;
        }else if(t == "ctrl" || t == "control") {
            modify |= wanted ? MK_CTRL_YES : MK_CTRL_NO;
        }else if(t == "meta") {
            modify |= wanted ? MK_META_YES : MK_META_NO;
        }else if(t == "anymod" || t == "anymodifier") {
            modify |= wanted ? MK_ANY_YES : MK_ANY_NO;
        }else if(t == "keypad") {
            modify |= wanted ? MK_KEYPAD_YES : MK_KEYPAD_NO;
        }else if(t == "appcukeys" || t == "appcursorkeys") {
            mode |= wanted ? VM_APPCUKEY_YES : VM_APPCUKEY_NO;
        }else if(t == "ansi") {
            mode |= wanted ? VM_ANSI_YES : VM_ANSI_NO;
        }else if(t == "newline") {
            mode |= wanted ? VM_NEWLINE_YES : VM_NEWLINE_NO;
        }else if(t == "appscreen") {
            mode |= wanted ? VM_APPSCREEN_YES : VM_APPSCREEN_NO;
        }else if(t == "appkeypad") {
            mode |= wanted ? VM_APPKEYPAD_YES : VM_APPKEYPAD_NO;
        }
    }
    ki.raw = key.toUtf8();
    ki.modify = modify;
    ki.mode = mode;
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

void QKxKeyTranslator::parseOutput(const QString &key, const QString &output)
{
    //qDebug() << "output" << key << output;
    KeyInfo ki;
    if(!parseKey(key, ki)) {
        return;
    }
    QByteArray out;
    QByteArray left = output.toLatin1();
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

    ki.out = out;
    QList<KeyInfo> lki = m_keys.value(ki.key);
    lki.append(ki);
    m_keys.insert(ki.key, lki);
}

bool QKxKeyTranslator::modifierEqual(ModifyKeys mk, Qt::KeyboardModifiers modifier)
{
    ModifyKeys flags;
    int anyModify = Qt::ControlModifier|Qt::ShiftModifier|Qt::AltModifier;
    flags |= modifier & Qt::ControlModifier ? MK_CTRL_YES : MK_CTRL_NO;
    flags |= modifier & Qt::ShiftModifier ? MK_SHIFT_YES : MK_SHIFT_NO;
    flags |= modifier & Qt::AltModifier ? MK_ALT_YES : MK_ALT_NO;
    flags |= modifier & anyModify ? MK_ANY_YES : MK_ANY_NO;

    return (mk & flags) == mk;
}
