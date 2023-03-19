import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Popup {
    id: thiz
    dim: true
    modal: true
    visible: false
    padding: 10

    x: (quick.width - width) / 2
    y: (quick.height - height) / 2

    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside
    property string title: qsTr("Input Dialog")
    property string lable: qsTr("Input")
    signal result(string txt)

    function text(title, lable) {
        thiz.title = title
        thiz.lable = lable
        loader.sourceComponent = cTextFiled
        let promise = Q.promise()
        Q.shot(thiz.result, function(txt) {
            promise.done(txt)
        },thiz)
        thiz.open();
        return promise;
    }

    Component {
        id: cTextFiled
        TextField {
            id: input
        }
    }

    background: Rectangle {
        color: quick.backgroundColor
        radius: 10
    }

    contentItem: ColumnLayout{
        spacing: 5
        Label{
            Layout.fillWidth: true
            padding: 5
            text: title
            clip: true
            font.bold: true
            wrapMode: Label.WrapAnywhere
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }
        Label{
            Layout.fillWidth: true
            padding: 5
            text: lable
            clip: true
            wrapMode: Label.WrapAnywhere
        }
        Loader{
            Layout.fillWidth: true
            id: loader
            Layout.fillHeight: true
        }

        RowLayout {
            Item {
                Layout.leftMargin: 10
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("Ok")
                onClicked: {
                    var txt = loader.item.text
                    thiz.result(txt)
                    thiz.close()
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: {
                    thiz.close()
                }
            }
            Item {
                Layout.rightMargin: 10
                Layout.fillWidth: true
            }
        }
    }
}
