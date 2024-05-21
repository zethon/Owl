import QtQuick 2.7
import QtQuick.Controls 2.3

// https://stackoverflow.com/questions/47404304/qml-how-to-custom-a-component-and-use-it-in-same-file

Item
{
    width: 485
    height: 300
    property int itemCount: 5

//    Component { id: btn; Rectangle { width : 100; height : 100; color : "red" } }

    Column {
        anchors.fill: parent

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                padding: 10
                text: "Add New Connection"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                font.capitalization: Font.SmallCaps
                font.pointSize: 20
            }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "New Chat Connection" }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered: parent.color = "yellow"
                onExited: parent.color = "white"
                onClicked: console.log("corvett")
            }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Message Board" }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Reddit" }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Browser" }
        }
    }
}
