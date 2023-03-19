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

    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside
    property string title: qsTr("New Directory")
    property string lable: qsTr("Please enter a new directory name.")

    signal result(string fileName, int permission)

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: ColumnLayout{
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
            wrapMode: Label.WrapAnywhere
        }

        TextField {
            id: input
            Layout.fillWidth: true
        }

        RowLayout {
            Item {
                Layout.fillWidth: true
            }

            CheckBox {
                id: chkSave
                Layout.fillWidth: true
                text: qsTr("Save password")
            }

            CheckBox {
                id: chkEcho
                Layout.fillWidth: true
                text: qsTr("Visible")
            }

            Item {
                Layout.fillWidth: true
            }
        }


        RowLayout {
            Item {
                Layout.leftMargin: 10
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("Ok")
                onClicked: {
                    if(input.text === "") {
                        gMessageBox.information(qsTr("Parameter error"), qsTr("The directory name can not be empty."));
                        return
                    }
                    thiz.result(input.text, permission.checked ? 0x1C0 : 0x1FF)
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
