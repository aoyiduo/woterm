import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2


Rectangle {
    color: Playbook.backgroundColor
    StackView{
        id: gstack
        anchors.fill: parent

        Component {
            id: proc
            MyDeployProcess {
                onPreviousArrived: {
                    gstack.pop()
                }
            }
        }

        initialItem: MyDeployConfigure {
            onDeployArrived: {
                gstack.push(proc, {"hosts": hosts, "fileLocal":fileLocal, "fileRemote": fileRemote, "enDesc": enDesc, "zhDesc": zhDesc});
            }
        }
        pushEnter: Transition {}
        pushExit: Transition {}
        popEnter: Transition {}
        popExit: Transition {}
        replaceEnter: Transition {}
        replaceExit: Transition {}
    }
}
