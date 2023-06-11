import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.11
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import LocalDir 1.0
import LocalCommand 1.0
import RemoteCommand 1.0
import FileTransferCommand 1.0
import FileListCommand 1.0
import FileContentCommand 1.0

Item {
    id: thiz
    property string hosts
    property string fileLocal
    property string fileRemote
    property string enDesc
    property string zhDesc

    signal previousArrived()


    function isVersion(frag) {
        let verAlphas="v0123456789.";
        let cnt = 0;
        for(let i in frag) {
            let c = frag[i]
            if(c === '.') {
                cnt++;
                continue
            }

            if(verAlphas.indexOf(c) < 0) {
                return false;
            }
        }
        return cnt === 2;
    }

    function extractVersion(fileName) {
        let frags = fileName.split("-")

        for(let i in frags) {
            let frag = frags[i];
            console.log("frag", frag)
            if(frag.length <= 0 || frag[0] !== 'v') {
                continue;
            }
            if(isVersion(frag)) {
                return frag
            }
        }
        return ""
    }

    function versionToInt(ver) {
        if(ver === "") {
            return 0;
        }
        if(ver[0] === 'v') {
            ver = ver.substring(1)
        }
        let frags = ver.split('.')
        let total = 0;
        for(let i = 0; i < frags.length; i++) {
            let frag = frags[i];
            total *= 1000;
            total += parseInt(frag)
        }
        return total
    }

    function maxVersion(ver, verHit) {
        let tver = versionToInt(ver)
        let tverHit = versionToInt(verHit)
        console.log("maxVersion", ver, tver, verHit, tverHit)
        return tver > tverHit ? ver : verHit;
    }

    LocalDir {
        id: dir
    }

    LocalCommand {
        id: local
        onCommandStart: {
            output.append("command start: "+command)
        }

        onCommandFinished:  {
            output.append("command finish: exit code:"+lastExitCode)
        }

        onDataArrived: {
            output.append(data)
        }
    }

    RemoteCommand {
        id: remote
        hosts: thiz.hosts.split(',')
        property string result
        onSubcommandStart: {            
            output.append(host+": command start: "+command)
            result = ""
        }

        onSubcommandFinished: {
            output.append(host+": command finish: exit code: "+code)
        }

        onDataArrived: {
            result += data
            output.append(data)
        }
        onErrorArrived: {
            output.append(error)
        }
    }

    FileTransferCommand {
        id: transfer
        hosts: thiz.hosts.split(',')

        onSubcommandStart: {
            output.append(host+": transfer start: " + fileLocal + (fileDownload ? "<--" : "-->") + fileRemote)
        }

        onSubcommandFinished: {
            output.append(host+": transfer finish: code:"+code)
        }

        onProgressArrived: {
            output.append(host+" transfer progress:"+v)
            myPogress.iProgress = 10 + v * 80 / 100;
        }

        onErrorArrived: {
            output.append(error)
        }
    }

    FileListCommand {
        id: fileList
        hosts: thiz.hosts.split(',')
    }

    FileContentCommand {
        id: fileContent
        hosts: thiz.hosts.split(',')
    }


    ColumnLayout {
        anchors.fill: parent
        ColumnLayout {
            Layout.fillWidth: true
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                color: Playbook.backgroundColor
                border.color: Playbook.textColor
                Rectangle {
                    id: myPogress
                    property double iProgress: 0
                    x: 2
                    y: 2
                    width: parent.width * iProgress / 100
                    height: parent.height - 4
                    color: Playbook.highlightColor
                }
            }
        }
        RowLayout {
            Button {
                id: btnPrevious
                visible: btnStart.visible
                text: "Previous"
                onClicked: {
                    thiz.previousArrived()
                }
            }
            Button {
                id: btnStart
                text: "Start"
                onClicked: {
                    output.text = ""
                    btnStart.visible = false
                    asyncGenerator(function *(){
                        myPogress.iProgress = 0;
                        //----
                        output.log("Ready to check if the local file is valid.");
                        let fileLocal = thiz.fileLocal
                        if(!dir.exist(fileLocal)) {
                            output.log("not exist file.", fileLocal);
                            return
                        }

                        myPogress.iProgress = 5;
                        //-----
                        output.log("Ready to check if the remote directory has been created.");
                        let path = thiz.fileRemote
                        yield remoteCommand(remote, "ls -a " + path).catch(function() {
                            output.log("ready to make path:", path);
                            yield remoteCommand(remote, "mkdir -p " + path)
                        })
                        myPogress.iProgress = 10;
                        //----
                        let pathRemote = thiz.fileRemote
                        let fileRemote = pathRemote + "/tmp.tar.gz"
                        output.log("ready to upload file", fileLocal, "-->", fileRemote);
                        yield fileTransferCommand(transfer, fileLocal, fileRemote, false, false)

                        myPogress.iProgress = 90;
                        //----
                        let cmd = "cd "+pathRemote + " && mkdir -p tmp && tar -xf ./tmp.tar.gz"  + " -C tmp && ls ./tmp";
                        output.log("decompress command:", cmd)
                        yield remoteCommand(remote, cmd)
                        yield fileListCommand(fileList, pathRemote+"/tmp");
                        let filesUpload = fileList.fileNames;
                        myPogress.iProgress = 91;
                        //---
                        output.log("check if the window directory has been created or else create it.")
                        yield remoteCommand(remote, "cd "+pathRemote + "/window || mkdir -p " + pathRemote + "/window")

                        myPogress.iProgress = 92;
                        //---
                        output.log("check if the linux directory has been created or else create it.")
                        yield remoteCommand(remote, "cd "+pathRemote + "/linux || mkdir -p " + pathRemote + "/linux")

                        myPogress.iProgress = 93;
                        //---
                        output.log("check if the macosx directory has been created or else create it.")
                        yield remoteCommand(remote, "cd "+pathRemote + "/macosx || mkdir -p " + pathRemote + "/macosx")

                        myPogress.iProgress = 94;
                        //---
                        output.log("check if the android directory has been created or else create it.")
                        yield remoteCommand(remote, "cd "+pathRemote + "/android || mkdir -p " + pathRemote + "/android")

                        myPogress.iProgress = 95;
                        //---
                        let mver = "";
                        let ver = "";
                        for(let id in filesUpload) {
                            let fileName = filesUpload[id];
                            let verHit = thiz.extractVersion(fileName);
                            console.log("extract version", verHit)
                            if(fileName.indexOf("window") > 0) {
                                yield remoteCommand(remote, "cd "+pathRemote + "/tmp && mv " + fileName + " " + pathRemote+"/window")
                                if(fileName.indexOf(".exe") > 0) {
                                    yield remoteCommand(remote, "cd "+pathRemote + "/window && ln -sf " + fileName + " woterm-windows-latest.exe")
                                }else{
                                    yield remoteCommand(remote, "cd "+pathRemote + "/window && ln -sf " + fileName + " woterm-windows-latest.tar.gz")
                                }
                                ver = thiz.maxVersion(ver, verHit)
                            }else if(fileName.indexOf("linux") > 0) {
                                yield remoteCommand(remote, "cd "+pathRemote + "/tmp && mv " + fileName + " " + pathRemote+"/linux")
                                if(fileName.indexOf(".run") > 0) {
                                    yield remoteCommand(remote, "cd "+pathRemote + "/linux && ln -sf " + fileName + " woterm-linux-x86_64-latest.run")
                                }else{
                                    yield remoteCommand(remote, "cd "+pathRemote + "/linux && ln -sf " + fileName + " woterm-linux-x86_64-latest.tar.gz")
                                }
                                ver = thiz.maxVersion(ver, verHit)
                            }else if(fileName.indexOf("macosx") > 0) {
                                yield remoteCommand(remote, "cd "+pathRemote + "/tmp && mv " + fileName + " " + pathRemote+"/macosx")
                                yield remoteCommand(remote, "cd "+pathRemote + "/macosx && ln -sf " + fileName + " woterm-macosx-x86_64-latest.tar.gz")
                                ver = thiz.maxVersion(ver, verHit)
                            }else if(fileName.indexOf("android") > 0) {
                                yield remoteCommand(remote, "cd "+pathRemote + "/tmp && mv " + fileName + " " + pathRemote+"/android")
                                yield remoteCommand(remote, "cd "+pathRemote + "/android && ln -sf " + fileName + " woterm-android-armv7-latest.apk")
                                mver = thiz.maxVersion(mver, verHit)
                            }
                        }
                        myPogress.iProgress = 98;
                        yield remoteCommand(remote, "mkdir -p "+pathRemote + "/.desc")
                        let zhDesc = thiz.zhDesc;
                        let enDesc = thiz.enDesc;
                        output.log("desktop version:", ver, "android version:", mver);
                        if(ver !== "") {
                            let zhDescFile = ver+"-zh";
                            let enDescFile = ver+"-en";
                            yield fileContentCommand(fileContent, pathRemote + "/.desc/"+enDescFile, enDesc);
                            yield fileContentCommand(fileContent, pathRemote + "/.desc/"+zhDescFile, zhDesc);
                            yield remoteCommand(remote, "cd " + pathRemote + "/.desc && ln -sf " + enDescFile + " latest-en");
                            yield remoteCommand(remote, "cd " + pathRemote + "/.desc && ln -sf " + zhDescFile + " latest-zh");
                            yield fileContentCommand(fileContent, pathRemote + "/.ver", ver);
                        }
                        if(mver !== "") {
                            yield fileContentCommand(fileContent, pathRemote + "/.mver", mver);
                        }
                        myPogress.iProgress = 100;
                        //----
                        yield remoteCommand(remote, "rm -rf "+pathRemote + "/tmp*")
                        yield remoteCommand(remote, "ls -l -a "+pathRemote + "/*")
                        yield remoteCommand(remote, "ls -l -a "+pathRemote + "/.desc")
                        output.log("finish all.");
                        btnStart.visible = true
                    })();
                }
            }
            Button {
                id: btnAbort
                visible: !btnStart.visible
                text: "Abort"
                onClicked: {
                    remote.abort()
                    transfer.abort()
                    btnStart.visible = true
                }
            }
        }

        MyTextEdit {
            id: output
            Layout.fillHeight: true
            Layout.fillWidth: true
            readyOnly: true

            function log() {
                let msgs = [];
                for(var id in arguments) {
                    var arg = arguments[id];
                    msgs.push(""+arg);
                }
                append(msgs.join(' '))
            }
        }
    }
}
