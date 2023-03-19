import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Rectangle {
    id: g_rootWindow
    visible: true
    color: quick.themeColor


    StackView{
        id: gstack
        anchors.fill: parent

        initialItem: MoMainView {
            stack: gstack
        }
        pushEnter: Transition {}
        pushExit: Transition {}
        popEnter: Transition {}
        popExit: Transition {}
        replaceEnter: Transition {}
        replaceExit: Transition {}
    }
}
