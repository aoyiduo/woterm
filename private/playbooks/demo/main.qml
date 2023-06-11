import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import LocalDir 1.0
import LocalCommand 1.0
import RemoteCommand 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor
    property var output

    LocalCommand {
        id: local
        onCommandStart: {
            output.append("command start: "+command)
        }

        onCommandFinished:  {
            output.append("command finish: exit code:"+lastExitCode)
        }

        onDataArrived: {
            output.append(data)
        }
    }

    RemoteCommand {
        id: remote
        onSubcommandStart: {
            output.append(host+": command start: "+command)
        }

        onSubcommandFinished: {
            output.append(host+": command finish: exit code: "+code)
        }

        onDataArrived: {
            output.append(data)
        }
        onErrorArrived: {
            output.append(error)
        }
    }

    Timer {
        id: mytimer
        interval: 1000
        repeat: false
        running: false

        function sleep(ms) {
            return new Promise(function(resolve, reject) {
                let fnOnTriggered = function(){
                    mytimer.triggered.disconnect(fnOnTriggered)
                    Qt.callLater(resolve);
                }
                mytimer.triggered.connect(fnOnTriggered)
                mytimer.interval = ms;
                mytimer.start()
            });
        }
    }

    Button{
        anchors.centerIn: parent
        text: "Run script"
        onClicked: {
            let name = Playbook.hostChooser()
            if(name.length <= 0) {
                return;
            }
            remote.hosts = [name];
            asyncGenerator(function *(){
                Playbook.openTerminal();
                thiz.output = Playbook.terminal();
                for(let i = 0; i < 10; i++) {
                    //---
                    output.append("-----------local command: "+i)
                    yield localCommand(local, "ls ~").catch(function(){})
                    //---
                    output.append("-----------sleep for a second "+i)
                    yield mytimer.sleep(1000);
                    //---
                    output.append("-----------remote command:  "+i)
                    yield remoteCommand(remote, "ls ~")
                }
               // Playbook.closeTerminal();
            })();
        }
    }
}
