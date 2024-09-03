import QtQuick 2.7
import QtQuick.Controls 2.3

// https://stackoverflow.com/questions/47404304/qml-how-to-custom-a-component-and-use-it-in-same-file

Item
{
    width: 485
    height: 300
    property int itemCount: 5

//    Component { id: btn; Rectangle { width : 100; height : 100; color : "red" } }

    // Loader
    // {
    //     id: dialogLoader
    //     // sourceComponent: column
    // }

    Loader
    {
        id: overlayLoader
        anchors.fill: parent
        onLoaded:
        {
            overlayLoader.item.visible = true
        }
    }

    Column {
        anchors.fill: parent

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                // padding: 10
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
                onClicked:
                {
                    console.log("New Chat Connection")
                    newConnectionPage.onOptionSelected(1);
                }
            }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Message Board" }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered: parent.color = "yellow"
                onExited: parent.color = "white"
                onClicked:
                {
                    console.log("New Message Board Connection")
                    newConnectionPage.onOptionSelected(2);
                }
            }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Reddit" }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered: parent.color = "yellow"
                onExited: parent.color = "white"
                onClicked:
                {
                    console.log("New Reddit Connection")
                    newConnectionPage.onOptionSelected(3);
                }
            }
        }

        Rectangle {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text { text: "Browser" }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered: parent.color = "yellow"
                onExited: parent.color = "white"
                onClicked:
                {
                    console.log("New Browser Connection")
                    newConnectionPage.onOptionSelected(4);
                }
            }
        }
    }
}
