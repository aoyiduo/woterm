import QtQml 2.2
import QtQuick 2.12
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2


Rectangle {
    id: thiz
    color: Playbook.backgroundColor
    Component.onCompleted: {
        Playbook.openTerminal()
    }

    function resizeTerminal() {
        Playbook.adjustTerminal()
    }

    TabView {
        id: tabView
        anchors.fill: parent
        Component.onCompleted: {
            Qt.callLater(thiz.resizeTerminal)
        }

        onCurrentIndexChanged: {
            Qt.callLater(thiz.resizeTerminal)
        }

        Tab {
            title: "LocalDir"
            TestLocalDir {

            }
        }

        Tab {
            title: "LocalFile"
            TestLocalFile {

            }
        }
        Tab {
            title: "LocalCommand"
            TestLocalCommand {

            }
        }
        Tab {
            title: "RemoteCommand"
            TestRemoteCommand {

            }
        }
        Tab {
            title: "FileContentCommand"
            TestFileContentCommand {

            }
        }
        Tab {
            title: "FileListCommand"
            TestFileListCommand {

            }
        }
        Tab {
            title: "FileTransferCommand"
            TestFileTransferCommand {

            }
        }
        Tab {
            title: "BigScriptCommand"
            TestBigScriptCommand {

            }
        }
        Tab {
            title: "HttpCommand"
            TestHttpCommand {

            }
        }

        Tab {
            title: "CustomCommand"
            TestCustomCommand {

            }
        }
    }
}
