import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page {
    id: thiz

    signal closeArrived()

    function reload() {
        var lsv = gIdentify.qmlLoadFromSqlite();
        myModel.clear()
        for(var id in lsv) {
            var v = lsv[id]
            myModel.append(v)
        }
    }

    Component.onCompleted: {
        thiz.reload()
    }

    background: Rectangle{
        color: quick.themeColor
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/left2.png"
        title: qsTr("Identity")
        onLeftClicked: {
            closeArrived()
        }
    }

    MoIdentifyPublicKey {
        id: detail
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width * 4 / 5
        height: thiz.height / 2
    }

    MoFileDialog {
        id: dlgImport
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width - 20
        height: thiz.height - 20
        folder: "~"
        property bool failLast: false
        property string fileLast
        onFileArrived: {
            console.log("fileDialog.onselect", path, fileName)
            var file = path + "/" + fileName;
            if(failLast && file === fileLast) {
                gMessageBox.information(qsTr("Import error"), "test QInputDialog & QDialog on android.");
            }
            fileLast = file
            var retVal = gIdentify.qmlImport(file, fileName);
            if(typeof(retVal) === "string") {
                failLast = true;
                gMessageBox.information(qsTr("Import error"), retVal);
            }else{
                failLast = false
                thiz.reload()
            }
        }
    }

    contentItem: ColumnLayout {
        Flow {
            Layout.fillWidth: true

            spacing: 5
            leftPadding: 5
            rightPadding: 5
            topPadding: 5

            Button {
                text: qsTr("View")
                onClicked: {                    
                    if(listView.currentIndex < 0) {
                        gMessageBox.information(qsTr("Parameter error"), qsTr("Please select the view item first."))
                        return;
                    }
                    var item = myModel.get(listView.currentIndex)
                    detail.text = item.pubKey
                    detail.open()
                }
            }
            Button {
                text: qsTr("Import")
                onClicked: {
                    dlgImport.open()
                }
            }
            Button {
                text: qsTr("Delete")
                onClicked: {
                    if(listView.currentIndex < 0) {
                        gMessageBox.information(qsTr("Parameter error"), qsTr("Please select the view item first."))
                        return;
                    }
                    var item = myModel.get(listView.currentIndex)
                    if(item) {
                        if(gIdentify.remove(item.name)) {
                            thiz.reload()
                        }
                    }
                }
            }
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: quick.seperatorColor
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Identify list")
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id:listView

            clip:true
            currentIndex:-1

            model: ListModel {
                id: myModel
            }

            Component {
                id: itemDelegate
                Rectangle {
                    width: listView.width
                    height: 55
                    color: listView.currentIndex == index ? quick.highlightColor : quick.backgroundColor
                    Rectangle{
                        x:0
                        y:0
                        width:parent.width
                        height:1
                        color:quick.seperatorColor
                        visible: index === 0 ? true : false
                    }
                    ColumnLayout{
                        anchors.fill: parent
                        Item{
                            Layout.fillHeight: true
                        }

                        Text {
                            Layout.preferredHeight: 18
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            text: model.type + " - " + model.name
                            font.pixelSize: 15
                            verticalAlignment: Qt.AlignVCenter
                        }

                        Text {
                            Layout.preferredHeight: 12
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            text: model.figureprint
                            font.pixelSize: 10
                            verticalAlignment: Qt.AlignVCenter
                        }
                        Item{
                            Layout.fillHeight: true
                        }
                    }
                    Rectangle{
                        x:0
                        y:parent.height - 1
                        width:parent.width
                        height:1
                        color:quick.seperatorColor
                    }
                    MouseArea{
                        id:maMouse
                        anchors.fill: parent
                        onClicked: {
                            if (listView.currentIndex != index) {
                                listView.currentIndex = index;
                            }
                        }
                    }
                    RowLayout{
                        anchors.fill: parent
                        spacing: 10
                        visible: listView.currentIndex == index
                        Item{
                            Layout.fillWidth: true
                        }

                        Button{
                            flat: true
                            background: Image{
                                sourceSize.width: 32
                                sourceSize.height: 32
                                source: "qrc:/woterm/resource/skin/right2.png"
                            }
                            onClicked: {
                                if(listView.currentIndex >= 0) {
                                    var item = myModel.get(listView.currentIndex)
                                    detail.text = item.pubKey
                                    detail.open()
                                }
                            }
                        }

                        Item{
                            Layout.preferredWidth: 20
                        }
                    }
                }
            }

            delegate: itemDelegate
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
}
