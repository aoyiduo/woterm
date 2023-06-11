import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import FileTransferCommand 1.0
import LocalFile 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    FileDialog {
        id: dlgFile
        property var ptr
        onAccepted: {
            let filePath = Playbook.urlToLocalFile(dlgFile.fileUrl)
            ptr.text = filePath
        }
    }

    LocalFile {
        id: myFile
    }

    FileTransferCommand {
        id: fileTransfer

        onSubcommandStart: {
            Playbook.log(host+": transfer start: " + fileTransfer.fileLocal + (fileDownload ? "<--" : "-->") + fileTransfer.fileRemote)
        }

        onSubcommandFinished: {
            Playbook.log(host+": command finish: exit code: "+code)
        }

        onProgressArrived: {
            Playbook.log(host+" transfer progress:"+v)
        }

        onErrorArrived: {
            Playbook.log(error)
        }
    }

    ColumnLayout{
        anchors.fill: parent
        Label {
            text: "Remote hosts:"
        }
        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: hosts
                readOnly: true
            }
            Button {
                text: "choose"
                onClicked: {
                    let host = Playbook.hostChooser()
                    if(host === "") {
                        return;
                    }

                    if(hosts.text === "") {
                        hosts.text = host;
                    }else{
                        hosts.text += "," + host;
                    }
                }
            }
        }
        RowLayout {
            spacing: 20
            RadioButton {
                id: btnUpload
                text: "Upload"
                checked: true
                onClicked: {
                    btnDownload.checked = false
                }
            }
            RadioButton {
                id: btnDownload
                text: "Download"
                checked: false
                onClicked: {
                    btnUpload.checked = false
                }
            }
        }
        ColumnLayout {
            id: uploadArea
            visible: btnUpload.checked
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label {
                text: "Local file:"
            }
            RowLayout {
                Layout.fillWidth: true
                TextField{
                    Layout.fillWidth: true
                    id: fileLocal
                    readOnly: true
                }
                Button {
                    text: "choose"
                    onClicked: {
                        dlgFile.selectFolder = false;
                        dlgFile.ptr = fileLocal
                        dlgFile.open()
                    }
                }
            }
            Label {
                text: "Remote save path:"
            }
            RowLayout {
                Layout.fillWidth: true
                TextField{
                    Layout.fillWidth: true
                    id: pathRemote
                    text: "~"
                }
            }
            RowLayout {
                Button {
                    text: "Upload"
                    onClicked: {
                        if(hosts.text === "") {
                            Playbook.log("the remote hosts is empty.")
                            return;
                        }
                        if(fileLocal.text === "") {
                            Playbook.log("the local file is empty.")
                            return;
                        }
                        if(pathRemote.text === "") {
                            Playbook.log("the remote path is empty.")
                            return;
                        }
                        let pos = fileLocal.text.lastIndexOf('/');
                        let fileName = fileLocal.text.substring(pos)
                        let fileRemote = pathRemote.text + fileName
                        fileTransfer.hosts = hosts.text.split(',')
                        //Playbook.logClear()
                        asyncGenerator(function *(){
                            Playbook.log("ready to upload---------")
                            yield fileTransferCommand(fileTransfer, fileLocal.text, fileRemote, false, false)
                            Playbook.log("success to upload----------")
                        })()
                    }
                }
                Button {
                    text: "Abort"
                    onClicked: {
                        fileTransfer.abort()
                    }
                }
            }
        }
        ColumnLayout {
            id: downloadArea
            visible: btnDownload.checked
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label {
                text: "Remote file:"
            }
            RowLayout {
                Layout.fillWidth: true
                TextField{
                    Layout.fillWidth: true
                    id: fileRemote
                    text: "~/.bashrc"
                }
            }
            Label {
                text: "Local save path:"
            }
            RowLayout {
                Layout.fillWidth: true
                TextField{
                    Layout.fillWidth: true
                    id: pathLocal
                    readOnly: true
                }
                Button {
                    text: "choose"
                    onClicked: {
                        dlgFile.selectFolder = true;
                        dlgFile.ptr = pathLocal
                        dlgFile.open()
                    }
                }
            }
            RowLayout {
                Button {
                    text: "Download"
                    onClicked: {
                        if(hosts.text === "") {
                            Playbook.log("the remote hosts is empty.")
                            return;
                        }
                        if(fileRemote.text === "") {
                            Playbook.log("the remote file is empty.")
                            return;
                        }
                        if(pathLocal.text === "") {
                            Playbook.log("the local path is empty.")
                            return;
                        }
                        let pos = fileRemote.text.lastIndexOf('/');
                        let fileName = fileRemote.text.substring(pos)
                        let fileLocal = pathLocal.text + fileName
                        fileTransfer.hosts = hosts.text.split(',')
                        //Playbook.logClear()
                        asyncGenerator(function *(){
                            Playbook.log("ready to download---------")
                            let idx = 0;
                            yield fileTransferCommand(fileTransfer, fileLocal, fileRemote.text, false, true, function(host, code){
                                let nameNew = fileLocal + idx++;
                                if(!myFile.rename(fileLocal, nameNew)) {
                                    Playbook.log("failed to rename file:", fileLocal, nameNew)
                                }else{
                                    Playbook.log("success to rename file:", fileLocal, nameNew)
                                }
                            })
                            Playbook.log("success to download----------")
                        })()
                    }
                }
                Button {
                    text: "Abort"
                    onClicked: {
                        fileTransfer.abort()
                    }
                }
            }

        }
        Item {
            Layout.fillHeight: true
        }
    }
}
