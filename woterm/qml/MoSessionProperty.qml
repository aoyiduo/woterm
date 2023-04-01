import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

Page {
    id: thiz
    property string hostName: ""
    property var hostInfo: hostName === "" ? gSshConf.qmlDefault() : gSshConf.qmlFind(hostName)
    signal finished()

    MessageDialog {
        title: qsTr("Infomation")
        id: msgBox
    }

    MoSimpleChoose{
        id: identifyChooser
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width - 20
        height: thiz.height - 20
        onResult: {
            identifyFile.text = name
        }
    }
    MoSimpleChoose{
        id: jumpChooser
        x: (thiz.width - width) / 2
        y: (thiz.height - height) / 2
        width: thiz.width - 20
        height: thiz.height - 20
        onResult: {
            proxyJump.text = name
        }
    }


    Component.onCompleted: {
        var hi = thiz.hostInfo
        var ta = {
            SshWithSftp: 0,
            SftpOnly: 1,
            Telnet: 2,
            RLogin: 3,
            Mstsc: 4,
            Vnc: 5
        }
        type.currentIndex = ta[hi.type]
        type.resetLayout()
        loginType.currentIndex = hi.identify !== "" ? 1 : 0
        password.text = hi.password
        name.text = hi.name
        host.text = hi.host
        port.text = hi.port
        loginName.text = hi.user
        identifyFile.text = hi.identify
        memo.text = hi.memo
        script.text = hi.script
        proxyJump.text = hi.proxyJump
    }

    header: MoToolBar {
        source: "qrc:/woterm/resource/skin/left2.png"
        title: qsTr("Session property")
        onLeftClicked: {
            finished()
        }
    }

    Flickable{
        anchors.fill: parent
        contentHeight: layout.implicitHeight
        ColumnLayout{
            id: layout
            anchors.fill: parent
            anchors.margins: 5
            RowLayout{
                Layout.fillWidth: true
                Label {
                    text:qsTr("Type:")
                }
                ComboBox {
                    id: type
                    Layout.fillWidth: true
                    model: ListModel {
                        id: model
                        ListElement { text: "SshWithSftp" }
                        ListElement { text: "SftpOnly" }
                        ListElement { text: "Telnet" }
                        ListElement { text: "RLogin" }
                        ListElement { text: "Rdp/Mstsc" }
                        ListElement { text: "Vnc" }
                    }

                    function isDefaultPort() {
                        return port.text === "22"
                                || port.text === "23"
                                || port.text === "513"
                                || port.text === "3389"
                                || port.text === "5901"
                                || port.text==="";
                    }

                    onCurrentTextChanged: {
                        resetLayout();
                    }

                    function resetLayout() {
                        if(currentText == "SshWithSftp") {
                            loLoginType.visible = true
                            loProxyJump.visible = true
                            loScript.visible = true
                            if(loginType.currentText === qsTr("Password")) {
                                loPassword.visible = true
                                loIdentify.visible = false
                            }else{
                                loPassword.visible = false
                                loIdentify.visible = true
                            }
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "22"
                            }
                        }else if(currentText == "SftpOnly") {
                            loLoginType.visible = true
                            loProxyJump.visible = true
                            loScript.visible = false
                            if(loginType.currentText === qsTr("Password")) {
                                loPassword.visible = true
                                loIdentify.visible = false
                            }else{
                                loPassword.visible = false
                                loIdentify.visible = true
                            }
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "22"
                            }
                        }else if(currentText == "Telnet") {
                            loLoginType.visible = false
                            loProxyJump.visible = false
                            loScript.visible = false
                            loPassword.visible = true
                            loIdentify.visible = false
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "23"
                            }
                        }else if(currentText == "RLogin") {
                            loLoginType.visible = false
                            loProxyJump.visible = false
                            loScript.visible = false
                            loPassword.visible = true
                            loIdentify.visible = false
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "513"
                            }
                        }else if(currentText == "Rdp/Mstsc") {
                            loLoginType.visible = false
                            loProxyJump.visible = false
                            loScript.visible = false
                            loPassword.visible = true
                            loIdentify.visible = false
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "3389"
                            }
                        }else {
                            loLoginType.visible = false
                            loProxyJump.visible = false
                            loScript.visible = false
                            loPassword.visible = true
                            loIdentify.visible = false
                            loLoginName.visible = false                            
                            if(hostName === "" && isDefaultPort()) {
                                port.text = "5091"
                            }
                        }
                    }
                }
            }
            RowLayout{
                Layout.fillWidth: true
                Label{
                    text:qsTr("Name:")
                }
                TextField {
                    id: name
                    Layout.fillWidth: true
                }
            }
            RowLayout{
                Layout.fillWidth: true
                Label{
                    text:qsTr("Host:")
                }
                TextField {
                    id: host
                    Layout.fillWidth: true
                }
            }
            RowLayout{
                Layout.fillWidth: true
                Label{
                    text:qsTr("Port:")
                }
                TextField {
                    id: port
                    Layout.fillWidth: true
                    validator: IntValidator{bottom: 10; top: 65534;}
                }
                RowLayout{
                    visible: {
                        var vp_hit = port.text
                        var vp = "22"
                        if(type.currentText == "Telnet") {
                            vp = "23"
                        }else if(type.currentText == "RLogin"){
                            vp = "513"
                        }else if(type.currentText == "Rdp/Mstsc") {
                            vp = "3389"
                        }else if(type.currentText == "Vnc") {
                            vp = "5901"
                        }

                        portDef.text = vp
                        console.log("visible", vp_hit, vp, vp_hit !== vp)
                        return vp_hit !== vp
                    }

                    Label{
                        text: qsTr("default:")
                        color: "red"
                    }
                    Label{
                        id:portDef
                        color: "red"
                    }
                }


            }
            RowLayout{
                id: loLoginName
                Layout.fillWidth: true
                Label{
                    text:qsTr("LoginName:")
                }
                TextField {
                    id: loginName
                    Layout.fillWidth: true
                    text: "root"
                }
            }
            RowLayout{
                id:loLoginType
                Layout.fillWidth: true
                Label{
                    text:qsTr("LoginType:")
                }
                ComboBox {
                    id: loginType
                    Layout.fillWidth: true
                    model: ListModel {
                        ListElement { text: qsTr("Password") }
                        ListElement { text: qsTr("Identify") }
                    }
                    onCurrentTextChanged: {
                        if(currentText === qsTr("Password")) {
                            loPassword.visible = true
                            loIdentify.visible = false
                        }else{
                            loPassword.visible = false
                            loIdentify.visible = true
                        }
                    }
                }
            }
            RowLayout{
                id: loPassword
                Layout.fillWidth: true
                Label{
                    text:qsTr("Password:")
                }
                TextField {
                    id: password
                    Layout.fillWidth: true
                    echoMode: TextInput.PasswordEchoOnEdit
                }
            }
            RowLayout{
                id:loIdentify
                Layout.fillWidth: true
                Label{
                    text:qsTr("IdentifyFile:")
                }
                TextField {
                    Layout.fillWidth: true
                    id: identifyFile
                    readOnly: true
                    Button{
                        x: parent.width - 30
                        y: 0
                        width: 30
                        height: parent.height
                        Image{
                            anchors.centerIn: parent
                            source: "qrc:/woterm/resource/skin/tabclose.png"
                            sourceSize.width: 16
                            sourceSize.height: 16
                        }
                        onClicked: {
                            identifyFile.clear()
                        }
                    }
                }
                Button{
                    text: qsTr("...")
                    onClicked: {
                        var all = gIdentify.qmlFileNames()
                        identifyChooser.title = qsTr("IdentifyList")
                        identifyChooser.updateList(all)
                        identifyChooser.open()
                    }
                }
            }
            RowLayout{
                id:loProxyJump
                Layout.fillWidth: true
                Label{
                    text:qsTr("ProxyJump:")
                }
                TextField {
                    id: proxyJump
                    Layout.fillWidth: true
                    readOnly: true
                    text: thiz.hostInfo.proxyJump

                    Button{
                        x: parent.width - width
                        y: (parent.height - height) / 2
                        width: parent.height
                        height: parent.height
                        Image{
                            anchors.centerIn: parent
                            source: "qrc:/woterm/resource/skin/tabclose.png"
                            sourceSize.width: 16
                            sourceSize.height: 16
                        }
                        onClicked: {
                            proxyJump.clear()
                        }
                    }
                }

                Button{
                    text: qsTr("...")
                    onClicked: {
                        var all = gSshConf.qmlProxyJumpers()
                        jumpChooser.title = qsTr("ProxyJump")
                        jumpChooser.updateList(all)
                        jumpChooser.open()
                    }
                }
            }
            RowLayout{
                id:loScript
                Layout.fillWidth: true
                Label{
                    text:qsTr("Command:")
                }
                TextField {
                    id: script
                    text: thiz.hostInfo.script
                    Layout.fillWidth: true
                }
            }
            RowLayout{
                id:loGroup
                Layout.fillWidth: true
                Label{
                    text:qsTr("Group:")
                }
                ComboBox {
                    id: group
                    Layout.fillWidth: true

                    model: ListModel {
                        id: proxyModel
                    }
                    onCurrentTextChanged: {

                    }

                    Component.onCompleted: {
                        var all = gSshConf.qmlGroupNameList();
                        var groupName = thiz.hostInfo.group
                        console.log("groupName:"+groupName)
                        group.popup.modal = true
                        for(var id in all){
                            var name = all[id];
                            proxyModel.append({name: name})
                            if(name === groupName) {
                                group.currentIndex = id
                            }
                        }
                    }
                }
            }
            Label{
                text:qsTr("Memo")
            }
            Frame{
                Layout.fillWidth: true
                TextEdit {
                    anchors.fill: parent
                    id: memo
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Item{
                    Layout.fillWidth: true
                }
                Button{
                    text: qsTr("Save")
                    onClicked: {
                        var v = {
                            type: type.currentText,
                            name: name.text,
                            host: host.text,
                            port: port.text,
                            user: loginName.text,
                            password: loginType.currentText === qsTr("Password") ? password.text : "",
                            identify: loginType.currentText === qsTr("Password") ? "" : identifyFile.text,
                            memo: memo.text,
                            script: script.text,
                            proxyJump: proxyJump.text,
                            group: group.currentText,
                        }
                        if(proxyJump.text === name.text && type.currentText === "SshWithSftp") {
                            msgBox.text = qsTr("The name is the same with proxyJump")
                            msgBox.open()
                            return
                        }

                        if(name.text === "" && type.currentText !== "Vnc") {
                            msgBox.text = qsTr("The name is empty.")
                            msgBox.open()
                            return
                        }
                        if(host.text === "") {
                            msgBox.text = qsTr("The host is empty.")
                            msgBox.open()
                            return
                        }
                        if(loginName.text === "") {
                            msgBox.text = qsTr("The login name is empty.")
                            msgBox.open()
                            return
                        }

                        if(hostName === "" && gSshConf.exists(name.text)) {
                            msgBox.text = qsTr("The name had been exist, change it to another one.")
                            msgBox.open()
                            return
                        }

                        gSshConf.qmlModifyOrAppend(v)
                        finished()
                    }
                }
                Item{
                    Layout.fillWidth: true
                }
            }

            Item{
                Layout.fillHeight: true
            }
        }
        ScrollIndicator.vertical: ScrollIndicator { }
    }
}
