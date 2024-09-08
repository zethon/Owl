import QtQuick 2.15
import QtQuick.Controls 2.15

Item
{
    anchors.fill: parent  // Make the Item fill the entire parent container

    function setFocus()
    {
        console.log("setFocus");
        redditClientId.forceActiveFocus();
    }

    Component.onCompleted:
    {
        console.log("NewConnectionDlg.qml completed")
        redditClientId.forceActiveFocus()
    }


    Column
    {
        anchors.fill: parent  // Make the Column fill the Item
        spacing: 20
        padding: 20  // Add padding for better spacing from the edges

        // Paragraph text with filler and a clickable link
        Text
        {
            text: "Owl requires a Reddit Client ID to connect to Reddit. You can get your own Client ID by visiting Reddit's developer portal and creating an application."
            wrapMode: Text.WordWrap
            width: parent.width * 0.9  // Make the text 90% of the parent's width
        }

        // Clickable link
        Text
        {
            text: "<a href=\"https://www.reddit.com/prefs/apps\">Click here to get a Reddit Client ID</a>"
            color: "blue"
            font.underline: true
            wrapMode: Text.WordWrap
            width: parent.width * 0.9  // Make the link text 90% of the parent's width
            onLinkActivated: Qt.openUrlExternally(link)
        }

        // Input field for Reddit Client ID
        TextField
        {
            id: redditClientId
            selectByMouse: true
            width: parent.width * 0.9  // Make the TextField 90% of the parent's width
            placeholderText: "Enter Reddit Client ID"
            readOnly: false
            text: thisPage.redditId
        }

        // Input field for Random String (REDDIT_RANDOM_STRING)
        TextField
        {
            id: redditRandomString
            selectByMouse: true
            width: parent.width * 0.9
            placeholderText: "Enter Reddit Random String"
            text: "ArccClientForReddit"  // Default value from C++
        }

        // Input field for Redirect URL (REDDIT_REDIRECT_URL)
        TextField
        {
            id: redditRedirectUrl
            selectByMouse: true
            width: parent.width * 0.9
            placeholderText: "Enter Reddit Redirect URL"
            // text: "http://localhost:27182/oauth2"  // Default value from C++
        }

        // TextField
        // {
        //     id: redditScope
        //     selectByMouse: true
        //     width: parent.width * 0.9
        //     placeholderText: "Enter Reddit Scope"
        //     text: "identity,edit,history,mysubreddits,privatemessages,read,save,submit,subscribe,vote"  // Default value from C++
        // }

        // Input field for User Agent (USER_AGENT)
        TextField
        {
            id: userAgent
            selectByMouse: true
            width: parent.width * 0.9
            placeholderText: "Enter User Agent"
            text: "arcc/0.1 by /u/zethon"  // Default value from C++
        }

        // Spacer to push the buttons to the bottom
        Rectangle
        {
            width: 1
            height: 1
            visible: false
        }

        // Button Row
        Row
        {
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            // Ok Button
            Button
            {
                text: "Ok"
                onClicked:
                {
                    console.log("Ok clicked.")
                    console.log("Client ID: " + redditClientId.text)
                    console.log("Random String: " + redditRandomString.text)
                    console.log("Redirect URL: " + redditRedirectUrl.text)
                    // console.log("Scope: " + redditScope.text)
                    console.log("User Agent: " + userAgent.text)
                    thisPage.redditId = redditClientId.text
                    
                    thisPage.onAccept()
                }
            }

            // Cancel Button
            Button
            {
                text: "Cancel"
                onClicked:
                {
                    newConnectionPage.onCancel()
                }
            }
        }
    }
}
