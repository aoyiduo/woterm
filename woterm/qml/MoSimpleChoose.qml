import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Popup {
    id: chooser
    dim: true
    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside

    property alias title: title.text
    signal result(string name)

    function updateList(all) {
        proxyModel.clear()
        for(var id in all){
            proxyModel.append({name: all[id]})
        }
    }

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
            text: qsTr("JumpList")
            horizontalAlignment: Qt.AlignHCenter
        }
        Frame
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            padding: 2
            ListView {
                id: view
                anchors.fill: parent
                clip: true
                model: ListModel{
                    id: proxyModel
                }
                Component {
                    id: delegate
                    Rectangle{
                        width: view.width
                        height: 40
                        color: view.currentIndex === index ? "#D4237A" : "white"
                        Text {
                            id: label
                            anchors.fill: parent
                            text: name
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        MouseArea{
                            z: 10
                            anchors.fill: parent
                            onClicked: {
                                if (view.currentIndex != index) {
                                    view.currentIndex = index;
                                }
                            }
                        }
                    }
                }
                delegate: delegate
                ScrollIndicator.vertical: ScrollIndicator { }
            }
        }
        RowLayout{
            Layout.fillWidth: true
            Item{
                Layout.fillWidth: true
            }
            Button{
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: {
                    chooser.close()
                }
            }
            Button{
                Layout.fillWidth: true
                text: qsTr("Select")
                onClicked: {
                    if(view.currentIndex < 0) {
                        return
                    }
                    var obj = proxyModel.get(view.currentIndex)
                    result(obj.name)
                    chooser.close()
                }
            }
            Item{
                Layout.fillWidth: true
            }
        }
    }
}
