import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

ColumnLayout {
    property var crypts: ["AES-CBC-256","AES-CTR-256","AES-GCM-256","DES-CBC","DES-ECB","DES-OFB64","RC4","Blowfish"]
    property string cryptType: "Blowfish"
    property alias cryptPwd: passInput.text

    onCryptTypeChanged: {
        if(cryptType !== typeInput.currentText) {
            typeInput.currentIndex = crypts.indexOf(cryptType)
        }
    }

    RowLayout{
        Layout.fillWidth: true
        Label{
            text:qsTr("Encryption type:")
        }
        ComboBox {
            id: typeInput
            Layout.fillWidth: true
            model: crypts
            onCurrentTextChanged: {
                cryptType = currentText
            }
        }
    }
    RowLayout{
        Layout.fillWidth: true
        Label{
            text:qsTr("Encryption password")
        }
        TextField {
            id: passInput
            Layout.fillWidth: true
            text: cryptPwd
        }
    }
}
