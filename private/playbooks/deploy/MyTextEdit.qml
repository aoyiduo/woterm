import QtQml 2.2
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Rectangle {
    property alias placeholderText: input.placeholderText
    property alias readyOnly: input.readOnly
    property alias text: input.text

    border.color: Playbook.textColor
    color: "transparent"

    function append(txt) {
        input.append(txt)
    }

    ScrollView{
        anchors.fill: parent
        clip: true
        TextArea{
            id: input
            color: Playbook.textColor
            font.pointSize: 12
            selectByMouse: true
            mouseSelectionMode: TextInput.SelectWords
            wrapMode: TextEdit.WrapAnywhere
        }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    }
}
