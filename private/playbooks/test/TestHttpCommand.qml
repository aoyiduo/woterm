import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import "http.js" as Http

Rectangle {
    id: thiz
    color: Playbook.backgroundColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Label {
            text: "Url:"
        }

        RowLayout {
            Layout.fillWidth: true
            TextField{
                Layout.fillWidth: true
                id: url
                text: "http://down.woterm.com/.ver"
            }
        }
        Button {
            text: "Start"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                Playbook.logClear();
                asyncGenerator(function *(){
                    yield Http.get(url.text).then(function(obj){
                        Playbook.log("success to get------------")
                        Playbook.log(obj.headers)
                        Playbook.log("\r\n\r\n"+obj.body)
                    }).catch(function(err){
                        Playbook.log("failed to get content.", err.status, err.statusText)
                    })
                })()
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }

}
