import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page {
    id: thiz

    signal nextStep()
    signal finished()

    MoDBRestoreSftpInput {
        id: sftpInput
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width  - 20
        visible: false
        onVisibleChanged: {
            if(!visible) {
                server.text = thiz.sftpUrl()
            }
        }
    }

    function sftpUrl() {
        var dm = gSetting.value("DBBackup/sftpDetail");
        if(dm) {
            var url = "sftp://"+dm["name"]+"@"+dm["host"]+":"+dm["path"]+"?port="+dm["port"];
            return url
        }
        return "";
    }

    MoFileDialog {
        id: dlgFile
    }

    background: Rectangle{
        color: quick.themeColor
    }

    contentItem: Flickable {
        ColumnLayout{
            anchors.fill: parent
            anchors.margins: 5

            Component.onCompleted: {
                var lastFile = gSetting.value("DBBackup/lastLocalFile")
                if(lastFile) {
                    localFile.text = lastFile
                }
                var cryptType = gSetting.value("DBBackup/lastCryptType")
                if(cryptType) {
                    cryptInput.cryptType = cryptType
                }

                var cryptPass = gSetting.value("DBBackup/lastCryptPassword")
                if(cryptPass) {
                    cryptInput.cryptPwd = cryptPass
                }

                server.text = thiz.sftpUrl()
             }

            RowLayout{
                Layout.fillWidth: true
                Label {
                    text:qsTr("Type")
                }
                ComboBox {
                    id: type
                    Layout.fillWidth: true
                    currentIndex: 0
                    model: ListModel {
                        id: model
                        ListElement { text: qsTr("Sftp server") }
                        ListElement { text: qsTr("Local file") }
                    }
                    onCurrentTextChanged: {
                        if(currentIndex == 0) {
                            fileArea.visible = false
                            sftpArea.visible = true
                        }else {
                            fileArea.visible = true
                            sftpArea.visible = false
                        }
                    }
                }
            }
            ColumnLayout {
                id: fileArea
                visible: false
                Layout.fillWidth: true                
                RowLayout{
                    id: fileLayout
                    Layout.fillWidth: true
                    Label{
                        text:qsTr("Local file")
                    }
                    TextField {
                        id: localFile
                        readOnly: true
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.source: "qrc:/woterm/resource/skin/dirs.png"
                        icon.color: "transparent"
                        onClicked: {
                            var pathLast = gSetting.lastBackupPath()
                            dlgFile.nameFilters = ["SQLiteDB (*.db)"]
                            dlgFile.getOpenFileName(pathLast, qsTr("SQLiteDB selection"), qsTr("Please select the backup file.")).then(function(file){
                                localFile.text = file
                            })
                        }
                    }
                }
                RowLayout{
                    Layout.fillWidth: true
                    Item{
                        Layout.fillWidth: true
                    }
                    Button {
                        display: Button.TextBesideIcon
                        text: qsTr("Apply")
                        icon.source: "qrc:/woterm/resource/skin/right.png"
                        icon.color: "transparent"

                        onClicked: {
                            // do restore.
                            gSetting.setValue("DBBackup/lastLocalFile", localFile.text)
                            if(gSshConf.restoreByUrl(localFile.text)) {
                                gMessageBox.information(qsTr("Restore information"), qsTr("success to restore database."));
                                thiz.finish();
                            }else{
                                gMessageBox.warning(qsTr("Restore information"), qsTr("failed to restore database."));
                            }
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }
            }
            ColumnLayout {
                id: sftpArea
                visible: false
                Layout.fillWidth: true

                RowLayout{
                    Layout.fillWidth: true
                    Label{
                        text:qsTr("Server:")
                    }
                    TextField {
                        id: server
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.source: "qrc:/woterm/resource/skin/sftp.png"
                        icon.color: "transparent"
                        onClicked: {
                            sftpInput.open()
                        }
                    }
                }

                MoDBRestoreCryptInput {
                    id: cryptInput
                    Layout.fillWidth: true
                }

                RowLayout{
                    Layout.fillWidth: true
                    Item{
                        Layout.fillWidth: true
                    }
                    Button {
                        display: Button.TextBesideIcon
                        text: qsTr("Next")
                        icon.source: "qrc:/woterm/resource/skin/forward.png"
                        icon.color: "transparent"

                        onClicked: {
                            // do restore.
                            if(cryptInput.cryptPwd !== "") {
                                gSetting.setValue("DBBackup/lastCryptPassword", cryptInput.cryptPwd)
                            }
                            gSetting.setValue("DBBackup/lastCryptType", cryptInput.cryptType)
                            thiz.nextStep()
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }
            }

            Item{
                Layout.fillHeight: true
            }
        }
    }
}
