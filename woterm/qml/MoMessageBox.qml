import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Popup {
    id: thiz


    property string title
    property string text
    property bool okOnly: true
    property alias okText: btnOk.text
    property alias cancelText: btnCancel.text

    signal result(int v)

    modal: true
    focus: true

    x:0
    y:0
    width:parent.width
    height:parent.height

    background: Rectangle{
        color: "#00000000"
    }

    Rectangle {
        id: panel
        anchors.centerIn: parent
        width: column.width + 20
        height: column.implicitHeight + 20
        radius: 10
        border.color: "white"

        color: "#23262B"

        ColumnLayout{
            id: column
            width: 320
            anchors.centerIn: parent
            spacing: 10

            Label{
                text: title
                Layout.fillWidth: true
                horizontalAlignment: Qt.AlignHCenter
                color: "#39CADB"
                font.pointSize: 15
                font.bold: true
            }

            Label{
                text: thiz.text
                color: "#39CADB"
                font.pointSize: 15
                wrapMode: Label.WrapAnywhere
                Layout.topMargin: 20
                Layout.bottomMargin: 20
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            RowLayout{
                Layout.preferredWidth: 100
                Layout.preferredHeight: 35
                Item{
                    Layout.fillWidth: true
                }

                Button {
                    id: btnOk
                    text: "确认"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 35
                    hoverEnabled: true
                    contentItem: Label{
                        text: btnOk.text
                        anchors.fill: parent
                        color: "white"
                        font.pointSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        border.color: parent.hovered ? "white" : "#39CADB"
                        border.width: parent.hovered ? 2 : 0
                        color: parent.pressed ? "#39C0AB" : "#39CADB"
                    }
                    onClicked: {
                        thiz.result(1)
                        thiz.close()
                    }
                }

                Button {
                    id: btnCancel
                    text: "取消"
                    visible: !okOnly
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 35
                    hoverEnabled: true
                    contentItem: Label{
                        text: btnCancel.text
                        anchors.fill: parent
                        color: "white"
                        font.pointSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        border.color: parent.hovered ? "white" : "#39CADB"
                        border.width: parent.hovered ? 2 : 0
                        color: parent.pressed ? "#39C0AB" : "#39CADB"
                    }
                    onClicked: {
                        thiz.result(0)
                        thiz.close()
                    }
                }
                Item{
                    Layout.fillWidth: true
                }
            }

            Item{
                Layout.fillHeight: true
            }
        }
    }
}
