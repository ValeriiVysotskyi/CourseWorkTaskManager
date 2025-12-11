// TaskCreateDlg.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id:taskCreateDlg
    modal: true
    width: parent.width - 120
    height: parent.height - 120
    x: (parent.width - width)/2
    y: (parent.height - height)/2

    property int createTaskStatus
    property string uuid
    property int departmentId

    onVisibleChanged: {
        if (createTaskStatus) {
            taskCreateDlg.title = "Створити завдання"
            createTaskBtn.visible = true
            redactTaskBtn.visible = false
            completeTaskBtn.visible = false
        }
        else{
            taskCreateDlg.title = "Інформація про завдання"
            createTaskBtn.visible = false
            redactTaskBtn.visible = true
            completeTaskBtn.visible = true

            var taskInfo = dbManager.getTaskInfo(uuid)
            if(Object.keys(taskInfo).length === 0){
                taskCreateDlg.close()
            }
            else{
                taskTitle.text = taskInfo["title"]
                taskDescription.text = taskInfo["description"]
                assignee.text = taskInfo["assignee"]
                assigner.text = taskInfo["assigner"]
                dueDate.text = taskInfo["due_date"]
                department.text = taskInfo["department_name"]
                departmentId = taskInfo["department_id"]
            }
        }
    }

    onClosed: {
        uuid = ""
        departmentId = 0
        taskTitle.text = ""
        taskDescription.text = ""
        assigner.text = ""
        assignee.text = ""
        department.text = ""
        dueDate.text = ""
    }

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"
        radius: 10

        Column {
            width: parent.width - 40
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20

            Text {
                text: "Введіть назву завдання:"
                color: "#ffffff"
                font.pixelSize: 24
            }

            Rectangle {
                width: parent.width
                height: 50
                radius: 6
                border.color: "#555555"
                border.width: 1

                TextInput {
                    id: taskTitle
                    anchors.fill: parent
                    anchors.margins: 4
                    font.pixelSize: 24
                    selectByMouse: true
                }
            }

            Text {
                text: "Опис задачі:"
                color: "#ffffff"
                font.pixelSize: 24
            }

            Rectangle {
                width: parent.width
                height: 150
                radius: 6
                border.color: "#555555"
                border.width: 1

                TextInput {
                    id: taskDescription
                    anchors.fill: parent
                    anchors.margins: 4
                    font.pixelSize: 24
                    wrapMode: TextInput.Wrap
                    selectByMouse: true
                }
            }

            Row {
                spacing: 10

                Text {
                    text: "Постановник:"
                    color: "#ffffff"
                    font.pixelSize: 24
                }

                Rectangle {
                    width: 200
                    height: 35
                    radius: 6
                    border.color: "#555555"
                    border.width: 1

                    TextInput {
                        id: assigner
                        anchors.fill: parent
                        anchors.margins: 4
                        font.pixelSize: 24
                    }
                }
            }

            Row {
                spacing: 10

                Text {
                    text: "Відповідальний:"
                    color: "#ffffff"
                    font.pixelSize: 24
                }

                Rectangle {
                    width: 200
                    height: 35
                    radius: 6
                    border.color: "#555555"
                    border.width: 1

                    TextInput {
                        id: assignee
                        anchors.fill: parent
                        anchors.margins: 4
                        font.pixelSize: 24
                    }
                }
            }

            Row {
                spacing: 10

                Text {
                    text: "Департамент:"
                    color: "#ffffff"
                    font.pixelSize: 24
                }

                Rectangle {
                    width: 200
                    height: 35
                    radius: 6
                    border.color: "#555555"
                    border.width: 1

                    TextInput {
                        id: department
                        anchors.fill: parent
                        anchors.margins: 4
                        font.pixelSize: 24
                    }
                }
            }

            Row {
                spacing: 10

                Text {
                    text: "Крайній термін (YYYY-MM-DD):"
                    color: "#ffffff"
                    font.pixelSize: 24
                }

                Rectangle {
                    width: 150
                    height: 35
                    radius: 6
                    border.color: "#555555"
                    border.width: 1

                    TextInput {
                        id: dueDate
                        anchors.fill: parent
                        anchors.margins: 4
                        font.pixelSize: 24
                    }
                }
            }
        }

        Row {
            spacing: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20

            Button {
                id: completeTaskBtn
                text: "Завершити"
                font.pixelSize: 25
                font.bold: true
                onClicked: {
                    dbManager.completeTask(uuid)
                    taskCreateDlg.close()
                }
            }

            Button {
                id: redactTaskBtn
                text: "Редагувати"
                font.pixelSize: 25
                font.bold: true
                onClicked: {
                    var taskInfo = {
                        "uuid":uuid,
                        "title":taskTitle.text,
                        "description":taskDescription.text,
                        "assigner":assigner.text,
                        "assignee":assignee.text,
                        "fk_department_id":departmentId,
                        "due_date":dueDate.text
                    }
                    dbManager.redactTask(taskInfo)
                    taskCreateDlg.close()
                }
            }

            Button {
                id: createTaskBtn
                text: "Створити"
                font.pixelSize: 25
                font.bold: true
                onClicked: {
                    var fk_department_id = dbManager.getDepartmentNumber(department.text)
                    var taskInfo = {
                        "title":taskTitle.text,
                        "description":taskDescription.text,
                        "assigner":assigner.text,
                        "assignee":assignee.text,
                        "fk_department_id":fk_department_id,
                        "due_date":dueDate.text
                    }

                    dbManager.addTask(taskInfo)
                    taskCreateDlg.close()
                }
            }

            Button {
                text: "Скасувати"
                font.pixelSize: 25
                font.bold: true
                onClicked: {
                    taskCreateDlg.close()
                }
            }
        }
    }
}
