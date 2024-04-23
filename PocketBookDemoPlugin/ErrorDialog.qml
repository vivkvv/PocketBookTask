import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    id: errorDialog
    objectName: "errorDialog"
    width: 300
    height: 200
    modality: Qt.ApplicationModal
    visible: true

    property string errorMessage: ""

    Text {
        text: errorMessage
        anchors.centerIn: parent
    }

    OkButton {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: errorDialog.close()
    }
}
