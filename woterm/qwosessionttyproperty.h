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

#ifndef QWOSESSIONTTYPROPERTY_H
#define QWOSESSIONTTYPROPERTY_H

#include <QDialog>
#include <QPointer>
#include <QVariantMap>
#include <QStyledItemDelegate>
#include <QEvent>

namespace Ui {
class QWoSessionTTYProperty;
}

class QTabBar;
class QKxTermWidget;
class QKxTermItem;
class QTimer;
class QComboBox;
class QToolButton;


class QFontCleanDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit QFontCleanDelegate(QWidget *parent = 0);
    bool editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const;

signals:
    void removeArrived(const QString& family) const;
private:
    bool _editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);
    QRect buttonRect(const QRect& rt) const;

private:
    QPointer<QWidget> m_parent;
    QToolButton *m_btnRemove;
    QStringList m_customs;
    QPoint m_ptMouse;
    QEvent::Type m_event;
};

class QWoSessionTTYProperty : public QDialog
{
    Q_OBJECT
public:
    enum ETTYType {
        ETTY_Default,
        ETTY_RemoteTarget,
        ETTY_LocalShell,
        ETTY_SerialPort
    };
public:
    explicit QWoSessionTTYProperty(ETTYType type, QWidget *parent = nullptr);
    ~QWoSessionTTYProperty();
    void setCustom(const QVariantMap& prop);
    QVariantMap result() const;
private slots:
    void onPageCurrentChanged(int idx);
    void onColorCurrentIndexChanged(const QString & txt);
    void onFontCurrentIndexChanged(const QString& family);
    void onStyleCurrentIndexChanged(const QString& style);
    void onFontValueChanged(int v);
    void onBlockCursorToggled();
    void onUnderlineCursorToggled();
    void onBeamCursorToggled();
    void onTimeout();
    void onButtonSaveClicked();
    void onShellPathButtonClicked();
    void onItemDoubleClicked(const QModelIndex &index);
    void onButtonImportClicked();
    void onFontFamilyRemove(const QString& family);


private:
    void initDefault();
    void setShortCut(const QVariantMap& dm);
    void setFixPreviewString();
    void refleshFontPreview();
    bool importFont(const QStringList& families, const QString& fileName);
private:
    Ui::QWoSessionTTYProperty *ui;
    bool m_bCustom;
    ETTYType m_ttyType;
    QVariantMap m_result;
    QPointer<QTabBar> m_tabBar;
    QPointer<QKxTermWidget> m_preview;
    QPointer<QKxTermItem> m_term;
    QPointer<QFontCleanDelegate> m_delegate;
};

#endif // QWOSESSIONTTYPROPERTY_H
