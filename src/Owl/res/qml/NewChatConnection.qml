import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog
{
    title: "Dialog Screen"
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: parent.width
    height: parent.height

    onAccepted:
    {
        console.log("Dialog accepted");
    }

    onRejected:
    {
        console.log("Dialog rejected");
    }

    Text
    {
        text: "This is a dialog screen."
        anchors.centerIn: parent
    }
}
