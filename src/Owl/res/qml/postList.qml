import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Window 2.0
// import QtWebEngine 1.4

Item
{
    id: rootItem

    Component.onCompleted:
    {
        setHasPosts(false);
    }

    // gets called by ThreadViewWidget.cpp everytime new threads are loaded
    function setHasPosts(hasPosts)
    {
        postScrollView.visible = hasPosts;
        noPostsImage.visible = !hasPosts;
        fillRect.visible = !hasPosts;
    }

    Rectangle
    {
        id: fillRect
        anchors.fill: parent
    }

    Image
    {
        id: noPostsImage
        opacity: 0.07
        anchors.horizontalCenter: rootItem.horizontalCenter
        anchors.verticalCenter: rootItem.verticalCenter
        source: "../images/owl-bg1.png"
    }

    ScrollView
    {
        id: postScrollView
        anchors.fill: parent;

        ListView
        {
        id: postListView
        anchors.fill: parent
        model: postListModel
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        delegate: Item
        {
            id: postListDelegateItem
            width: parent.width
            height: authorText.height +
                    postText.height +
                    spacerRect.height +
                    replyButton.height
                    + 15

            Rectangle
            {
                width: parent.width
                anchors.top: parent.top

                Text
                {
                    id: authorText
                    anchors.top: parent.top;
                    anchors.topMargin: 2
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    text: model.modelData.author;
                    color: "#326464"
                    font.pointSize: 14
                }

                Text
                {
                    id: indexText
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.right: dateText.left
                    text: model.modelData.postIndex > 0 ? ("#" + model.modelData.postIndex) : "";
                    color: "#326464"
                    font.pointSize: 12
                    anchors.rightMargin: 10
                }

                Text
                {
                    id: dateText
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    text: model.modelData.dateText
                    color: "#326464"
                    font.pointSize: 12
                }

                Text
                {
                    id: postText
                    width: parent.width - 10
                    anchors.top: authorText.bottom
                    anchors.topMargin: 10
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    text: model.modelData.text
                    wrapMode: Text.WordWrap
                }

//                    WebEngineView
//                    {
//                        id: postText
//                        height: 100
//                        width: parent.width - 10
//                        anchors.top: authorText.bottom
//                        anchors.topMargin: 10
//                        anchors.left: parent.left
//                        anchors.leftMargin: 5
//                        anchors.rightMargin: 5
//                        Component.onCompleted:
//                        {
//                            loadHtml(model.modelData.text);
//                        }

//                        onContentsSizeChanged:
//                        {
//                            postText.height = contentsSize.height;
//                        }
//                    }

                Button
                {
                    id: replyButton
                    text: "Reply"
                    anchors.top: postText.bottom
                    anchors.topMargin: 5
                    anchors.right: parent.right
                    onClicked:
                    {
                        console.log("reply");
                    }
                }

                Button
                {
                    id: quoteButton
                    text: "Quote"
                    anchors.top: postText.bottom
                    anchors.topMargin: 5
                    anchors.right: replyButton.left
                    onClicked:
                    {
                        console.log("quote");
                    }
                }

                Rectangle
                {
                    id: spacerRect
                    height: 1
                    width: parent.width
                    anchors.top: parent.top
                    color: "black"
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
                        if (postListDelegateItem.ListView.view.currentIndex !== index)
                        {
                            postListDelegateItem.ListView.view.currentIndex = index;
                        }
                    }
                }
            }
        }
    }
}


}
