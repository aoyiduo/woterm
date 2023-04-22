import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page {
    id: thiz
    property StackView stack

    signal closeArrived()

    Component{
        id: cProperty
        MoSessionProperty {
            onFinished: {
                stack.pop();
            }
        }
    }

    background: Rectangle{
        color: quick.themeColor
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/left2.png"
        title: qsTr("Session manage")
        onLeftClicked: {
            closeArrived()
        }
    }

    contentItem: ColumnLayout {
        spacing: 0
        Flow {
            Layout.fillWidth: true
            spacing: 5
            leftPadding: 5
            rightPadding: 5
            topPadding: 5

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Add")
                icon.source: "qrc:/woterm/resource/skin/add2.png"
                icon.color: "transparent"
                onClicked: {
                    stack.push(cProperty)
                }
            }

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Clear")
                icon.source: "qrc:/woterm/resource/skin/remove.png"
                icon.color: "transparent"
                onClicked: {
                    gSshConf.qmlClear()
                }
            }


            Button {
                display: Button.TextUnderIcon
                text: qsTr("Restore")
                icon.source: "qrc:/woterm/resource/skin/import2.png"
                icon.color: "transparent"

                onClicked: {
                    stack.push(restore);
                }
            }
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: quick.seperatorColor
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("Quick search")
        }
        TextField {
            Layout.fillWidth: true
            selectByMouse: true
            mouseSelectionMode: TextInput.SelectWords

            placeholderText: qsTr("Multiple keywords separated by spaces")
            onTextChanged: {
                gHostListModel.search(text)
            }
        }

        ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                id: listView
                model: gHostListModel
                Component {
                    id: delegate
                    Rectangle {
                        width: listView.width
                        height: loItem.implicitHeight
                        color: listView.currentIndex === index ? quick.highlightColor : quick.backgroundColor
                        ColumnLayout{
                            id: loItem
                            z: 100
                            anchors.fill: parent
                            Rectangle{
                                Layout.preferredHeight: 1
                                Layout.fillWidth: true
                                color:"lightGray"
                                opacity: index === 0 ? 1 : 0
                            }
                            RowLayout{
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Layout.leftMargin: 5
                                Layout.rightMargin: 5
                                Image {
                                    sourceSize.width: 32
                                    sourceSize.height: 32
                                    smooth: true
                                    verticalAlignment: Image.AlignVCenter
                                    fillMode: Image.PreserveAspectFit
                                    source: model.iconUrl
                                }
                                ColumnLayout{
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Item{
                                        Layout.fillHeight: true
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        Layout.leftMargin: 10
                                        font.family: "Courier New"
                                        text: model.name
                                        font.pixelSize: 20
                                        verticalAlignment: Qt.AlignVCenter
                                    }
                                    Text{
                                        Layout.fillWidth: true
                                        Layout.leftMargin: 10
                                        font.family: "Courier New"
                                        text: model.shortString
                                        font.pixelSize: 12
                                        verticalAlignment: Qt.LeftToRight
                                    }
                                    Item{
                                        Layout.fillHeight: true
                                    }
                                }
                                RowLayout{
                                    Layout.fillHeight: true
                                    spacing: 10
                                    visible: listView.currentIndex === index
                                    Button{
                                        flat: true
                                        background: Image{
                                            sourceSize.width: 32
                                            sourceSize.height: 32
                                            source: "qrc:/woterm/resource/skin/connect.png"
                                        }
                                        onClicked: {                                            
                                            if(model.type === "SshWithSftp") {
                                                sshftp.popup()
                                            }else if(model.type === "SftpOnly") {
                                                gMainWindow.openSftp(model.name)
                                            }else if(model.type === "Telnet") {
                                                gMainWindow.openTelnet(model.name)
                                            }else if(model.type === "RLogin") {
                                                gMainWindow.openRLogin(model.name)
                                            }else if(model.type === "Mstsc") {
                                                gMessageBox.information(qsTr("Telnet information"), qsTr("The current version is not yet supported."))
                                            }else if(model.type === "Vnc") {
                                                gMainWindow.openVnc(model.name)
                                            }
                                        }

                                        Menu {
                                            id: sshftp
                                            modal: true
                                            MenuItem {
                                                text: qsTr("SSH Terminal")
                                                onClicked: {
                                                    gMainWindow.openSsh(model.name)
                                                }
                                            }
                                            MenuItem {
                                                text: qsTr("SFTP Manage")
                                                onClicked: {
                                                    gMainWindow.openSftp(model.name)
                                                }
                                            }
                                        }
                                    }
                                    Button{
                                        flat: true
                                        background: Image{
                                            sourceSize.width: 32
                                            sourceSize.height: 32
                                            source: "qrc:/woterm/resource/skin/edit.png"
                                        }
                                        onClicked: {
                                            console.log("clicked now....")
                                            let name = model.name;
                                            stack.push(cProperty, {hostName:name})
                                        }
                                    }
                                    Button{
                                        flat: true
                                        background: Image{
                                            sourceSize.width: 32
                                            sourceSize.height: 32
                                            source: "qrc:/woterm/resource/skin/remove2.png"
                                        }
                                        onClicked: {
                                            let retVal = gMessageBox.information(qsTr("Remove confirmation"), qsTr("Continue to remove related sessions?"), MessageDialog.Yes|MessageDialog.No)
                                            if(retVal === MessageDialog.Yes) {
                                                gHostListModel.qmlRemove(model.name)
                                            }
                                        }
                                    }
                                }
                            }
                            Rectangle{
                                Layout.preferredHeight: 1
                                Layout.fillWidth: true
                                color:"lightGray"
                            }
                        }
                        MouseArea{
                            z: 1
                            anchors.fill: parent
                            onClicked: {
                                if (listView.currentIndex != index) {
                                    listView.currentIndex = index;
                                }
                            }
                        }
                    }
                }
                delegate: delegate
                ScrollIndicator.vertical: ScrollIndicator { }
            }
    }
}
