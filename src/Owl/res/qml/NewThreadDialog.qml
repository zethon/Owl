import QtQuick.Window 2.2
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
// import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3

Item
{
    Component.onCompleted:
    {
        inputArea.forceActiveFocus()
    }

    Pane
    {
        id: pane
        Material.elevation: 16

        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: 10

            Text
            {
                text: qsTr("Title")
                font.bold: true
            }

            Text
            {
                text: qsTr("This is the description")
                font.weight: Font.Light
            }

            TextArea
            {
                id: inputArea
                placeholderText: "Enter post text here..."
            }


            Button
            {
                text: "click me!";
                onClicked:
                {
                    globalPopUpDialog.close();
                }
            }
        }
    }
}
