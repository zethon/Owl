import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: rectangle
    width: 800
    height: 500

    color: Constants.backgroundColor

    function setFocus()
    {
        console.log("setFocus");
        messageInput.forceActiveFocus();
    }

    TextArea {
        id: textArea
        x: 893
        y: 323
        width: 334
        height: 311
        selectionColor: "#00ff04"
        placeholderText: qsTr("Text Area")
    }

    Rectangle {
        id: buddyList
        width: 250
        height: 200
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "lightblue"

        ItemDelegate {
            id: itemDelegate
            text: qsTr("Item Delegate")
            anchors.fill: parent
        }
    }

    Rectangle {
        id: rectangle1
        height: 30
        border.width: 1
        border.color: "brown"
        anchors.left: buddyList.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        TextInput {
            id: messageInput
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Message"
            anchors.leftMargin: 7
        }
    }
}
