import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Drawer {
    id: thiz
    width: quick.width * 0.66
    height: quick.height

    signal aboutArrived();
    signal languageArrived();
    signal systemInformationArrived();

    Column {
        anchors.fill: parent

        ItemDelegate {
            text: qsTr("About")
            width: parent.width
            onClicked: {
                thiz.close()
                thiz.aboutArrived()
            }
        }
        ItemDelegate {
            text: qsTr("Language")
            width: parent.width
            onClicked: {
                thiz.close()
                thiz.languageArrived()
            }
        }
        ItemDelegate {
            text: qsTr("Office website")
            width: parent.width
            onClicked: {
                quick.openUrl("http://www.woterm.com")
                thiz.close()
            }
        }
        ItemDelegate {
            text: qsTr("Exit application")
            width: parent.width
            onClicked: {
                thiz.close()
                Qt.quit()
            }
        }
        ItemDelegate {
            text: qsTr("System information")
            width: parent.width
            visible: quick.debugVersion
            onClicked: {
                thiz.close()
                thiz.systemInformationArrived()
            }
        }
    }
}
