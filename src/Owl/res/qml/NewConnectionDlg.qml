import QtQuick 2.7
import QtQuick.Controls 2.3

Item
{
    width: 485
    height: 300
    property int itemCount: 5

    // Loader
    // {
    //     id: overlayLoader
    //     anchors.fill: parent
    //     onLoaded
    //     {
    //         overlayLoader.item.visible = true
    //     }
    // }

    // Close Button 'X'
    Rectangle
    {
        width: 40
        height: 40
        radius: 20 // Makes it a circle
        color: "transparent" // Initially transparent
        border.color: "gray"
        border.width: 1
        z: 999 // Ensure it stays on top
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 2

        Text
        {
            text: "X"
            anchors.centerIn: parent
            font.pointSize: 16
        }

        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true

            onEntered:
            {
                parent.color = "#ffcccc" // Change color on hover
            }
            onExited:
            {
                parent.color = "transparent" // Revert back to transparent
            }
            onClicked:
            {
                newConnectionPage.onCancel()
            }
        }
    }

    Column
    {
        anchors.fill: parent

        Rectangle
        {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                text: "Add New Connection"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                font.capitalization: Font.SmallCaps
                font.pointSize: 20
            }
        }

        Rectangle
        {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                text: "New Chat Connection"
            }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered:
                {
                    parent.color = "yellow"
                }
                onExited:
                {
                    parent.color = "white"
                }
                onClicked:
                {
                    console.log("New Chat Connection")
                    newConnectionPage.onOptionSelected(1);
                }
            }
        }

        Rectangle
        {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                text: "Message Board"
            }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered:
                {
                    parent.color = "yellow"
                }
                onExited:
                {
                    parent.color = "white"
                }
                onClicked:
                {
                    console.log("New Message Board Connection")
                    newConnectionPage.onOptionSelected(2);
                }
            }
        }

        Rectangle
        {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                text: "Reddit"
            }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered:
                {
                    parent.color = "yellow"
                }
                onExited:
                {
                    parent.color = "white"
                }
                onClicked:
                {
                    console.log("New Reddit Connection")
                    newConnectionPage.onOptionSelected(3);
                }
            }
        }

        Rectangle
        {
            border.width: 1
            width: parent.width
            height: parent.height / itemCount
            Text
            {
                text: "Browser"
            }
            MouseArea
            {
                hoverEnabled: true
                anchors.fill: parent
                onEntered:
                {
                    parent.color = "yellow"
                }
                onExited:
                {
                    parent.color = "white"
                }
                onClicked:
                {
                    console.log("New Browser Connection")
                    newConnectionPage.onOptionSelected(4);
                }
            }
        }
    }
}
