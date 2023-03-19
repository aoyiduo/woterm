import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Popup {
    id: thiz
    modal: true
    dim: true
    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside

    property alias title: title.text
    property alias text: shower.text

    background: Rectangle{
        radius: 8
        color: "white"
    }
    ColumnLayout{
        id: lolist
        anchors.fill: parent

        Label {
            Layout.fillWidth: true
            id: title
            font.pointSize: 15
            text: qsTr("Public key")
            horizontalAlignment: Qt.AlignHCenter
        }
        Frame
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            padding: 2
            contentItem: TextArea{
                id: shower
                selectByMouse: true
                wrapMode: TextEdit.WrapAnywhere
                padding: 0
            }
        }
        RowLayout{
            Layout.fillWidth: true
            Item{
                Layout.fillWidth: true
            }
            Button{
                Layout.fillWidth: true
                text: qsTr("Copy")
                onClicked: {
                    gApp.qmlCopyText(thiz.text)
                }
            }
            Button{
                Layout.fillWidth: true
                text: qsTr("Close")
                onClicked: {
                    thiz.close()
                }
            }
            Item{
                Layout.fillWidth: true
            }
        }
    }
}
