// TaskCard.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: card
    property string uuid
    property string assignee
    property string dueDate
    property string title
    property var redactTaskDlg
    width: parent.width
    height: 60
    radius: 10
    color: "#2E2E2E"
    border.color: "#555555"
    border.width: 1

    Text {
        text: title
        font.pixelSize: 20
        font.bold: true
        color: "#FFFFFF"
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
        elide: Text.ElideRight
        width: parent.width * 0.7
    }

    Column {
        anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            spacing: 2

        Text {
            text: "Відповідальний: " + assignee
            font.pixelSize: 16
            color: "#AAAAAA"
        }

        Text {
            text: "Виконати до: " + dueDate
            font.pixelSize: 16
            color: "#AAAAAA"
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            redactTaskDlg.createTaskStatus = 0
            redactTaskDlg.uuid = uuid
            redactTaskDlg.open()
        }
    }
}
