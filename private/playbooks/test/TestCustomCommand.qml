import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Rectangle {
    id: thiz
    color: Playbook.backgroundColor
    Label {
        text: ""
    }

    Timer {
        id: timer
    }

    function sleep(ms) {
        return new Promise(function(resolve, reject){
            timer.interval = ms
            timer.repeat = false
            timer.running = false

            let fnTriggered = function() {
                timer.triggered.disconnect(fnTriggered)
                Qt.callLater(resolve)
            }
            timer.triggered.connect(fnTriggered)
            timer.start()
        })
    }

    function assertEqual(x, y) {
        return new Promise(function(resolve, reject) {
            if(x === y) {
                Qt.callLater(resolve, "Yes, it's equal.")
            }else{
                Qt.callLater(reject, "No, it's not equal.")
            }
        });
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Button {
            text: "Start"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                Playbook.logClear();
                asyncGenerator(function *(){
                    for(let i = 0; i < 10; i++) {
                        let a = Math.random() * 100;
                        let b = Math.random() * 100;
                        let x = Math.floor(a % 2)
                        let y = Math.floor(b % 2)
                        Playbook.log("loop i=", i, x, y)
                        yield assertEqual(x, y).then(function(result){
                            Playbook.log("assertResult.then", result)
                        }).catch(function(err){
                            Playbook.log("assertResult.catch", err)
                        })
                        yield sleep(1000)
                    }
                })()
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }

}
