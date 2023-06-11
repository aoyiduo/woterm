import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import LocalCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    LocalCommand {
        id: local
        onDataArrived: {
            Playbook.log(data)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Label {
            text: "Local command:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: cmd
                text: "ping 127.0.0.1"
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
                    asyncGenerator(function *(){
                        Playbook.log("command start.")
                        yield localCommand(local, cmd.text).catch(function(){})
                        Playbook.log("command finish.")
                    })();
                }
            }
            Button {
                text: "Abort"

                onClicked: {
                    local.abort()
                }
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
