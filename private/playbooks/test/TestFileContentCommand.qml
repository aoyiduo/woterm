import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import FileContentCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    FileDialog {
        id: dlgFile
        onAccepted:  {
            fileRemote.text = Playbook.urlToLocalFile(dlgFile.fileUrl)
        }
    }

    FileContentCommand {
        id: fileContent

        onSubcommandStart: {
            Playbook.log(host+": command start: "+filePath)
        }

        onSubcommandFinished: {
            Playbook.log(host+": command finish: exit code: "+code)
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
        Label {
            text: "Remote file:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: fileRemote
                text: "~/fileContent.txt"
            }
        }

        Label {
            text: "FileContent:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: myContent
                text: "This is a text."
            }
        }
        RowLayout {
            Button {
                text: "Write"
                onClicked: {
                    if(hosts.text === "") {
                        Playbook.information("Parameter error", "hosts is empty.")
                        return
                    }
                    Playbook.logClear()
                    if(false) {
                        fileContent.filePath = fileRemote.text
                        fileContent.hosts = hosts.text.split(',')
                        fileContent.content = myContent.text
                        fileContent.fileRead = false
                        fileContent.start();
                    }else{
                        asyncGenerator(function*(){
                            fileContent.hosts = hosts.text.split(',')
                            yield fileContentCommand(fileContent, fileRemote.text, myContent.text)
                            Playbook.log("fileWrite", fileContent.content);
                        })()
                    }
                }
            }
            Button {
                text: "Read"
                onClicked: {
                    if(hosts.text === "") {
                        Playbook.information("Parameter error", "hosts is empty.")
                        return
                    }
                    Playbook.logClear()
                    asyncGenerator(function*(){
                        fileContent.hosts = hosts.text.split(',')
                        yield fileContentCommand(fileContent, fileRemote.text, true)
                        Playbook.log("fileRead", fileContent.content);
                    })()
                }
            }
            Button {
                text: "Abort"
                onClicked: {
                    fileContent.abort()
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
