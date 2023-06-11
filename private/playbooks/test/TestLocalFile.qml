import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import LocalFile 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    FileDialog {
        id: dlgFile
        property var ptr: null
        onAccepted:  {
            Playbook.log(dlgFile.fileUrl)
            let path = Playbook.urlToLocalFile(dlgFile.fileUrl)
            ptr.text = path
            if(ptr == fileGet) {
                fileSave.text = path + ".copy"
            }
        }
    }

    LocalFile {
        id: fileReader
    }

    LocalFile {
        id: fileWriter
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Label {
            text: "File read path:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: fileGet
                readOnly: true
            }
            Button {
                text: "Browser"
                onClicked: {
                    dlgFile.ptr = fileGet
                    dlgFile.selectExisting = true
                    dlgFile.selectFolder = false
                    dlgFile.open()
                }
            }
        }
        Label {
            text: "File save path:"
        }
        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: fileSave
                readOnly: true
            }
            Button {
                text: "Browser"
                onClicked: {
                    dlgFile.ptr = fileSave
                    dlgFile.selectExisting = false
                    dlgFile.selectFolder = false
                    dlgFile.open()
                }
            }
        }
        Button {
            text: "Start"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                Playbook.logClear();
                if(!fileReader.exist(fileGet.text)){
                    Playbook.log("the file is not exist")
                    return
                }
                if(!fileWriter.open(fileSave.text, false)) {
                    Playbook.log("failed to write file:", file.errorString())
                    return;
                }
                if(!fileReader.open(fileGet.text, true)) {
                    Playbook.log("failed to read file.")
                    return
                }

                let fsize = fileReader.fileSize();
                let bsize = 1024 * 100
                let cnt = (fsize + bsize-1)  / bsize;
                for(let i = 0; i < cnt; i++) {
                    let data = fileReader.read(1024);
                    fileWriter.write(data)
                    Playbook.log("read index.", i)
                }
                fileReader.close()
                fileWriter.close()
                Playbook.log("success to finish.")
            }
        }
        Item {
            objectName: "resizeIgnore"
            Layout.fillHeight: true
        }
    }
}
