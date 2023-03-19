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

    signal selectArrived(string fileName)

    function onSftpListAll(files) {
        for(var id in files) {
            backFiles.append({name: files[id]})
        }
    }

    background: Rectangle{
        radius: 3
        color: quick.themeColor
    }
    contentItem: ColumnLayout{
        RowLayout{
            Layout.fillWidth: true
            Label{
                text: qsTr("Host name:")
            }
        }
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 300

            id:listView

            clip:true
            currentIndex:-1

            model: ListModel {
                id: backFiles
            }

            Component {
                id: itemDelegate
                Rectangle {
                    width: listView.width
                    height: 55
                    color: listView.currentIndex == index ? quick.highlightColor : quick.backgroundColor
                    Rectangle{
                        x:0
                        y:0
                        width:parent.width
                        height:1
                        color:quick.seperatorColor
                        visible: index === 0 ? true : false
                    }
                    ColumnLayout{
                        anchors.fill: parent
                        Item{
                            Layout.fillHeight: true
                        }

                        Text {
                            Layout.preferredHeight: 18
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            text: model.name
                            font.pixelSize: 15
                            verticalAlignment: Qt.AlignVCenter
                        }

                        Item{
                            Layout.fillHeight: true
                        }
                    }
                    Rectangle{
                        x:0
                        y:parent.height - 1
                        width:parent.width
                        height:1
                        color:quick.seperatorColor
                    }
                    MouseArea{
                        id:maMouse
                        anchors.fill: parent
                        onClicked: {
                            if (listView.currentIndex != index) {
                                listView.currentIndex = index;
                            }
                        }
                    }
                }
            }

            delegate: itemDelegate
            ScrollIndicator.vertical: ScrollIndicator { }
        }
        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("Select")
                onClicked: {
                    var my = backFiles.get(listView.currentIndex)
                    thiz.selectArrived(my.name)
                    thiz.close()
                }
            }

            Button {
                text: qsTr("Close")
                onClicked: thiz.close()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
