import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

ToolBar {
    property alias source: left.source
    property alias title: title.text
    signal leftClicked()
    RowLayout {
        anchors.fill: parent
        Button {
            id: tbtn
            Layout.leftMargin: 8
            Layout.fillHeight: true
            background: Image{
                id: left
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }
            onClicked: {
                leftClicked()
            }
        }
        Label {
            id: title
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }
        Item{
            Layout.preferredWidth: tbtn.width
            Layout.rightMargin: 8
        }
    }
}

