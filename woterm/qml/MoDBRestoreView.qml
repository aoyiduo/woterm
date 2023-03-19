import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page {
    id: thiz

    signal closeArrived()

    background: Rectangle{
        color: quick.themeColor
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/left2.png"
        title: qsTr("Restore")
        onLeftClicked: {
            closeArrived()
        }
    }

    Component {
        id: stepFinish
        MoDBRestoreFinish {
            onPreviousStep: {
                mystack.pop()
            }
            onCloseArrived: {
                thiz.closeArrived()
            }
        }
    }

    Component {
        id: stepSetup
        MoDBRestoreSetup {
            onNextStep: {
                mystack.push(stepFinish)
            }
        }
    }

    contentItem: StackView {
        id: mystack
        initialItem: stepSetup
    }
}
