import QtQuick 2.4
import QtQuick.Controls 1.4
//import QtWebEngine 1.4

Item 
{
    id: page

    Text {
        id: helloText
        text: "Hello world!"
        anchors.fill: parent
        font.pointSize: 24; font.bold: true
    }
}
