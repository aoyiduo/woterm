import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoDir 1.0

Popup {
    id: thiz
    dim: true
    modal: true
    visible: false
    padding: 10

    x: 10
    y: (quick.height - height) / 2
    width: quick.width - 20

    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside

    MoDir {
        id: dir
    }

    MoFileDialog {
        id: dlgFile
    }

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: Column {
        spacing: 5
        Label{
            width: parent.width
            padding: 5
            text: qsTr("Sftp Configure")
            clip: true
            font.bold: true
            wrapMode: Label.WrapAnywhere
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }
        Rectangle {
            width: parent.width
            height: 1
            color: "gray"
        }

        Label{
            width: parent.width
            padding: 5
            text: qsTr("Download path")
            clip: true
            wrapMode: Label.WrapAnywhere
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        Label{
            width: parent.width
            padding: 5
            text: qsTr("Please input a path to save download files.")
            clip: true
            wrapMode: Label.WrapAnywhere
        }

        TextField {
            width: parent.width
            id: pathSave
            readOnly: true
            text: gSetting.downloadPath()
            Button{
                x: parent.width - 50
                y: 0
                width: 50
                height: parent.height
                Image{
                    anchors.centerIn: parent
                    source: "qrc../private/skins/black/dirs.png"
                    sourceSize.width: 16
                    sourceSize.height: 16
                }
                onClicked: {
                    dlgFile.getExistingDirectory(pathSave.text).then(function(path){
                        var fi = dir.fileInfo(path)
                        console.log("getExistingDirectory", fi.permissions)
                        if(fi && !fi.canWrite) {
                            gMessageBox.warning(qsTr("Write error"), qsTr("The directory does not have write permission."))
                            return
                        }
                        pathSave.text = path
                    })
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: qsTr("Ok")
                width: 100
                onClicked: {
                    if(pathSave.text === "") {
                        gMessageBox.information(qsTr("Parameter error"), qsTr("The directory name can not be empty."));
                        return
                    }
                    gSetting.setDownloadPath(pathSave.text)
                    thiz.close()
                }
            }
            Button {
                width: 100
                text: qsTr("Cancel")
                onClicked: {
                    thiz.close()
                }
            }
        }
    }
}
