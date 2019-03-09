import QtQuick 2.4
import QtQuick.Controls 1.4
//import QtWebEngine 1.4

Item
{
    id: page

    Text
    {
        id: helloText
        text: "Hello world!"
        horizontalAlignment: Text.AlignHCenter
        anchors.fill: parent
        anchors.horizontalCenter: parent.center
        anchors.verticalCenter: parent.center
        font.pointSize: 9;
        font.bold: false
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
