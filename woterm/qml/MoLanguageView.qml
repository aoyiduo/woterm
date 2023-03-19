import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Popup {
    id: thiz
    dim: true
    modal: true
    visible: false
    padding: 10

    x: (quick.width - width) / 2
    y: (quick.height - height) / 2
    width: quick.width * 4 / 5

    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside
    property string title: qsTr("Language option")
    property string lable: qsTr("After modifying the language, you need to restart the application to take effect.")

    onVisibleChanged: {
        var pathUsed = gSetting.languageFile()
        var name = gSetting.languageName(pathUsed)
        var names = gSetting.allLanguageNames()
        lang.currentIndex = -1
        lang.currentIndex = names.indexOf(name)
    }

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: Row {
        ColumnLayout{
            width: parent.width
            spacing: 5
            Label{
                Layout.fillWidth: true
                padding: 5
                text: title
                clip: true
                font.bold: true
                wrapMode: Label.WrapAnywhere
                horizontalAlignment: Label.AlignHCenter
                verticalAlignment: Label.AlignVCenter
            }
            Label{
                Layout.fillWidth: true
                padding: 5
                text: lable
                clip: true
                color: "red"
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            }

            ComboBox {
                id: lang
                Layout.fillWidth: true
                model: gSetting.allLanguageNames()
            }

            RowLayout {
                Item {
                    Layout.leftMargin: 10
                    Layout.fillWidth: true
                }
                Button {
                    text: qsTr("Apply")
                    enabled: {
                        var name = lang.currentText
                        var path = gSetting.languagePath(name)
                        var pathUsed = gSetting.languageFile()
                        return pathUsed !== path
                    }

                    onClicked: {
                        var name = lang.currentText
                        var path = gSetting.languagePath(name)
                        var pathUsed = gSetting.languageFile()
                        console.log("language name:"+name+"  path:"+path+"   pathUsed:"+pathUsed)
                        if(path !== pathUsed) {
                            gSetting.setLanguageFile(path)
                        }else if(path === ""){
                            gMessageBox.information(qsTr("Parameter error"), qsTr("the language file lost."))
                        }

                        thiz.close()
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: {
                        thiz.close()
                    }
                }
                Item {
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                }
            }
        }
    }
}
