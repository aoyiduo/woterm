import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoDir 1.0

Popup {
    id: thiz
    dim: true
    modal: true
    visible: false
    padding: 10

    x: 10
    y: (quick.height - height) / 2
    width: quick.width - 20

    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside

    Component.onCompleted: {
        gSftpTransfer.commandStart.connect(thiz.onCommandStart)
        gSftpTransfer.commandFinish.connect(thiz.onCommandFinished)
        gSftpTransfer.onHideArrived.connect(thiz.onHideArrived)
        gSftpTransfer.onLabelArrived.connect(thiz.onLabelArrived)
        gSftpTransfer.progress.connect(thiz.onTransferProgress)
        gSftpTransfer.init(thiz);
    }

    onClosed: {
        gSftpTransfer.resetAll()
    }

    function onCommandStart(type, userData) {

    }

    function onCommandFinished(type, userData) {
        console.log("onCommandFinished", type, userData)
    }

    function onTransferProgress(type, v, userData) {
        progressBar.value = v;
    }

    function onHideArrived() {
        console.log("hideArrived")
        thiz.close()
    }

    function onLabelArrived(local, remote) {
        console.log(local, remote)
        pathLocal.text = local
        pathRemote.text = remote
    }

    MoDir {
        id: dir
    }

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: ColumnLayout{
        width: parent.width
        onHeightChanged: {
            console.log("contentHeight2", height)
        }

        spacing: 5
        Label{
            Layout.fillWidth: true
            padding: 5
            text: qsTr("File transfer")
            clip: true
            font.bold: true
            wrapMode: Label.WrapAnywhere
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Label{
                padding: 5
                clip: true
                text: qsTr("Local:")
            }
            Label{
                id: pathLocal
                Layout.fillWidth: true
                padding: 5
                clip: true
                elide: Label.ElideLeft
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label{
                padding: 5
                clip: true
                text: qsTr("Remote:")
            }
            Label{
                id: pathRemote
                Layout.fillWidth: true
                padding: 5
                clip: true
                elide: Label.ElideLeft
            }
        }

        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            from: 0
            to: 100
        }

        RowLayout {
            Item {
                Layout.leftMargin: 10
                Layout.fillWidth: true
            }
            Button {
                id: btnStart
                objectName: "btnStart"
                visible: false
                text: qsTr("Start")
                onClicked: {
                    btnStop.visible = true
                    visible = false
                }

                Component.onCompleted: {
                    clicked.connect(gSftpTransfer.onTransferStart)
                }
            }
            Button {
                id: btnStop
                text: qsTr("Stop")
                objectName: "btnStop"
                onClicked: {
                    btnStart.visible = true
                    visible = false
                }

                Component.onCompleted: {
                    clicked.connect(gSftpTransfer.onTransferStop)
                }
            }
            Button {
                text: qsTr("Abort")
                objectName: "btnAbort"
                Component.onCompleted: {
                    clicked.connect(gSftpTransfer.onTransferAbort)
                }
            }
            Button {
                text: qsTr("Abort all")
                objectName: "btnAbortAll"
                Component.onCompleted: {
                    clicked.connect(gSftpTransfer.onTransferAbortAll)
                }
            }
            Item {
                Layout.rightMargin: 10
                Layout.fillWidth: true
            }
        }
        ListView {
            id: listView
            Layout.fillWidth: true
            model: gSftpTransferModel
            visible: true
            clip: true
            Layout.preferredHeight: {
                var count = gSftpTransferModel.length;
                if(count > 5) {
                    count = 5;
                }
                return count * 55;
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
                    RowLayout{
                        anchors.fill: parent
                        Image {
                            Layout.leftMargin: 5
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            sourceSize.width: 32
                            sourceSize.height: 32
                            source: model.iconUrl
                        }

                        ColumnLayout{
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Item {
                                Layout.fillHeight: true
                            }
                            RowLayout{
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Text {
                                    Layout.fillHeight: true
                                    text: qsTr("Local:")
                                    font.pointSize: 10
                                    verticalAlignment: Qt.AlignVCenter
                                }
                                Text {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    text: model.local
                                    font.pointSize: 10
                                    elide: Label.ElideLeft
                                    verticalAlignment: Qt.AlignVCenter
                                }
                            }

                            RowLayout{
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Label {
                                    Layout.fillHeight: true
                                    text: qsTr("Remote:")
                                    font.pointSize: 10
                                    verticalAlignment: Qt.AlignVCenter
                                }
                                Label {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    text: model.remote
                                    font.pointSize: 10
                                    elide: Label.ElideLeft
                                    verticalAlignment: Qt.AlignVCenter
                                }
                            }
                            Item {
                                Layout.fillHeight: true
                            }
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
                    RowLayout{
                        anchors.fill: parent
                        spacing: 10
                        visible: listView.currentIndex == index
                        Item{
                            Layout.fillWidth: true
                        }

                        Button{
                            flat: true
                            background: Image{
                                sourceSize.width: 32
                                sourceSize.height: 32
                                source: "qrc:/woterm/resource/skin/remove.png"
                            }
                            onClicked: {
                                gSftpTransferModel.removeTask(listView.currentIndex)
                            }
                        }

                        Item{
                            Layout.preferredWidth: 20
                        }
                    }
                }
            }

            delegate: itemDelegate
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
}
