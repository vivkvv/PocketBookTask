import QtQuick 2.0
import QtQuick.Controls

Button {
    id: okButton
    text: "Close"

    property color defaultColor: "green"
    property color pressedColor: "blue"

    hoverEnabled: false

    background: Rectangle {
        color: okButton.pressed ? okButton.pressedColor : okButton.defaultColor
        radius: 5
    }
}
