import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoAbout 1.0

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

    onVisibleChanged: {
        if(visible) {
            upgradeArea.visible = false
            about.checkLatestVersion()
        }
    }
    onClosed: {
        about.stopUpgrade()
    }

    MoAbout {
        id: about
        onDownloadProgress: {
            console.log("download progress", bytesReceived, bytesTotal)
            upgradeBar.from = 0
            upgradeBar.to = bytesTotal
            upgradeBar.value = bytesReceived
        }

        onUpgradeMessageArrived: {
            upgradeMsg.text = msg
        }

        onFinished: {
            console.log("finish.")
        }

        onInstallArrived: {
            var ret = gSystem.installAPK(path)
            upgradeMsg.text = "result:"+ret
        }
    }

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: Row {
        ColumnLayout{
            width: parent.width
            spacing: 5
            Label {
                Layout.fillWidth: true
                text: qsTr("About...")
                font.bold: true
                horizontalAlignment: Label.AlignHCenter
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: quick.seperatorColor
            }
            RowLayout{
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.bottomMargin: 10
                Label {
                    text: qsTr("Thank you for your use!")
                }
            }
            RowLayout{
                Layout.fillWidth: true
                Label {
                    text: qsTr("Current version:")
                }
                Label{
                    Layout.fillWidth: true
                    padding: 5
                    wrapMode: Label.WrapAnywhere
                    verticalAlignment: Label.AlignVCenter
                    Component.onCompleted: {
                        text = about.currentVersion
                    }
                }
            }
            RowLayout{
                Layout.fillWidth: true
                Label {
                    text: qsTr("Latest version:")
                }
                Label{
                    Layout.fillWidth: true
                    padding: 5
                    text: about.latestVersion
                    wrapMode: Label.WrapAnywhere
                    verticalAlignment: Label.AlignVCenter
                }
            }

            ColumnLayout {
                id: upgradeArea
                Layout.fillWidth: true
                visible: false
                Label {
                    Layout.fillWidth: true
                    id: upgradeMsg
                    elide: Label.ElideMiddle
                }
                ProgressBar {
                    Layout.fillWidth: true
                    id: upgradeBar
                }
            }

            Label{
                Layout.fillWidth: true
                visible: upgradeArea.visible
                padding: 5
                wrapMode: Label.WrapAnywhere
                color: "red"
                text: qsTr("If the installation process cannot be popped up normally, please open the website through a browser, download and install it manually.")
            }

            RowLayout {
                Item {
                    Layout.leftMargin: 10
                    Layout.fillWidth: true
                }
                Button {
                    text: qsTr("Upgrade version")
                    enabled: about.latestVersion !== about.currentVersion
                    onClicked: {
                        about.upgradeVersion()
                        upgradeArea.visible = true
                    }
                }
                Button {
                    text: qsTr("Close")
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
