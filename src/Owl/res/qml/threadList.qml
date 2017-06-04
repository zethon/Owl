import QtQuick 2.7
import QtQuick.Controls 1.4
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
            text: "Hide Stickies"

            onTriggered:
            {
                threadListPage.showStickies = !threadListPage.showStickies;
                threadListPage.refreshThreadDisplay();
                hideStickyMenuItem.text = threadListPage.showStickies ? "Hide Stickies" : "Show Stickies";
                stickyDisplayChanged();
            }
        }

        MenuSeparator { }

        MenuItem
        {
            text: "Copy URL"
            onTriggered:
            {
                threadListPage.copyUrl(threadListView.currentIndex);
            }
        }

        MenuItem
        {
            text: "Open in Browser"
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
                width: parent.width
                height: authorText.height +
                        titleText.height +
                        (previewText.visible ? previewText.height + 5 : 0) +
                        spacerRect.height +
                        (lastAuthorRect.visible ? lastAuthorRect.height : 0)

                visible: !model.modelData.sticky || threadListPage.showStickies

                Rectangle
                {
                    id: delegateFillRect
                    width: parent.width
                    anchors.top: parent.top

                    Image
                    {
                        id: lastAuthorImage
                        visible: threadListSettings.read("threadlist.avatars.visible") && model.modelData.iconUrl.length > 0
                        source: model.modelData.iconUrl
                        width: 32
                        height: 32
                        anchors.top: parent.top
                        anchors.topMargin: 2
                        anchors.left: parent.left
                        anchors.leftMargin: 2
                        onStatusChanged:
                        {
                            if (lastAuthorImage.status == Image.Error)
                            {
                                lastAuthorImage.source = "qrc:/icons/no-avatar.png";
                            }
                        }
                    }

                    Text
                    {
                        id: authorText
                        text: author //lastAuthor
                        font.pointSize: 13; font.bold: unread
                        color: "#326464"
                        anchors.topMargin: 2
                        anchors.left: lastAuthorImage.visible ? lastAuthorImage.right : parent.left
                        anchors.leftMargin: 5
                    }

                    Image
                    {
                        id: stickyImage
                        anchors.topMargin: 2
                        anchors.top: parent.top
                        anchors.right: model.modelData.replyCount > 0 ? replyCountRect.left : parent.right
                        anchors.rightMargin: 5
                        width: 16
                        height: 16
                        source: "/icons/pin.png"
                        visible: model.modelData.sticky
                    }

                    Rectangle
                    {
                        id: replyCountRect
                        visible: model.modelData.replyCount > 0
                        anchors.right: parent.right
                        anchors.rightMargin: 15
                        anchors.top: parent.top
                        anchors.topMargin: 2
                        width: replyCountImage.width + replyCountObj.width

                        Image
                        {
                            id: replyCountImage
                            anchors.top: parent.top
                            anchors.left: parent.left
                            source: "/icons/replies.png"
                            width: 16
                            height: 16
                        }

                        Text
                        {
                            id: replyCountObj
                            anchors.left: replyCountImage.right
                            anchors.leftMargin: 3
                            text: model.modelData.replyCount
                            font.pointSize: smallTextSize; font.bold: unread
                            color: "#326464"
                        }
                    }

                    Text
                    {
                        id: titleText
                        anchors.top: authorText.bottom
                        text: title
                        font.pointSize: 13; font.bold: unread
                        wrapMode: Text.WordWrap
                        anchors.left: lastAuthorImage.visible ? authorText.left : parent.left
                        anchors.leftMargin: lastAuthorImage.visible ? 0 : 5
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }

                    Text
                    {
                        id: previewText
                        text: model.modelData.previewText
                        anchors.left: parent.left
                        anchors.top: titleText.bottom
                        width: parent.width - 10
                        wrapMode: Text.WordWrap
                        visible: threadListSettings.read("threadlist.previewtext.visible") && model.modelData.previewText.length > 0
                        font.pointSize: 12; font.bold: unread
                        color: "grey"
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        anchors.topMargin: 5
                    }

                    Rectangle
                    {
                        id: lastAuthorRect
                        anchors.left: parent.left;
                        anchors.top: previewText.visible ? previewText.bottom : titleText.bottom
                        color: threadListView.currentIndex == index ? "#e4ebf1" : "#F7F9F9" // if this item is selected then #e4ebf1
                        width: parent.width
                        height: lastAuthorText.height
                        visible: threadListSettings.read("threadlist.lastauthor.visible")

                        Text
                        {
                            id: lastAuthorText
                            text: model.modelData.lastAuthor
                            font.pointSize: smallTextSize;
                            font.bold: unread
                            anchors.top: parent.top
                            anchors.leftMargin: 5
                            anchors.left: parent.left
                            color: "grey"
                        }

                        Text
                        {
                            id: dateTextObj
                            text: model.modelData.dateText
                            font.pointSize: smallTextSize;
                            anchors.right: parent.right
                            anchors.rightMargin: 5
                            color: "grey"
                        }
                    }

                    Rectangle
                    {
                        id: spacerRect
                        height: 1
                        width: parent.width
                        anchors.top: parent.top
                        color: "lightgrey"
                        visible: index != 0
                    }
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
