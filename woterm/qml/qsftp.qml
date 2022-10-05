import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.1

Item {
    id: root
    property int zval: 1000

    Component{
        id: myMenuItem
        MenuItem {
            property string icon
            property string label
            contentItem: RowLayout{
                Image{
                    Layout.preferredWidth: 24
                    Layout.fillHeight: true
                    source: icon
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 24
                    sourceSize.height: 24
                }
                Text {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    text: label
                }
            }
        }
    }

    function onFileSelected(file) {
        var idx = file.lastIndexOf("/");
        var name = file.substring(idx)
        var remote = ftpModel.path() + name
        var local = file;
        ftp.upload(local, remote)
        transferDesc.text = qsTr("upload:") + name;
    }

    function onFolderSelected(folder) {
        if(listView.currentIndex > 0) {
            var name = ftpModel.name(listView.currentIndex)
            var remote = ftpModel.path() + "/" + name
            var local = folder +"/" + name;
            ftp.download(remote, local)
            transferDesc.text = qsTr("download:") + name;
        }
    }

    Component.onCompleted: {
        fileDialog.fileSelected.connect(onFileSelected)
        folderDialog.fileSelected.connect(onFolderSelected)
    }

    ColumnLayout{
        anchors.fill: parent
        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
            Component.onCompleted: {
                ftpModel.bindToView(listView)
                ftp.errorArrived.connect(onErrorArrived)
            }

            function onErrorArrived(msg) {
                msgbox.content = msg
                msgbox.visible = true
            }

            onContentYChanged: {
                menu.close()
            }

            model: ftpModel
            Component {
                id: folderDelegate
                Rectangle {
                    width: listView.width
                    height: 30
                    color:listView.currentIndex == index ? "#D4237A" : "white"
                    ColumnLayout{
                        anchors.fill: parent
                        Rectangle{
                            Layout.preferredHeight: 1
                            Layout.fillWidth: true
                            color:"lightGray"
                            visible:index === 0 ? true : false
                        }
                        RowLayout{
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Image {
                                Layout.fillHeight: true;
                                Layout.preferredWidth: 24;
                                source: ftpModel.icon(index)
                            }
                            Text {
                                Layout.preferredHeight: 18
                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                                font.family: "Courier New"
                                text: ftpModel.label(index)
                                font.pixelSize: 15
                                verticalAlignment: Qt.AlignVCenter
                            }
                        }
                        Rectangle{
                            Layout.preferredHeight: 1
                            Layout.fillWidth: true
                            color:"lightGray"
                        }
                    }
                }
            }
            delegate: folderDelegate
            ScrollIndicator.vertical: ScrollIndicator { }
            MouseArea{
                id:mouseArea
                z: 10
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onDoubleClicked: {
                    var mouseX = listView.contentX + mouse.x
                    var mouseY = listView.contentY + mouse.y
                    var idx = listView.indexAt(mouseX, mouseY);
                    listView.currentIndex = idx
                    if(idx >= 0 && ftpModel.type(idx) !== "-") {
                        var path = ftpModel.path();
                        path += "/" + ftpModel.name(idx);
                        ftp.openDir(path);
                    }
                }                

                onClicked: {
                    var mouseX = listView.contentX + mouse.x
                    var mouseY = listView.contentY + mouse.y
                    var idx = listView.indexAt(mouseX, mouseY);
                    listView.currentIndex = idx
                    if(mouse.button & Qt.RightButton) {
                        menu.x = mouse.x
                        menu.y = mouse.y
                        menu.focus = true
                        menu.removeItem(3)
                        menu.removeItem(3)
                        menu.removeItem(3)
                        menu.removeItem(3)
                        menu.removeItem(3)
                        menu.removeItem(3)
                        if(ftpModel.type(idx) === "-") {
                            menu.addItem(menuDownload)
                            menu.addItem(menuRmFile)
                        }else if(ftpModel.type(idx) === "d") {
                            menu.addItem(menuRmDir)
                            menu.addItem(menuDir)
                        }else if(ftpModel.type(idx) === "l") {
                            menu.addItem(menuLink)
                            menu.addItem(menuRmFile)
                        }
                        menu.addItem(menuUpload)
                        menu.open()
                    }else{
                        menu.close()
                    }
                }
            }
            Menu {
                id: menu
                focus: true
                closePolicy: ~Popup.NoAutoClose

                function onDirEnter() {
                    console.log("dirEnter")
                    if(listView.currentIndex > 0) {
                        var name = ftpModel.name(listView.currentIndex)
                        ftp.openDir(ftpModel.path() + "/" + name)
                    }else{
                        ftp.openDir(ftpModel.path() + "/..")
                    }
                    menu.close()
                }

                function onReturn() {
                    ftp.openDir(ftpModel.path() + "/..")
                    menu.close()
                }

                function onReflesh() {
                    ftp.openDir(ftpModel.path() + "/.")
                    menu.close()
                }

                function onDownload() {
                    menu.close()
                    folderDialog.open()
                }

                function onUpload() {
                    menu.close();
                    fileDialog.open()
                }

                function onMakeDirectory() {
                    dirmake.visible = true                    
                    menu.close();
                }

                function onRemoveDirectory() {
                    if(listView.currentIndex > 0) {
                        var name = ftpModel.name(listView.currentIndex)
                        ftp.rmDir(ftpModel.path(), name)
                    }
                    menu.close();
                }

                function onRemoveFile() {
                    if(listView.currentIndex > 0) {
                        var name = ftpModel.name(listView.currentIndex)
                        ftp.unlink(ftpModel.path(), name)
                    }
                    menu.close();
                }

                Loader {
                    id: menuBack
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/back3.png"
                        item.label = qsTr("Back")
                        item.clicked.connect(menu.onReturn)
                    }
                }
                Loader {
                    id: menuReflesh
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/reload.png"
                        item.label = qsTr("Reflesh")
                        item.clicked.connect(menu.onReflesh)
                    }
                }
                Loader {
                    id: menuMkDir
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/mkdir.png"
                        item.label = qsTr("Create Directory")
                        item.clicked.connect(menu.onMakeDirectory)
                    }
                }
                Loader {
                    id: menuRmDir
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/rmdir.png"
                        item.label = qsTr("Remove Directory")
                        item.clicked.connect(menu.onRemoveDirectory)
                    }
                }
                Loader {
                    id: menuRmFile
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/rmfile.png"
                        item.label = qsTr("Remove File")
                        item.clicked.connect(menu.onRemoveFile)
                    }
                }
                Loader {
                    id: menuDir
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/enter.png"
                        item.label = qsTr("Enter")
                        item.clicked.connect(menu.onDirEnter)
                    }
                }
                Loader {
                    id: menuLink
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/enter3.png"
                        item.label = qsTr("TryEnter")
                        item.clicked.connect(menu.onDirEnter)
                    }
                }
                Loader {
                    id: menuDownload
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/download2.png"
                        item.label = qsTr("Download")
                        item.clicked.connect(menu.onDownload)
                    }
                }
                Loader {
                    id: menuUpload
                    sourceComponent: myMenuItem
                    onLoaded: {
                        item.icon = "qrc:/qwoterm/resource/skin/upload2.png"
                        item.label = qsTr("Upload")
                        item.clicked.connect(menu.onUpload)
                    }
                }
                onFocusChanged: {
                    if(menu.opend) {
                        menu.close()
                    }
                }
            }
        }
    }
    Item{
        anchors.fill: parent
        id: waitCommand
        z: visible ? root.zval++ : 100
        Component.onCompleted: {
            ftp.commandStart.connect(onCommandStart)
            ftp.commandFinish.connect(onCommandFinish)
            ftp.connectionStart.connect(onConnectionStart)
            ftp.connectionFinished.connect(onConnectionFinished)
            ftp.progress.connect(onProgress)
        }

        function onCommandStart(t) {
            if(t === 15 || t === 14) {
                transfer.visible = true
            }else{
                waitCommand.visible = true
            }
        }

        function onCommandFinish(t) {
            waitCommand.visible = false
            if(t === 15 || t === 14) {
                transfer.visible = false
            }else{
                waitCommand.visible = false
            }
        }

        function onConnectionStart() {
            waitCommand.visible = true;
        }

        function onConnectionFinished(ok) {
            if(!ok) {
                waitCommand.visible = false;
            }
        }

        function onProgress(t, v) {
            transVal.value = v
        }

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        Image{
            anchors.centerIn: parent
            sourceSize.width: 48
            sourceSize.height: 48
            source: "qrc:/qwoterm/resource/skin/loading.png"
            RotationAnimation on rotation {
                from: 0
                to: 360
                duration: 1500
                running: waitCommand.visible
                loops: Animation.Infinite
            }
        }
    }
    Item {
        anchors.fill: parent
        z: visible ? root.zval++ : 100
        id:dirmake
        visible: false
        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }
        Rectangle {
            anchors.centerIn: parent
            width: 300
            height:200
            radius: 10
            border.width: 2
            border.color: "gray"
            ColumnLayout{
                anchors.fill: parent
                Text {
                    Layout.preferredHeight: 35
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Create Directory")
                    font.bold: true
                    font.pointSize: 13
                }
                Rectangle {
                    color: "gray"
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                }
                Item{
                    Layout.preferredHeight: 30
                    Layout.fillWidth: true
                    Layout.bottomMargin: 10
                    CheckBox {
                        anchors.centerIn: parent
                        id: prv
                        text: qsTr("Private")
                    }
                }

                RowLayout {
                    Layout.preferredHeight: 30
                    Layout.fillWidth: true
                    Layout.leftMargin: 30
                    Layout.rightMargin: 30
                    Layout.bottomMargin: 0
                    Layout.topMargin: 0
                    Text{
                        Layout.preferredWidth: contentWidth + leftPadding + rightPadding
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        text:qsTr("Name:")
                    }
                    TextField{
                        id: textField
                        Layout.fillWidth: true
                        readOnly: false
                        hoverEnabled: true
                        selectByMouse: true
                        placeholderText: qsTr("Enter Directory Name")
                    }
                }
                RowLayout{
                    Layout.topMargin: 15
                    Layout.preferredHeight: 25
                    Layout.fillWidth: true
                    spacing: 10
                    Item{
                        Layout.fillWidth: true
                    }
                    Button{
                        text: qsTr("Continue")
                        onClicked: {
                            if(textField.text.length == 0) {
                                return
                            }
                            ftp.mkDir(ftpModel.path(), textField.text, prv.checked? 0x1C0 : 0x1FF);
                            dirmake.visible = false
                        }
                    }

                    Button{
                        text: qsTr("Cancel")
                        onClicked: {
                            dirmake.visible = false
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }
                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }
    Item {
        anchors.fill: parent
        z: visible ? root.zval++ : 100
        id:msgbox
        visible: false
        property alias content: msgContent.text
        property alias title: msgTitle.text
        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }
        Rectangle {
            anchors.centerIn: parent
            width: 300
            height: {
                var h = txtArea.implicitHeight + 10
                return h
            }
            radius: 10
            border.width: 2
            border.color: "gray"
            ColumnLayout{
                id: txtArea
                anchors.fill: parent
                spacing: 0
                Text {
                    id: msgTitle
                    Layout.preferredHeight: 35
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Information")
                    font.bold: true
                    font.pointSize: 13
                }
                Rectangle {
                    color: "gray"
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                }

                Text {
                    id: msgContent
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.margins: 10
                    wrapMode: Text.WrapAnywhere
                    text:""
                }

                RowLayout{
                    Layout.topMargin: 15
                    Layout.fillWidth: true
                    Layout.bottomMargin: 15
                    spacing: 10
                    Item{
                        Layout.fillWidth: true
                    }
                    Button{
                        text: qsTr("Ok")
                        Layout.maximumHeight: 30
                        onClicked: {
                            msgbox.visible = false
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
    Item {
        anchors.fill: parent
        z: visible ? root.zval++ : 100
        id: transfer
        visible: false
        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }
        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: 150
            radius: 10
            border.width: 2
            border.color: "gray"
            ColumnLayout{
                anchors.fill: parent
                spacing: 0
                Text {
                    id: caption
                    Layout.preferredHeight: 35
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Progress")
                    font.bold: true
                    font.pointSize: 13
                }
                Rectangle {
                    color: "gray"
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                }
                Text {
                    Layout.margins: 10
                    Layout.fillWidth: true
                    id: transferDesc
                    text: ""
                }
                ProgressBar{
                    id: transVal
                    Layout.margins: 10
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                }

                RowLayout{
                    Layout.topMargin: 15
                    Layout.fillWidth: true
                    Layout.bottomMargin: 15
                    spacing: 10
                    Item{
                        Layout.fillWidth: true
                    }
                    Button{
                        text: qsTr("Abort")
                        Layout.maximumHeight: 30
                        onClicked: {
                            ftp.abort()
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
