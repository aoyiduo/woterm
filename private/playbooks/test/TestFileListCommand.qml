import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import FileListCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    FileListCommand {
        id: fileList

        onSubcommandStart: {
            Playbook.log(host+": command start: ")
        }

        onSubcommandFinished: {
            Playbook.log(host+": command finish: exit code: "+code)
        }

        onErrorArrived: {
            Playbook.log(error)
        }
    }

    ColumnLayout {
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
            text: "Remote command:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: pathRemote
                text: "~"
            }
        }
        Row {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: "Start"
                onClicked: {
                    Playbook.logClear();
                    if(pathRemote.text === "") {
                        Playbook.log("the command text is empty.")
                        return
                    }
                    if(hosts.text === "") {
                        Playbook.log("the hosts is empty.")
                        return
                    }

                    asyncGenerator(function *(){
                        Playbook.log("command start.")
                        fileList.hosts = hosts.text.split(',')
                        yield fileListCommand(fileList, pathRemote.text, function(host, code){
                            Playbook.log(host+"---------------------------")
                            Playbook.log(host+"---------------------------")
                            Playbook.log(host+"---------------------------")
                            Playbook.log("show "+host+" file names----------------.")
                            Playbook.log(fileList.fileNames)
                            Playbook.log("show "+host+" file informations----------------.")
                            for(let id in fileList.fileInfos) {
                                let fi = fileList.fileInfos[id]
                                let msgs = []
                                for(let key in fi) {
                                    msgs.push(key+":"+fi[key])
                                }
                                Playbook.log(msgs.join(','))
                            }
                        })

                        Playbook.log("command finish.")
                    })();
                }
            }
            Button {
                text: "Abort"

                onClicked: {
                    remote.abort()
                }
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
