import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls.Material 2.5
import com.vovkvv.pocketprocessing 1.0
import PluginBins 1.0

Window {
    function stateText(state) {
            switch(state) {
                case 1: return "coding ...";
                case 2: return "decoding ...";
                default: return "";
            }
        }

    function resultText(result) {
            switch(result) {
                case 0: return `${result} (success)`;

                case -1: return "";
                case -2: return "Cant Load Input Image";
                case -3: return "Cant Open Barch As File Stream";
                case -4: return "Barch Is Not Exist";
                case -5: return "Barch Header Too Small";
                case -6: return "Barch Row Index Too Small";
                case -7: return "Cant Load Barch Row Index";
                case -8: return "General File Stream Error";

                case 1: return "Low memory";
                case 2: return "Cant create output file";
                case 3: return "Out of range";
                case 4: return "Exception occured";
                case 5: return "Cancel";
                case 6: return "Ba format";

                default: return `${result} (error)`;
            }
        }


    id: root
    objectName: "PocketMainWindowQML"
    visible: true
    color: "white"
    width: 640
    height: 480

    Component.onCompleted: {
        processor.initializeFiles()
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: fileListModel
        clip: true
        highlight: Rectangle { color: "lightblue"; radius: 5 }
        interactive: contentHeight > height

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: Item {
            id: delegateItem
            width: listView.width
            height: 40

            Rectangle{
                width: parent.width
                height: parent.height
                color: listView.currentIndex === index ? "lightblue" : "white"

                RowLayout {
                    anchors.fill: parent
                    spacing: 10

                    Text {
                        Layout.fillWidth: true
                        Layout.maximumWidth: parent.width - stateTextId.width - 20
                        text: "   " + model.display
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                listView.currentIndex = index
                                processor.onItemSelected(index)
                            }
                        }

                     }

                    Column {
                        visible: model.state !== 0
                        Button {
                            id: cancelButton
                            text: "Cancel"
                            background: Rectangle {
                                color: "red"
                                radius: 2
                            }
                            Material.elevation: cancelButton.pressed ? 6 : 2
                            opacity: cancelButton.pressed ? 0.8 : 1.0
                            scale: cancelButton.pressed ? 0.95 : 1.0
                            onClicked: processor.cancelAction(model.index)
                        }
                    }

                    Text {
                        id: stateTextId
                        text: stateText(model.state)
                        Layout.preferredWidth: Math.max(stateText.contentWidth, 100)
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        clip: true
                        color: model.state === 1? "green" : "blue"
                    }

                     Text {
                        text: resultText(model.resultCode)
                     }

                     Text {
                        text: "     "
                     }
                }

            }
        }
    }
}
