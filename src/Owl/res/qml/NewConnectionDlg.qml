import QtQuick 2.7
import QtQuick.Controls 2.3

Item
{
    width: 485
    height: 300
    property int itemCount: 5
    property real scaleFactor: 0.65

    // Close Button 'X'
    Rectangle
    {
        width: 40
        height: 40
        radius: 5
        color: "transparent" // Initially transparent
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
                parent.color = "#f0f0f0" // Change color on hover
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
        anchors.margins: 10
        spacing: 10

        Rectangle
        {
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
            width: parent.width * 0.9 // Adjust width to have padding
            height: parent.height / itemCount * scaleFactor // Slightly smaller height for spacing
            radius: 10 // Rounded corners
            color: "white" // Default background color
            border.color: "#dcdcdc" // Light gray border color
            border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter // Center horizontally
            anchors.margins: 8 // Add margins for spacing

            Row // Use a row to align an icon and text
            {
                anchors.fill: parent
                anchors.margins: 10

                // Optional icon (left side)
                Image 
                {
                    source: "/images/owl_64.png"
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Text for the button
                Text
                {
                    text: "New Chat Connection"
                    font.pointSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.leftMargin: 12 // Space between icon and text
                }
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onEntered:
                {
                    parent.color = "#f1f1f1" // Light gray on hover
                    parent.border.color = "#cccccc" // Darken border on hover
                }
                onExited:
                {
                    parent.color = "white" // Revert back to white
                    parent.border.color = "#dcdcdc" // Revert border color
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
            width: parent.width * 0.9 // Adjust width to have padding
            height: parent.height / itemCount * scaleFactor // Slightly smaller height for spacing
            radius: 10 // Rounded corners
            color: "white" // Default background color
            border.color: "#dcdcdc" // Light gray border color
            border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter // Center horizontally
            anchors.margins: 8 // Add margins for spacing

            Row // Use a row to align an icon and text
            {
                anchors.fill: parent
                anchors.margins: 10

                // Optional icon (left side)
                Image 
                {
                    source: "/images/sad.png"
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Text for the button
                Text
                {
                    text: "Message Board Connection"
                    font.pointSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.leftMargin: 12 // Space between icon and text
                }
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onEntered:
                {
                    parent.color = "#f1f1f1" // Light gray on hover
                    parent.border.color = "#cccccc" // Darken border on hover
                }
                onExited:
                {
                    parent.color = "white" // Revert back to white
                    parent.border.color = "#dcdcdc" // Revert border color
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
            width: parent.width * 0.9 // Adjust width to have padding
            height: parent.height / itemCount * scaleFactor // Slightly smaller height for spacing
            radius: 10 // Rounded corners
            color: "white" // Default background color
            border.color: "#dcdcdc" // Light gray border color
            border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter // Center horizontally
            anchors.margins: 8 // Add margins for spacing

            Row // Use a row to align an icon and text
            {
                anchors.fill: parent
                anchors.margins: 10

                // Optional icon (left side)
                Image 
                {
                    source: "/images/quote-bubble.png"
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Text for the button
                Text
                {
                    text: "Reddit Connection"
                    font.pointSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.leftMargin: 12 // Space between icon and text
                }
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onEntered:
                {
                    parent.color = "#f1f1f1" // Light gray on hover
                    parent.border.color = "#cccccc" // Darken border on hover
                }
                onExited:
                {
                    parent.color = "white" // Revert back to white
                    parent.border.color = "#dcdcdc" // Revert border color
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
            width: parent.width * 0.9 // Adjust width to have padding
            height: parent.height / itemCount * scaleFactor // Slightly smaller height for spacing
            radius: 10 // Rounded corners
            color: "white" // Default background color
            border.color: "#dcdcdc" // Light gray border color
            border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter // Center horizontally
            anchors.margins: 8 // Add margins for spacing

            Row // Use a row to align an icon and text
            {
                anchors.fill: parent
                anchors.margins: 10

                // Optional icon (left side)
                Image 
                {
                    source: "/images/expand-arrow.png"
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Text for the button
                Text
                {
                    text: "New Browser Connection"
                    font.pointSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.leftMargin: 12 // Space between icon and text
                }
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onEntered:
                {
                    parent.color = "#f1f1f1" // Light gray on hover
                    parent.border.color = "#cccccc" // Darken border on hover
                }
                onExited:
                {
                    parent.color = "white" // Revert back to white
                    parent.border.color = "#dcdcdc" // Revert border color
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
