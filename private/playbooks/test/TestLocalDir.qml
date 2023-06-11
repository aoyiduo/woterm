import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import LocalDir 1.0

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    FileDialog {
        id: dlgFile
        selectFolder: true
        onAccepted:  {
            Playbook.log(dlgFile.fileUrl)
            let path = Playbook.urlToLocalFile(dlgFile.fileUrl)
            filePath.text = path
        }
    }

    LocalDir {
        id: dir
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Label {
            text: "Local directory:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: filePath
                readOnly: true
                text: "/"
            }
            Button {
                text: "Browser"
                onClicked: {
                    dlgFile.open()
                }
            }
        }
        Button {
            text: "Start"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                Playbook.logClear();
                Playbook.log("filePath", filePath.text)
                if(!dir.exist(filePath.text)){
                    Playbook.log("the file is not exist")
                    return
                }
                let lsv = dir.entryInfoList(filePath.text)
                let first = ""
                for(let i = 0; i < lsv.length; i++) {
                    let v = lsv[i];
                    let msgs = []
                    for(var id in v) {
                        msgs.push(id+":"+v[id])
                    }
                    if(first === "") {
                        first = v["filePath"]
                    }
                    Playbook.log(msgs.join(','))
                }
                Playbook.log("first:"+first+"---------------------------")
                let fi = dir.fileInfo(first)
                for(let key in fi) {
                    Playbook.log(key + ":" + fi[key])
                }
            }
        }
        Item {            
            Layout.fillHeight: true
        }
    }
}
