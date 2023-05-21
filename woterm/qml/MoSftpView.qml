import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import MoDir 1.0

Page {
    id: thiz
    property StackView stack

    Component.onCompleted: {
        gSftpModel.pathChanged.connect(thiz.onPathChanged)
        gSftp.commandStart.connect(thiz.onCommandStart)
        gSftp.commandFinish.connect(thiz.onCommandFinish)
        gSftp.inputArrived.connect(thiz.onPasswordInputArrived)
    }

    function onPathChanged(path) {
        pathRemote.text = path
    }

    function onPasswordInputArrived(title, prompt, echo) {
        dlgPass.title = title
        dlgPass.prompt = prompt
        dlgPass.echo = echo
        dlgPass.open()
    }

    function onCommandStart(t, dm) {

    }

    function onCommandFinish(t, dm) {
        if(t === 11 || t === 12 || t === 13) {
            gSftp.openDir(pathRemote.text)
        }
    }


    onVisibleChanged: {
        if(visible) {

        }
    }

    MoDir {
        id: dir
    }

    MoPasswordInput {
        id: dlgPass
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
    }

    MoSftpNameDialog {
        id: dlgInput

        onResult: {
            console.log("directory.....", fileName)
            var path = pathRemote.text + "/" + fileName
            gSftp.mkDir(path, permission)
        }
    }

    MoFileDialog {
        id: dlgFile
    }

    MoSftpTransfer {
        id: dlgTransfer
    }

    MoSftpConfigure {
        id: dlgCfg
    }

    background: Rectangle{
        color: quick.themeColor
    }

    contentItem: ColumnLayout {
        Label {
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            font.bold: true
            text: gTitle
        }
        Flow {
            Layout.fillWidth: true

            spacing: 5
            leftPadding: 5
            rightPadding: 5
            topPadding: 0
            bottomPadding: 0

            Button {
                icon.source: "qrc../private/skins/black/home.png"
                icon.color: "transparent"
                onClicked: {
                    gSftp.openHome();
                }
            }
            Button {
                icon.source: "qrc../private/skins/black/back.png"
                icon.color: "transparent"
                onClicked: {
                    var path = pathRemote.text+"/.."
                    gSftp.openDir(path);
                }
            }
            Button {
                icon.source: "qrc../private/skins/black/forward.png"
                icon.color: "transparent"
                onClicked: {
                    var name = gSftpModel.fileName(listView.currentIndex)
                    var path = pathRemote.text+"/"+name
                    gSftp.openDir(path);
                }
            }
            Button {
                icon.source: "qrc../private/skins/black/reload.png"
                icon.color: "transparent"
                onClicked: {
                    var path = pathRemote.text
                    gSftp.openDir(path);
                }
            }
            Button {
                icon.source: "qrc../private/skins/black/menu2.png"
                icon.color: "transparent"
                onClicked: menu.open()
                Menu {
                    id: menu
                    y: parent.height
                    modal: true

                    MenuItem {
                        text: "New Directory"
                        onClicked: {
                            dlgInput.open()
                        }
                    }
                    MenuItem {
                        id: btnMultSelect
                        text: qsTr("Multiple selection")
                        checkable: true
                        onClicked: {
                            gSftpModel.clearSelection()
                        }
                    }
                    MenuItem {
                        enabled: btnMultSelect.checked
                        text: qsTr("Clear selection")
                        checkable: false
                        onClicked: {
                            gSftpModel.clearSelection()
                        }
                    }
                    MenuItem {
                        enabled: btnMultSelect.checked
                        text: qsTr("Select all")
                        checkable: false
                        onClicked: {
                            gSftpModel.selectAll()
                        }
                    }
                    MenuItem {
                        text: qsTr("Remove")
                        enabled: listView.currentIndex > 0
                        onClicked: {
                            if(listView.currentIndex <= 0) {
                                return
                            }
                            var fileName = gSftpModel.fileName(listView.currentIndex)
                            var path = pathRemote.text + "/" + fileName
                            gSftp.rmDir(path)
                        }
                    }
                    MenuItem {
                        text: qsTr("Upload")
                        onClicked: {
                            dlgFile.getOpenFileNames("~", qsTr("Select files"), qsTr("Please select one or more files to upload.")).then(function(files){
                                for(var id in files) {
                                    let file = files[id]
                                    var fi = dir.fileInfo(file)
                                    if(fi) {
                                        let fileRemote = pathRemote.text + "/" + fi.fileName
                                        let fileLocal = fi.filePath
                                        gSftpTransfer.addTask(fileLocal, fileRemote, fi.isDir, false, true)
                                    }
                                }
                                dlgTransfer.open()
                            })
                        }
                    }
                    MenuItem {
                        text: qsTr("Download")
                        onClicked: {
                            var fileInfos = []
                            if(btnMultSelect.checked) {
                                fileInfos = gSftpModel.fileInfosFromSelection();
                            }else{
                                var fileInfo = gSftpModel.fileInfo(listView.currentIndex)
                                if(fileInfo) {
                                    fileInfos.push(fileInfo)
                                }
                            }

                            if(fileInfos.length <= 0) {
                                gMessageBox.warning(qsTr("Selection error"), qsTr("The current selection is empty."));
                                return
                            }

                            dlgFile.getExistingDirectory("~", qsTr("Select directory"), qsTr("Please select a directory to save download file.")).then(function(pathSave){
                                for(var id in fileInfos) {
                                    let fi = fileInfos[id]
                                    let fileRemote = pathRemote.text + "/" + fi.name
                                    let fileLocal = pathSave + "/" + fi.name
                                    gSftpTransfer.addTask(fileLocal, fileRemote, fi.type === "d", true, true)
                                }
                                dlgTransfer.open()
                            })
                        }
                    }                    
                    MenuItem {
                        text: qsTr("Close")
                        onClicked: {
                            Qt.quit()
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
            id: pathRemote
            Layout.fillWidth: true
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: quick.seperatorColor
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id:listView

            clip:true
            currentIndex:-1

            model: gSftpModel

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
                    RowLayout{
                        anchors.fill: parent
                        Image {
                            Layout.leftMargin: 5
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            sourceSize.width: 32
                            sourceSize.height: 32
                            source: model.iconUrl
                        }

                        ColumnLayout{
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Item{
                                Layout.fillHeight: true
                            }

                            Text {
                                Layout.preferredHeight: 18
                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                                text: model.name
                                font.pixelSize: 15
                                verticalAlignment: Qt.AlignVCenter
                            }

                            Text {
                                Layout.preferredHeight: 12
                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                                text: model.shortString
                                font.pixelSize: 10
                                verticalAlignment: Qt.AlignVCenter
                            }
                            Item{
                                Layout.fillHeight: true
                            }
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
                        spacing: 0
                        Item{
                            Layout.fillWidth: true
                        }

                        Button{
                            flat: true
                            visible: model.type==="d" && listView.currentIndex == index
                            background: Image{
                                sourceSize.width: 32
                                sourceSize.height: 32
                                source: "qrc../private/skins/black/right2.png"
                            }
                            onClicked: {
                                var path = pathRemote.text + "/" + model.name
                                gSftp.openDir(path);
                            }
                        }
                        CheckBox {
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignVCenter

                            visible: btnMultSelect.checked && (!(model.name === "." || model.name === ".."))
                            checked: model.selected
                            checkable: true
                            onClicked: {
                                model.selected = checked
                            }
                        }
                    }
                }
            }

            delegate: itemDelegate
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
}
