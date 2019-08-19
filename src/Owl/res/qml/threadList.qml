import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import reader.owl 1.0


Item
{
    id: rootItem

    property int smallTextSize: 11

    signal stickyDisplayChanged

    Component.onCompleted:
    {
        setHasThreads(false);
        ListView.currentIndex = -1;
    }

    Settings
    {
        id: threadListSettings
    }

    // gets called by ThreadViewWidget.cpp everytime new threads are loaded
    function setHasThreads(hasThreads)
    {
        threadScrollView.visible = hasThreads;
        noThreadsImage.visible = !hasThreads;
        fillRect.visible = !hasThreads;
        threadListView.currentIndex = -1;
    }

    Rectangle
    {
        id: fillRect
        anchors.fill: parent
    }

    Image
    {
        id: noThreadsImage
        opacity: 0.07
        anchors.horizontalCenter: rootItem.horizontalCenter
        anchors.verticalCenter: rootItem.verticalCenter
        source: "../images/owl-bg1.png"
    }

    Menu
    {
        id: threadViewContextMenu

        MenuItem
        {
            id: hideStickyMenuItem
            text: qsTr("Hide Stickies")

            onTriggered:
            {
                threadListPage.showStickies = !threadListPage.showStickies;
                threadListPage.refreshThreadDisplay();
                hideStickyMenuItem.text = threadListPage.showStickies
                        ? qsTr("Hide Stickies") : qsTr("Show Stickies");
                stickyDisplayChanged();
            }
        }

        MenuSeparator { }

        MenuItem
        {
            text: qsTr("Copy URL")
            onTriggered:
            {
                threadListPage.copyUrl(threadListView.currentIndex);
            }
        }

        MenuItem
        {
            text: qsTr("Open in Browser")
            onTriggered:
            {
                threadListPage.loadInBrowser(threadListView.currentIndex);
            }
        }
    }

    ScrollView
    {
        id: threadScrollView
        anchors.fill: parent;

        ListView
        {
            id: threadListView
            anchors.fill: parent
            model: threadListModel
            focus: true

            activeFocusOnTab: true
            boundsBehavior: Flickable.StopAtBounds
            highlightFollowsCurrentItem: true
            highlightMoveDuration: 100
            highlightMoveVelocity: 100
            flickableDirection: Flickable.VerticalFlick

            highlight: Rectangle { color: "#e4ebf1" }

            delegate: Item
            {
                id: threadListDelegate

                property int avatarRectWidth: 68
                property int spacerRectHeight: 10
                property int delegateSpacerHeight: 25

                width: parent.width

//                property int componentHeight:
//                    authorText.height + authorText.anchors.topMargin + authorText.anchors.bottomMargin +
//                    titleText.height + titleText.anchors.topMargin + titleText.anchors.bottomMargin +
//                    (previewText.visible ? previewText.height + 5 : 0) +
//                    spacerRect.height +
//                    (lastAuthorRect.visible ? lastAuthorRect.height : 0)

                //height: Math.max(158, componentHeight)
                height: delegateSpacerHeight
                        + titleText.height
                        + authorText.height
                        + previewTextItem.height

                //visible: !model.modelData.sticky || threadListPage.showStickies

                Rectangle
                {
                    id: delegateFillRect
                    anchors.top: parent.top
                    width: parent.width

                    Rectangle
                    {
                        id: avatarRect
                        anchors.top: parent.top
                        anchors.left: parent.left
                        height: threadListDelegate.height-spacerRect.height
                        width: threadListDelegate.avatarRectWidth
                        visible: threadListSettings.read("threadlist.avatars.visible") && model.modelData.iconUrl.length > 0

                        Image
                        {
                            id: lastAuthorImage
                            width: 52
                            height: 52
                            anchors.horizontalCenter: avatarRect.horizontalCenter
                            anchors.top: avatarRect.top
                            source: model.modelData.iconUrl
                            fillMode: Image.PreserveAspectCrop

                            onStatusChanged:
                            {
                                if (lastAuthorImage.status == Image.Error)
                                {
                                    lastAuthorImage.source = "qrc:/icons/no-avatar.png";
                                }
                            }

                            property bool rounded: true
                            property bool adapt: false

                            layer.enabled: rounded
                            layer.effect: OpacityMask
                            {
                                maskSource: Item
                                {
                                    width: lastAuthorImage.width
                                    height: lastAuthorImage.height
                                    Rectangle
                                    {
                                        anchors.centerIn: parent
                                        width: lastAuthorImage.adapt ? lastAuthorImage.width : Math.min(lastAuthorImage.width, lastAuthorImage.height)
                                        height: lastAuthorImage.adapt ? lastAuthorImage.height : width
                                        radius: Math.min(width, height)
                                    }
                                }
                            }
                        }

                        Image
                        {
                            id: stickyImage
                            anchors.topMargin: 2
                            anchors.top: parent.top
                            anchors.right: avatarRect.right
                            width: 24
                            height: 24
                            source: "/icons/pin.png"
                            visible: model.modelData.sticky
                        }
                    }

                    Rectangle
                    {
                        id: contentRect
                        anchors.top: parent.top
                        anchors.left: avatarRect.right
                        height: threadListDelegate.height-spacerRect.height
                        width: threadListDelegate.width-avatarRect.width

                        Text
                        {
                            id: titleText
                            anchors.top: parent.top
                            anchors.left: parent.left
                            width: parent.width
                            text: title
                            font.pointSize: 11;
                            font.bold: unread
                            wrapMode: Text.Wrap
                        }

                        Text
                        {
                            id: authorText
                            anchors.top: titleText.bottom
                            text: author
                            font.pointSize: 9;
                            color: "grey"
                            font.bold: true
                        }

                        Text
                        {
                            id: repliesText
                            anchors.top: titleText.bottom
                            anchors.left: authorText.right
                            text: " " + String.fromCharCode(8226) + " "
                                  + replyCount + " "
                                  + (replyCount == 1 ? qsTr("reply") : qsTr("replies"))
                            font.pointSize: 9;
                            color: "grey"
                        }

                        Text
                        {
                            id: dateText
                            anchors.top: titleText.bottom
                            anchors.left: repliesText.right
                            text: " " + String.fromCharCode(8226) + " " + model.modelData.dateText
                            font.pointSize: 9;
                            color: "grey"
                        }

                        Text
                        {
                            id: previewTextItem
                            anchors.top: authorText.bottom
                            anchors.left: parent.left
                            width: parent.width
                            text: previewText
                            wrapMode: Text.Wrap
                            font.pointSize: 8
                            color: "grey"
                        }
                    }

                    Rectangle
                    {
                        id: spacerRect
                        height: threadListDelegate.spacerRectHeight
                        width: delegateFillRect.width
                        anchors.top: avatarRect.bottom
                    }

//                    Image
//                    {
//                        id: lastAuthorImage
//                        visible: threadListSettings.read("threadlist.avatars.visible") && model.modelData.iconUrl.length > 0
//                        source: model.modelData.iconUrl
//                        width: 52
//                        height: 52
//                        anchors.top: parent.top
//                        anchors.topMargin: 2
//                        anchors.bottomMargin: 2
//                        anchors.left: parent.left
//                        anchors.leftMargin: 2
//                        fillMode: Image.PreserveAspectCrop
//                        onStatusChanged:
//                        {
//                            if (lastAuthorImage.status == Image.Error)
//                            {
//                                lastAuthorImage.source = "qrc:/icons/no-avatar.png";
//                            }
//                        }

//                        property bool rounded: true
//                        property bool adapt: false

//                        layer.enabled: rounded
//                        layer.effect: OpacityMask
//                        {
//                            maskSource: Item
//                            {
//                                width: lastAuthorImage.width
//                                height: lastAuthorImage.height
//                                Rectangle
//                                {
//                                    anchors.centerIn: parent
//                                    width: lastAuthorImage.adapt ? lastAuthorImage.width : Math.min(lastAuthorImage.width, lastAuthorImage.height)
//                                    height: lastAuthorImage.adapt ? lastAuthorImage.height : width
//                                    radius: Math.min(width, height)
//                                }
//                            }
//                        }
//                    }

//                    Text
//                    {
//                        id: titleText
//                        anchors.top: parent.top
//                        text: title
//                        font.pointSize: 13;
//                        font.bold: unread
//                        wrapMode: Text.Wrap
//                        anchors.left: lastAuthorImage.visible ? authorText.left : parent.left
//                        anchors.leftMargin: lastAuthorImage.visible ? 0 : 20
//                        anchors.right: parent.right
//                        anchors.rightMargin: 5
//                    }

//                    Text
//                    {
//                        id: authorText
//                        text: author + ":" + lastAuthor
//                        font.pointSize: 8;
//                        font.bold: unread
//                        color: "grey"
//                        anchors.top: titleText.bottom
//                        anchors.topMargin: 5
//                        anchors.left: lastAuthorImage.visible
//                            ? lastAuthorImage.right : parent.left
//                        anchors.leftMargin: 5
//                    }

//                    Text
//                    {
//                        id: createdTimeText
////                        text: "Aug 19, 2017 3:44pm"
//                        text: model.modelData.createdTimeText
//                        font.pointSize: 8;
//                        color: "grey"
//                        anchors.top: authorText.top
//                        anchors.topMargin: authorText.anchors.topMargin
//                        anchors.left: authorText.right
//                        anchors.leftMargin: 5
//                    }

//                    Image
//                    {
//                        id: stickyImage
//                        anchors.topMargin: 2
//                        anchors.top: parent.top
//                        anchors.right: model.modelData.replyCount > 0 ? replyCountRect.left : parent.right
//                        anchors.rightMargin: 5
//                        width: 16
//                        height: 16
//                        source: "/icons/pin.png"
//                        visible: model.modelData.sticky
//                    }

//                    Rectangle
//                    {
//                        id: replyCountRect
//                        visible: model.modelData.replyCount > 0
//                        anchors.right: parent.right
//                        anchors.rightMargin: 15
//                        anchors.top: parent.top
//                        anchors.topMargin: 2
//                        width: replyCountImage.width + replyCountObj.width

//                        Image
//                        {
//                            id: replyCountImage
//                            anchors.top: parent.top
//                            anchors.left: parent.left
//                            source: "/icons/replies.png"
//                            width: 16
//                            height: 16
//                        }

//                        Text
//                        {
//                            id: replyCountObj
//                            anchors.left: replyCountImage.right
//                            anchors.leftMargin: 3
//                            text: model.modelData.replyCount
//                            font.pointSize: smallTextSize; font.bold: unread
//                            color: "#326464"
//                        }
//                    }

//                    Text
//                    {
//                        id: previewText
//                        text: model.modelData.previewText
//                        anchors.top: authorText.bottom
//                        anchors.left: lastAuthorImage.right
//                        anchors.leftMargin: 5
//                        anchors.rightMargin: 5
//                        anchors.topMargin: 5
//                        width: parent.width - 10
//                        wrapMode: Text.WordWrap
//                        //visible: threadListSettings.read("threadlist.previewtext.visible") && model.modelData.previewText.length > 0
//                        visible: false
//                        font.pointSize: 12; font.bold: unread
//                        color: "grey"
//                    }

//                    Rectangle
//                    {
//                        id: lastAuthorRect
//                        anchors.left: lastAuthorImage.right
//                        anchors.top: previewText.visible ? previewText.bottom : authorText.bottom
//                        color: threadListView.currentIndex == index ? "#e4ebf1" : "#F7F9F9" // if this item is selected then #e4ebf1
//                        width: parent.width
//                        height: lastAuthorText.height
//                        //visible: threadListSettings.read("threadlist.lastauthor.visible")
//                        visible: false;


//                        Text
//                        {
//                            id: lastAuthorText
//                            text: model.modelData.lastAuthor + ":" + model.modelData.author
//                            font.pointSize: smallTextSize;
//                            font.bold: unread
//                            anchors.top: parent.top
//                            anchors.leftMargin: 5
//                            anchors.left: parent.left
//                            color: "grey"
//                        }

//                        Text
//                        {
//                            id: dateTextObj
//                            text: "DT:" + model.modelData.dateText
//                            font.pointSize: smallTextSize;
//                            anchors.right: parent.right
//                            anchors.rightMargin: 5
//                            color: "grey"
//                        }
//                    }


                }

                MouseArea
                {
                    id: mouseArea
                    hoverEnabled: true
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked:
                    {
                        if (mouse.button === Qt.LeftButton)
                        {
                            if (threadListDelegate.ListView.view.currentIndex !== index)
                            {
                                threadListDelegate.ListView.view.currentIndex = index;
                                model.modelData.loadThread();
                            }
                        }
                    }

                    onPressed:
                    {
                        if (mouse.button === Qt.RightButton)
                        {
                            mouse.accepted = true;
                            threadViewContextMenu.popup();
                        }
                    }
                }
            } // Item
        }
    }
}
