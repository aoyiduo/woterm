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
    padding: 0
    parent: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent | Popup.CloseOnPressOutside

    x: 10
    y: 10
    width: quick.width - 20
    height: quick.height - 20

    property string title: qsTr("File dialog")
    property string label: qsTr("Please select a directory.")
    property string folder: "~"
    property bool showFiles: false
    property bool showDot: false
    property bool showDotDot: false
    property var folderSelect
    property alias nameFilters: dir.nameFilters
    property bool multipleSelect: false

    // fileInfos: FileInfoList
    // multipleSelect: false
    signal fileArrived(string path, string fileName)
    signal fileInfoArrived(string path, var fileInfo)
    // multipleSelect: true
    signal filesArrived(string path, var fileNames)
    signal fileInfosArrived(string path, var fileInfos)

    function setPath(path) {
        thiz.folder = dir.absoluteFilePath(path)
        var items = dir.entryInfoList(path)
        myModel.clear()
        for(var id in items) {
            let item = items[id]
            if(!thiz.showFiles) {
                if(item.isFile) {
                    continue
                }
            }
            if(item.isDir) {
                if(!thiz.showDot) {
                    if(item.fileName === ".") {
                        continue
                    }
                }
                if(!thiz.showDotDot) {
                    if(item.fileName === "..") {
                        continue
                    }
                }
            }

            item["selected"] = false
            item["shortString"] = item.isDir ? item.permissions : item.permissions + " " + item.size
            myModel.append(item)
        }
    }

    function getExistingDirectory(path, title, label) {
        thiz.multipleSelect = false
        thiz.folderSelect = true
        thiz.showFiles = false
        if(typeof(title) === "string") {
            thiz.title = title;
        }
        if(typeof(label) === "string") {
            thiz.label = label;
        }

        let result = Q.promise()
        Q.shot(thiz.fileArrived, function(path, fileName) {
            var absPath = dir.absoluteFilePath(path+"/"+fileName)
            result.done(absPath)
        }, thiz)
        thiz.setPath(path)
        thiz.open()
        return result;
    }

    function getOpenFileName(path, title, label) {
        thiz.showFiles = true
        thiz.multipleSelect = false
        if(typeof(title) === "string") {
            thiz.title = title;
        }
        if(typeof(label) === "string") {
            thiz.label = label;
        }
        let result = Q.promise()
        Q.shot(thiz.fileArrived, function(path, fileName) {
            var file = path + "/" + fileName
            result.done(file)
        }, thiz)
        thiz.setPath(path);
        thiz.open()
        return result;
    }

    function getOpenFileNames(path, title, label) {
        thiz.showFiles = true
        thiz.multipleSelect = true
        if(typeof(title) === "string") {
            thiz.title = title;
        }
        if(typeof(label) === "string") {
            thiz.label = label;
        }
        let result = Q.promise()
        Q.shot(thiz.filesArrived, function(path, fileNames) {
            var files = []
            for(var id in fileNames) {
                files.push(path + "/" + fileNames[id])
            }
            result.done(files)
        }, thiz)
        thiz.setPath(path);
        thiz.open()
        return result;
    }

    function clearSelection() {
        for(var i = 0; i < myModel.count; i++) {
            let item = myModel.get(i);
            item["selected"] = false
            myModel.set(i, item)
        }
    }

    function selectAll() {
        for(var i = 0; i < myModel.count; i++) {
            let item = myModel.get(i);
            item["selected"] = true
            myModel.set(i, item)
        }
    }

    Component.onCompleted: {
        thiz.setPath(thiz.folder);
    }

    background: Rectangle{
        radius: 8
        color: "white"
    }
    Component {
        id: cbtn
        Button {
            id: tbtn
            Layout.leftMargin: 8
            Layout.fillHeight: true
            property alias imageSource: img.source
            background: Image{
                id: img
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }
        }
    }

    MoDir {
        id: dir
    }

    MoInputDialog {
        id: dlgInput
    }

    contentItem: ColumnLayout{
        id: content
        spacing: 0
        RowLayout{
            Item {
                width: btnMenu.width
            }
            Label{
                Layout.fillWidth: true
                padding: 5
                text: title
                font.bold: true
                horizontalAlignment: Qt.AlignHCenter
            }
            Button{
                id: btnMenu
                flat: true
                Layout.preferredWidth: 48
                Layout.preferredHeight: 32

                background: Item {
                    Image{
                        anchors.centerIn: parent
                        sourceSize.width: 18
                        sourceSize.height: 18
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/woterm/resource/skin/menu3.png"
                    }
                }
                onClicked: {
                    submenu.popup(btnMenu, 0, height)
                }

                Menu {
                    id: submenu
                    modal: true
                    MenuItem {
                        text: qsTr("Reflesh")
                        checkable: false
                        onClicked: {
                            thiz.setPath(thiz.folder);
                        }
                    }
                    MenuItem {
                        text: qsTr("Create directory")
                        checkable: false
                        onClicked: {
                            dlgInput.text(qsTr("Create directory"), qsTr("Please input a new directory.")).then(function(txt){
                                console.log("create directory", txt)
                                if(!dir.makePath(thiz.folder + "/" + txt)) {
                                    gMessageBox.information(qsTr("Error"), qsTr("Failed to create directory."))
                                    return
                                }
                                thiz.setPath(thiz.folder);
                            });
                        }
                    }

                    MenuItem {
                        id: btnMultSelect
                        visible: multipleSelect
                        height: visible ? implicitHeight : 0
                        text: qsTr("Multiple selection")
                        checkable: true
                        onClicked: {
                            thiz.clearSelection()
                        }
                    }
                    MenuItem {
                        visible: multipleSelect
                        height: visible ? implicitHeight : 0
                        enabled: btnMultSelect.checked
                        text: qsTr("Clear selection")
                        checkable: false
                        onClicked: {
                            thiz.selectAll()
                        }
                    }
                    MenuItem {
                        visible: multipleSelect
                        height: visible ? implicitHeight : 0
                        enabled: btnMultSelect.checked
                        text: qsTr("Select all")
                        checkable: false
                        onClicked: {
                            for(var i = 0; i < myModel.count; i++) {
                                let item = myModel.get(i);
                                item["selected"] = true
                                myModel.set(i, item)
                            }
                        }
                    }
                }
            }
        }
        RowLayout{
            Layout.fillWidth: true
            Label{
                Layout.fillWidth: true
                padding: 5
                text: label
                wrapMode: Label.WrapAnywhere
            }
            Label{
                text: nameFilters.join(",")
            }
        }
        Rectangle{
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "gray"
        }
        Label{
            Layout.fillWidth: true
            padding: 5
            text: thiz.folder
            elide: Text.ElideMiddle
        }

        Rectangle{
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "gray"
        }
        ListView{
            id: listView
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true

            model: ListModel {
                id: myModel
            }

            delegate: ItemDelegate {
                leftPadding: 36
                width: listView.width
                height: 50
                ColumnLayout{
                    anchors.fill: parent
                    Rectangle{
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "gray"
                        visible:index === 0 ? true : false
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.leftMargin: 6
                        Layout.rightMargin: 6
                        Image {
                            Layout.fillHeight: true
                            source: model.isDir ? "qrc:/woterm/resource/skin/dirs.png" : "qrc:/woterm/resource/skin/file.png"
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 24
                            sourceSize.height: 24
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter
                        }
                        ColumnLayout{
                            Layout.fillWidth: true
                            spacing: 0
                            Item {
                                Layout.fillHeight: true
                            }

                            Text {
                                id: name
                                Layout.fillWidth: true
                                text: model.fileName
                                font.pointSize: 13
                            }
                            Text {
                                visible: !(model.fileName === "." || model.fileName === "..")
                                text: model.shortString
                                font.pointSize: 8
                            }
                            Item {
                                Layout.fillHeight: true
                            }
                        }
                        Loader{
                            visible: !!(model.isDir && listView.currentIndex == index && model.fileName !== ".")
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignVCenter
                            sourceComponent: cbtn
                            onLoaded: {
                                item.imageSource = "qrc:/woterm/resource/skin/right2.png"
                                item.clicked.connect(onClicked)
                            }
                            function onClicked() {
                                console.log("folder", model.filePath)
                                if(model.fileName === ".") {
                                    return;
                                }
                                thiz.setPath(model.filePath)
                            }
                        }
                        CheckBox {
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignVCenter

                            visible: btnMultSelect.checked && (!(model.fileName === "." || model.fileName === "..") && (typeof(folderSelect) === "undefined" || folderSelect === model.isDir))
                            checked: model.selected
                            checkable: true
                            onClicked: {
                                model.selected = checked
                            }
                        }
                    }
                    Rectangle{
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color:"gray"
                    }
                }

                highlighted: ListView.isCurrentItem
                onClicked: {
                    if (listView.currentIndex != index) {
                        listView.currentIndex = index
                    }
                }
            }
            ScrollIndicator.vertical: ScrollIndicator { }
        }
        Rectangle{
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "gray"
        }
        RowLayout{
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            Item{
                Layout.fillWidth: true
            }
            Loader{
                id: back
                sourceComponent: cbtn
                enabled: thiz.folder !== "/"
                onLoaded: {
                    item.text = qsTr("Back")
                    item.clicked.connect(onClicked)
                }
                function onClicked() {
                    if(thiz.folder === "/") {
                        return;
                    }
                    thiz.setPath(thiz.folder + "/..");
                }
            }
            Loader{
                id: enter
                sourceComponent: cbtn
                enabled: {
                    var item = myModel.get(listView.currentIndex)
                    if(item && item.isDir) {
                        return true
                    }
                    return false
                }

                onLoaded: {
                    item.text = qsTr("Enter")
                    item.clicked.connect(onClicked)
                }
                function onClicked() {
                    if(listView.currentIndex < 0) {
                        return
                    }
                    var item = myModel.get(listView.currentIndex)
                    if(item && item.isDir) {
                        thiz.setPath(item.filePath)
                    }
                }
            }
            Loader{
                id: select
                sourceComponent: cbtn
                enabled: {
                    var fi = myModel.get(listView.currentIndex)
                    if(btnMultSelect.checked) {
                        return true
                    }

                    if(fi && (thiz.folderSelect === fi.isDir || typeof(folderSelect) === "undefined")) {
                        return true
                    }
                    return false
                }

                onLoaded: {
                    item.text = qsTr("Select")
                    item.clicked.connect(onClicked)
                }
                function onClicked() {
                    if(btnMultSelect.checked) {
                        var files = []
                        var fileInfos = []
                        for(var i = 0; i < myModel.count; i++) {
                            var fi = myModel.get(i)
                            if(fi.selected &&(typeof(folderSelect) === "undefined" || folderSelect === fi.isDir)) {
                                if(fi.fileName === "." || fi.fileName === "..") {
                                    continue
                                }

                                fileInfos.push(fi)
                                files.push(fi.fileName)
                            }
                        }
                        if(files.length <= 0) {
                            gMessageBox.information(qsTr("Selection error"), qsTr("No selection"))
                            return
                        }
                        thiz.filesArrived(thiz.folder, files)
                        thiz.fileInfosArrived(thiz.folder, fileInfos)
                        thiz.close()
                        return
                    }

                    if(listView.currentIndex < 0) {
                        return
                    }

                    var fi = myModel.get(listView.currentIndex)
                    if(typeof(folderSelect) === "undefined" || folderSelect === fi.isDir) {
                        if(multipleSelect){
                            thiz.filesArrived(thiz.folder, [fi.fileName])
                            thiz.fileInfosArrived(thiz.folder, [fi])
                        }else{
                            thiz.fileArrived(thiz.folder, fi.fileName)
                            thiz.fileInfoArrived(thiz.folder, fi)
                        }
                    }
                    thiz.close()
                }
            }
            Loader{
                id: cancel
                sourceComponent: cbtn
                onLoaded: {
                    item.text = qsTr("Cancel")
                    item.clicked.connect(onClicked)
                }
                function onClicked() {
                    thiz.close()
                }
            }
            Item{
                Layout.fillWidth: true
            }
        }
    }
}
