import QtQuick 2.0

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import MoDir 1.0

Page {
    id: thiz

    signal closeArrived()

    Component.onCompleted: {
    }

    background: Rectangle{
        color: quick.themeColor
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/left2.png"
        title: qsTr("System information.")
        onLeftClicked: {
            closeArrived()
        }
    }
    MoDir {
        id: dir
    }

    contentItem: Flickable{
        ColumnLayout{
            Label{
                Layout.fillWidth: true
                text: qsTr("Download path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.downloadLocation()
                wrapMode: Label.WrapAnywhere
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("Temp path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.tempLocation()
                wrapMode: Label.WrapAnywhere
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("Picture path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.picturesLocation()
                wrapMode: Label.WrapAnywhere
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("AppConfig path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.appConfigLocation()
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("AppData path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.appDataLocation()
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("Home path:")
            }
            Label {
                Layout.fillWidth: true
                text:dir.homeLocation()
            }
        }
    }
}
