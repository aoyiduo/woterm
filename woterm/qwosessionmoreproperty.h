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

#ifndef QWOSESSIONMOREPROPERTY_H
#define QWOSESSIONMOREPROPERTY_H

#include <QDialog>
#include <QStandardItemModel>
#include <QPointer>

#include "qwoglobal.h"

namespace Ui {
class QWoSessionMoreProperty;
}

class QKxTermWidget;
class QKxTermItem;
class QWoShortCutModel;

class QWoSessionMoreProperty : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionMoreProperty(QWidget *parent = 0);
    ~QWoSessionMoreProperty();

    void setCustom(EHostType type, const QString& prop);
    QString result() const;
    QString language() const;
private slots:
    void onTimeout();
    void onTreeItemClicked(const QModelIndex& index);
    void onLangCurrentIndexChanged(const QString & txt);
    void onColorCurrentIndexChanged(const QString & txt);
    void onFontCurrentIndexChanged(const QString &family);
    void onFontValueChanged(int v);
    void onBlockCursorToggled();
    void onUnderlineCursorToggled();
    void onBeamCursorToggled();
    void onButtonSaveClicked();
    void onItemDoubleClicked(const QModelIndex &index);


private:
    void initDefault();
    void initShortCut();
    void setFixPreviewString();    
private:
    Ui::QWoSessionMoreProperty *ui;
    bool m_bCustom;
    QPointer<QKxTermWidget> m_preview;
    QPointer<QKxTermItem> m_term;
    QString m_result;
    QMap<QString, QString> m_langs;
    QString m_lang;
};

#endif // QWOSESSIONMOREPROPERTY_H
