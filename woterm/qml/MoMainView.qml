import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page{
    id: thiz
    property StackView stack

    Component.onCompleted:{
        gMainWindow.readyUpgradeVersion.connect(thiz.onReadyUpgradeVersion)
    }

    function onReadyUpgradeVersion() {
        dlgAbout.open()
    }

    background: Rectangle{
        color: quick.themeColor
    }    

    MoSftpConfigure{
        id: dlgCfg
    }

    MoAboutView {
        id: dlgAbout
    }

    MoMainMenu {
        id: drawer
        onAboutArrived: {
            dlgAbout.open()
        }
        onLanguageArrived: {
            dlgLang.open()
        }
    }

    MoLanguageView {
        id: dlgLang
    }

    MoInputDialog {
        id: dlg
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/menu.png"
        title: qsTr("WoTerm")
        onLeftClicked: {
            drawer.open()
        }
    }

    Component {
        id: restore
        MoDBRestoreView {
            onCloseArrived: {
                stack.pop();
            }
        }
    }

    Component {
        id: sessions
        MoSessionManageView {
            stack: thiz.stack
            onCloseArrived: {
                stack.pop();
            }
        }
    }

    Component {
        id: identify
        MoIdentifyView {
            onCloseArrived: {
                stack.pop()
            }
        }
    }

    MoFileDialog {
        id: dlgFile
    }

    contentItem: ColumnLayout {
        Flow {
            Layout.fillWidth: true

            spacing: 5
            leftPadding: 5
            rightPadding: 5
            topPadding: 5

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Remote Sessions")
                icon.source: "qrc:/woterm/resource/skin/nodes.png"
                icon.color: "transparent"
                onClicked: {
                    stack.push(sessions)
                }
            }

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Local terminal")
                icon.source: "qrc:/woterm/resource/skin/console.png"
                icon.color: "transparent"

                onClicked: {
                    gMainWindow.openLocalShell();
                }
            }

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Identity")
                icon.source: "qrc:/woterm/resource/skin/keyset.png"
                icon.color: "transparent"
                onClicked: {
                    gstack.push(identify)
                }
            }

            Button {
                display: Button.TextUnderIcon
                text: qsTr("Test")
                visible: quick.debugVersion
                icon.source: "qrc:/woterm/resource/skin/keyset.png"
                icon.color: "transparent"
                property int cnt: 0
                onClicked: {

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
            text: qsTr("Recent access history")
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: historyEmpty
                visible: gRecentHistory.length === 0
                anchors.fill: parent
                anchors.bottomMargin: 100
                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    Image {
                        smooth: true
                        source: "qrc:/woterm/resource/skin/sad3.png"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("No access history")
                        font.italic: true
                        font.pointSize: 12
                    }
                }
            }

            ListView {
                anchors.fill: parent
                id:listView
                visible: gRecentHistory.length > 0

                clip:true
                currentIndex:-1

                model: gRecentHistory

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

                                Text {
                                    Layout.preferredHeight: 12
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 10
                                    text: model.shortString
                                    font.pixelSize: 10
                                    verticalAlignment: Qt.AlignVCenter
                                }
                                Item{
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
                                    source: "qrc:/woterm/resource/skin/connect.png"
                                }
                                onClicked: {
                                    gMainWindow.openRecent(model.name, model.type)
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
}

