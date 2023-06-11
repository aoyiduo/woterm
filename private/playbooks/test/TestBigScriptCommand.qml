import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import BigScriptCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    BigScriptCommand {
        id: remote

        onSubcommandStart: {
            Playbook.log(host+": command start: ")
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
            TextArea {
                Layout.fillWidth: true
                id: cmd
                Component.onCompleted: {
                    cmd.append("#!/usr/bin/env bash")
                    cmd.append("df -h")
                    cmd.append("ps -ef")
                    cmd.append("ping -c 3 127.0.0.1")
                    cmd.append("ls woterm")
                    cmd.append("if [ $? -eq 0 ]; then")
                    cmd.append("  echo \"yes, it is ok\"")
                    cmd.append("else")
                    cmd.append("  echo \"Sorry, it is bad\"")
                    cmd.append("fi")
                }
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
                        Playbook.log("script start.")
                        remote.hosts = hosts.text.split(',')
                        yield bigScriptCommand(remote, cmd.text).catch(function(){})
                        Playbook.log("script finish.")
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
