import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    signal deployArrived(string hosts, string fileLocal, string fileRemote, string enDesc, string zhDesc)

    FileDialog {
        id: dlgFile
        selectMultiple: false
        title: "Please choose a file"
        folder: shortcuts.home

        onAccepted: {
            console.log("You chose: " + dlgFile.fileUrl)
            fileLocal.text = Playbook.urlToLocalFile(dlgFile.fileUrl)
        }
        onRejected: {
            console.log("Canceled")
        }
    }

    ColumnLayout{
        Layout.maximumWidth: 600
        Layout.minimumWidth: 600
        ColumnLayout {
            Layout.fillWidth: true
            Label{
                text: "Target hosts:"
                color: Playbook.textColor
                font.pointSize: 12
            }

            TextField{
                id: hosts
                Layout.fillWidth: true
                color: Playbook.textColor
                font.pointSize: 12
                selectByMouse: true
                readOnly: true
                text: Playbook.storageValue("hosts")
                placeholderText: "The target hosts that needs to be deployed"
                mouseSelectionMode: TextInput.SelectWords
                background: Rectangle {
                    border.color: Playbook.textColor
                    color: "transparent"
                }


                Button {
                    x: hostsChoose.x - width - 2
                    y: (parent.height - height) / 2
                    height: parent.height - 4
                    icon.source: "../private/skins/black/remove.png"
                    text: "Clear"

                    onClicked: {
                        hosts.clear()
                    }
                }
                Button {
                    id: hostsChoose
                    x: parent.width - width - 2
                    y: (parent.height - height) / 2
                    height: parent.height - 4
                    icon.source: "../private/skins/black/ssh2.png"
                    text: "Choose hosts"

                    onClicked: {
                        let name = Playbook.hostChooser()
                        if(hosts.text.length == 0) {
                            hosts.text = name
                        }else{
                            hosts.text = hosts.text + "," + name
                        }
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true

            Label{
                text: "Local file:"
                color: Playbook.textColor
                font.pointSize: 12
            }

            TextField{
                id: fileLocal
                Layout.fillWidth: true
                color: Playbook.textColor
                font.pointSize: 12
                selectByMouse: true
                readOnly: true
                text: Playbook.storageValue("fileLocal")
                placeholderText: "The file path that needs to be deployed"
                mouseSelectionMode: TextInput.SelectWords
                background: Rectangle {
                    border.color: Playbook.textColor
                    color: "transparent"
                }

                Button {
                    x: fileChoose.x - width - 2
                    y: (parent.height - height) / 2
                    height: parent.height - 4
                    icon.source: "../private/skins/black/remove.png"
                    text: "Clear"

                    onClicked: {
                        fileLocal.clear()
                    }
                }

                Button {
                    id: fileChoose
                    x: parent.width - width - 2
                    y: (parent.height - height) / 2
                    height: parent.height - 4
                    icon.source: "../private/skins/black/dirs.png"
                    text: "Choose file"

                    onClicked: {
                        dlgFile.open()
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true

            Label{
                text: "Remote path:"
                color: Playbook.textColor
                font.pointSize: 12
            }

            TextField{
                id: fileRemote
                Layout.fillWidth: true
                color: Playbook.textColor
                font.pointSize: 12
                selectByMouse: true
                text: Playbook.storageValue("fileRemote")
                placeholderText: "The remote path that needs to be deployed"
                mouseSelectionMode: TextInput.SelectWords
                background: Rectangle {
                    border.color: Playbook.textColor
                    color: "transparent"
                }

                Button {
                    x: parent.width - width - 2
                    y: (parent.height - height) / 2
                    height: parent.height - 4
                    icon.source: "../private/skins/black/remove.png"
                    text: "Clear"

                    onClicked: {
                        fileRemote.clear()
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            clip: true
            ColumnLayout {
                Label {
                    color: Playbook.textColor
                    text: "Chinese description"
                }
                MyTextEdit {
                    id: zhDesc
                    text: Playbook.storageValue("zhDesc")
                    Layout.preferredHeight: 200
                    Layout.preferredWidth: 500
                }
            }
            ColumnLayout {
                Label {
                    color: Playbook.textColor
                    text: "English description"
                }
                MyTextEdit {
                    id: enDesc
                    text: Playbook.storageValue("enDesc")
                    Layout.preferredHeight: 200
                    Layout.preferredWidth: 500
                }
            }
        }

        RowLayout {
            Button{
                text: "Next step"
                onClicked: {
                    if(hosts.text.length == 0) {
                        Playbook.information("Host information", "The parameter of target hosts should not be empty.");
                        return
                    }
                    if(fileLocal.text.length == 0) {
                        Playbook.information("File information", "The parameter of Local file should not be empty.");
                        return
                    }
                    if(fileRemote.text.length == 0) {
                        Playbook.information("File information", "The parameter of remote path should not be empty.");
                        return
                    }
                    if(zhDesc.text.length == 0) {
                        Playbook.information("Chinese description", "The parameter of Chinese description should not be empty.");
                        return
                    }
                    if(enDesc.text.length == 0) {
                        Playbook.information("English description", "The parameter of English description should not be empty.");
                        return
                    }
                    Playbook.setStorageValue("hosts", hosts.text)
                    Playbook.setStorageValue("fileLocal", fileLocal.text)
                    Playbook.setStorageValue("fileRemote", fileRemote.text)
                    Playbook.setStorageValue("zhDesc", zhDesc.text)
                    Playbook.setStorageValue("enDesc", enDesc.text)
                    thiz.deployArrived(hosts.text, fileLocal.text, fileRemote.text, enDesc.text, zhDesc.text)
                }
            }            
        }
    }
}
