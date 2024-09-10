import QtQuick 2.7
import QtQuick.Controls 2.3

Item
{
    width: 485
    height: 300
    property int itemCount: 5

    // Close Button 'X'
    Rectangle
    {
        width: 40
        height: 40
        radius: 20 // Makes it a circle
        color: "transparent" // Initially transparent
        // border.color: "gray"
        // border.width: 1
        z: 999 // Ensure it stays on top
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 2

        Text
        {
            id: closeText
            text: "X"
            anchors.centerIn: parent
            font.pointSize: 16
            font.weight: Font.Normal // Default weight
        }

        MouseArea
        {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor // Change cursor to hand

            onEntered:
            {
                parent.color = "#f8f8f8" // Change color on hover
                closeText.font.weight = Font.Bold // Make the 'X' bold on hover
            }
            onExited:
            {
                parent.color = "transparent" // Revert back to transparent
                closeText.font.weight = Font.Normal // Revert back to normal weight
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

            Column
            {
                anchors.centerIn: parent

                Text
                {
                    text: "Add New Connection"
                    font.bold: true
                    font.capitalization: Font.SmallCaps
                    font.pointSize: 24 // Title text size
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Adjusted small paragraph text
                Text
                {
                    text: "Choose a connection type to proceed. Each connection allows you to interact with different services."
                    font.pointSize: 12 // Smaller text size
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    width: parent.width * 0.70 // Slightly wider before wrapping
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        Rectangle
        {
            id: chatConnection
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
            id: messageBoardConnection
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
            id: redditConnection
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
            id: browserConnection
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
