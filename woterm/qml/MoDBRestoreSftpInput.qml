import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoDbSftpDetail 1.0

Popup {
    id: thiz
    modal: true
    dim: true
    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside
    property var identifyNames: gIdentify.qmlFileNames()

    Component.onCompleted: {
        var dm = gSetting.value("DBBackup/sftpDetail");
        if(dm) {
            var url = "sftp://"+dm["name"]+"@"+dm["host"]+":"+dm["path"]+"?port="+dm["port"];
            console.log("sftp url:"+url)
            host.text = dm.host
            name.text = dm.name
            port.text = dm.port
            type.currentIndex = dm.type
            password.text = dm.password
            identify.currentIndex = identifyNames.indexOf(dm.identify)
            path.text = dm.path
        }
    }

    function dataGet() {
        if(host.text === "") {
            gMessageBox.information(qsTr("Parameter error"), qsTr("The host parameter can not be empty."));
            return
        }
        if(name.text === "") {
            gMessageBox.information(qsTr("Parameter error"), qsTr("The login name parameter can not be empty."));
            return
        }
        if(path.text === "") {
            gMessageBox.information(qsTr("Parameter error"), qsTr("The path parameter can not be empty."));
            return
        }

        var loginPassword = ""
        var loginIdentify = ""
        if(type.currentIndex == 0) {
            // password
            loginPassword = password.text
            if(loginPassword === "") {
                gMessageBox.information(qsTr("Parameter error"), qsTr("The login password parameter can not be empty."));
                return
            }
        }else{
            loginIdentify = identify.currentText
            if(loginIdentify === "") {
                gMessageBox.information(qsTr("Parameter error"), qsTr("The login identify parameter can not be empty."));
                return
            }
        }
        var dm = {
            host: host.text,
            name: name.text,
            port: port.text,
            type: type.currentIndex,
            password: loginPassword,
            identify: loginIdentify,
            path: path.text
        }
        return dm;
    }

    MoDbSftpDetail {
        id: sftp
    }

    background: Rectangle{
        radius: 3
        color: quick.themeColor
    }
    contentItem: ColumnLayout{
        RowLayout{
            Label{
                text: qsTr("Host name:")
            }
            TextField {
                id: host
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Label{
                text: qsTr("Port:")
            }
            TextField {
                id: port
                Layout.fillWidth: true
                text: "22"
                validator: IntValidator{bottom: 1; top: 65534}
            }
        }

        RowLayout {
            Label{
                text: qsTr("Name:")
            }
            TextField {
                id: name
                Layout.fillWidth: true
                text: "root"
            }
        }

        RowLayout {
            Label {
                text:qsTr("Type:")
            }
            ComboBox {
                id: type
                Layout.fillWidth: true
                model: [qsTr("Password"), qsTr("Identity file")]
                onCurrentTextChanged: {
                    if(currentIndex == 0) {
                        pwdArea.visible = true
                        identityArea.visible = false
                    }else {
                        pwdArea.visible = false
                        identityArea.visible = true
                    }
                }
            }
        }
        RowLayout {
            id: pwdArea
            Label{
                text: qsTr("Password:")
            }
            TextField {
                id: password
                Layout.fillWidth: true
                echoMode: TextField.PasswordEchoOnEdit
            }
        }

        RowLayout {
            id: identityArea
            Label{
                text: qsTr("Identity file:")
            }
            ComboBox {
                id: identify
                Layout.fillWidth: true
                model: identifyNames
                onCurrentTextChanged: {
                }
            }
        }

        RowLayout {
            Label{
                text: qsTr("Path:")
            }
            TextField {
                id: path
                Layout.fillWidth: true
                text: "~/woterm_db_backup"
            }
        }

        RowLayout{
            Layout.fillWidth: true

            Item{
                Layout.fillWidth: true
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Verify")
                onClicked: {
                    sftp.verify(thiz.dataGet());
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Save")
                onClicked: {
                    gSetting.setValue("DBBackup/sftpDetail", thiz.dataGet());
                    thiz.close()
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: {
                    thiz.close()
                }
            }
            Item{
                Layout.fillWidth: true
            }
        }
    }
}
