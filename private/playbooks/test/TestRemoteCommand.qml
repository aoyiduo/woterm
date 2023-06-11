import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import RemoteCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    RemoteCommand {
        id: remote

        onSubcommandStart: {
            Playbook.log(host+": command start: "+command)
        }

        onSubcommandFinished: {
            Playbook.log(host+": command finish: exit code: "+code)
        }

        onDataArrived: {
            Playbook.log(data)
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
                id: cmd
                text: "ping -c 3 127.0.0.1"
            }
        }
        Row {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: "Start"
                onClicked: {
                    Playbook.logClear();
                    if(cmd.text === "") {
                        Playbook.log("the command text is empty.")
                        return
                    }
                    if(hosts.text === "") {
                        Playbook.log("the hosts is empty.")
                        return
                    }

                    asyncGenerator(function *(){
                        Playbook.log("command start.")
                        remote.hosts = hosts.text.split(',')
                        yield remoteCommand(remote, cmd.text).catch(function(){})
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
