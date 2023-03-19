import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoDBSftpDownSync 1.0

Page {
    id: thiz

    property string constDB_UNCRYPT_NAME: "woterm_restore_decryption.db"
    property alias cryptType: cryptInput.cryptType
    property alias cryptPwd: cryptInput.cryptPwd


    signal previousStep()
    signal closeArrived()

    Component.onCompleted: {
        var cryptType = gSetting.value("DBBackup/lastCryptType")
        if(cryptType) {
            thiz.cryptType = cryptType
        }

        var cryptPass = gSetting.value("DBBackup/lastCryptPassword")
        if(cryptPass) {
            thiz.cryptPwd = cryptPass
        }
        sftp.listArrived.connect(onlineList.onSftpListAll)
        sftp.infoArrived.connect(thiz.onInfoArrived)
        sftp.fetchFinished.connect(thiz.onFetchFinished)
        sftp.fetchLatest()
    }

    function decrypt(fileName) {
        var retVal = sftp.qmlDecryptFile(fileName, constDB_UNCRYPT_NAME, cryptType, cryptPwd);
        if(typeof(retVal) === "string") {
            gMessageBox.information(qsTr("Decryption error"), retVal.toString());
            decryptAgain.visible = true
            btnApply.enabled = false
            return false
        }
        decryptAgain.visible = false
        btnApply.enabled = true
        return true
    }

    function onFetchFinished(err, fileName) {
        if(err === 0){
            fileRemote.text = fileName
            thiz.decrypt(fileName)
        }
    }

    function onInfoArrived(action, err, desc) {
        console.log("infoArrived", action, err, desc)
        tip.text = desc
    }

    MoDBRestoreSftpInput {
        id: sftpInput
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width  - 20
    }

    MoDBRestoreOnlineList {
        id: onlineList
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width  - 20
        onSelectArrived: {
            sftp.fetch(fileName)
        }

        onVisibleChanged: {
            if(visible) {
                sftp.listAll()
            }
        }
    }

    MoDBSftpDownSync{
        id: sftp
    }

    background: Rectangle{
        color: quick.themeColor
    }

    contentItem: Flickable {
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 5
            RowLayout{
                Layout.fillWidth: true
                Label{
                    text:qsTr("Remote file:")
                }
                TextField {
                    id: fileRemote
                    Layout.fillWidth: true
                    readOnly: true
                }
                ToolButton {
                    icon.source: "qrc:/woterm/resource/skin/download2.png"
                    icon.color: "transparent"
                    onClicked: {
                        onlineList.open()
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                id: tip
                color: "red"
            }

            ColumnLayout {
                id: decryptAgain
                visible: false
                Layout.fillWidth: true
                MoDBRestoreCryptInput {
                    id: cryptInput
                    Layout.fillWidth: true
                }
                Button {
                    text: qsTr("Decrypt again")
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: thiz.decrypt(fileRemote.text)
                }
            }

            RowLayout{
                Layout.fillWidth: true
                Item{
                    Layout.fillWidth: true
                }
                Button {
                    display: Button.TextBesideIcon
                    text: qsTr("Previous")
                    icon.source: "qrc:/woterm/resource/skin/back.png"
                    icon.color: "transparent"

                    onClicked: {
                        thiz.previousStep()
                    }
                }
                Button {
                    id: btnApply
                    enabled: false
                    display: Button.TextBesideIcon
                    text: qsTr("Apply")
                    icon.source: "qrc:/woterm/resource/skin/right.png"
                    icon.color: "transparent"

                    onClicked: {
                        // do restore.
                        var path = sftp.qmlAbsolutePath(constDB_UNCRYPT_NAME);
                        if(path === false) {
                            gMessageBox.information(qsTr("Restore information"), qsTr("The file has been lost."))
                            return;
                        }
                        if(gSshConf.restoreByUrl(path)) {
                            gMessageBox.information(qsTr("Restore information"), qsTr("success to restore database."));
                            thiz.closeArrived();
                        }else{
                            gMessageBox.warning(qsTr("Restore information"), qsTr("failed to restore database."));
                        }
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

        ScrollIndicator.vertical: ScrollIndicator { }
    }
}
