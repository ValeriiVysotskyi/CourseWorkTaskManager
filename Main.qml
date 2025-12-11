import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/"

Window {
    width: 1200
    height: 800
    visible: true
    title: qsTr("TaskManager")

    property var tasksList: []
    property int page: 1
    property int filterMode
    property int departmentNumber

    function updateTasksView(){
        pageBorder.visible = true
        pageInfo.visible = true

        switch(filterMode) {
            case 1:
                tasksList = dbManager.getUserTasksList(authController.currentUser(), page)
                break

            case 2:
                tasksList = dbManager.getDepartmentTasksList(departmentNumber, page)
                break

            case 3:
                tasksList = dbManager.getAllTasksList(page)
                break
        }

        for (var taskIndex = container.children.length - 1; taskIndex >= 0; taskIndex--) {
            container.children[taskIndex].destroy();
        }
        for (taskIndex = 0; taskIndex < tasksList.length; taskIndex++) {
            var component = Qt.createComponent("TaskCard.qml")
            if (component.status === Component.Ready) {
                var obj = component.createObject(container, {
                    "uuid": tasksList[taskIndex]["uuid"],
                    "assignee": tasksList[taskIndex]["assignee"],
                    "dueDate": tasksList[taskIndex]["due_date"],
                    "title": tasksList[taskIndex]["title"],
                    "redactTaskDlg": taskCreateDlg
                })
            }
        }
    }

    Connections{
        target: dbManager

        function onTasksViewChanged(){
            updateTasksView()
        }
    }

    Rectangle {
        id: loginRect
        visible: true
        width: 400
        height: 370
        anchors.centerIn: parent
        color: "#1E1E1E"
        radius: 10

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: "Авторизація"
                font.pixelSize: 28
                font.bold: true
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "Логін"
                font.pixelSize: 16
                color: "#FFFFFF"
            }

            Rectangle {
                width: parent.width
                height: 40
                radius: 5
                border.color: "#555555"
                border.width: 1
                color: "#2E2E2E"
                clip: true

                TextInput {
                    id: usernameInput
                    anchors.fill: parent
                    padding: 4
                    font.pixelSize: 20
                    color: "#FFFFFF"
                    selectByMouse: true
                }
            }

            Text {
                text: "Пароль"
                font.pixelSize: 16
                color: "#FFFFFF"
            }

            Rectangle {
                width: parent.width
                height: 40
                radius: 5
                border.color: "#555555"
                border.width: 1
                color: "#2E2E2E"
                clip: true

                TextInput {
                    id: passwordInput
                    anchors.fill: parent
                    padding: 4
                    font.pixelSize: 20
                    color: "#FFFFFF"
                    echoMode: TextInput.Password
                    selectByMouse: true
                }
            }

            Text {
                id: errorText
                color: "#FF6B6B"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Button {
            text: "Вхід"
            font.pixelSize: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 50
            anchors.margins: 10
            onClicked: {
                var dbAuthorizationInfo = dbManager.getAuthorizationInfo(usernameInput.text);
                authController.checkAuthorization(usernameInput.text, passwordInput.text, dbAuthorizationInfo);
            }
        }

        Connections {
            target:authController

            function onLoginSuccessfull(){
                loginRect.visible = false;
                rootRect.visible = true;
            }

            function onLoginFailed(){
                errorText.text = "Помилка авторизації. Перевірте введені дані";
            }
        }
    }

    Rectangle {
        id: rootRect
        anchors.fill: parent
        color: "#121212"
        visible: false

        Connections {
            target:dbManager

            function onTasksViewChanged(){
            }
        }

        TaskCreateDlg {
            id:taskCreateDlg
        }
        Column {
            spacing: 10
            anchors.top: parent.top
            anchors.topMargin: 20
            width: parent.width

            Row {
                spacing: 5
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "Мої завдання"
                    font.pixelSize: 24
                    font.bold: true
                    onClicked: {
                        page = 1
                        filterMode = 1
                        tasksList = dbManager.getUserTasksList(authController.currentUser(), 1)
                        updateTasksView()
                    }
                }

                Rectangle {
                    width: 2
                    height: 40
                    color: "gray"
                }

                Button {
                    text: "Завдання департаменту"
                    font.pixelSize: 24
                    font.bold: true
                    onClicked: {
                        rootRect.visible = false
                        departmentDialog.visible = true
                    }
                }

                Rectangle {
                    width: 2
                    height: 40
                    color: "gray"
                }

                Button {
                    text: "Усі завдання"
                    font.pixelSize: 24
                    font.bold: true
                    onClicked: {
                        page = 1
                        filterMode = 3
                        tasksList = dbManager.getAllTasksList(1)
                        updateTasksView()
                    }
                }

                Rectangle {
                    width: 2
                    height: 40
                    color: "gray"
                }

                Button {
                    text: "Створити"
                    font.pixelSize: 24
                    font.bold: true
                    onClicked: {
                        taskCreateDlg.createTaskStatus = 1
                        taskCreateDlg.open()
                    }
                }

                Rectangle {
                    id: pageBorder
                    visible: false
                    width: 2
                    height: 40
                    color: "gray"
                }

                Text {
                    id: pageInfo
                    visible: false
                    text: "Сторінка:" + page
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                }
            }

            Column {
                id: container
                objectName: "container"
                spacing: 5
                width: rootRect.width - 20
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: rootRect.horizontalCenter
            anchors.bottom: rootRect.bottom
            anchors.bottomMargin: 20

            Button {
                text:"1"
                font.pixelSize:22
                onClicked: {
                    page = 1
                    updateTasksView()
                }
            }

            Button {
                text:"2"
                font.pixelSize:22
                onClicked: {
                    page = 2
                    updateTasksView()
                }
            }

            Button {
                text:"3"
                font.pixelSize:22
                onClicked: {
                    page = 3
                    updateTasksView()
                }
            }
        }
    }

    Rectangle {
        id: departmentDialog
        visible: false
        anchors.fill: parent
        color: "#121212"

        Rectangle {

            width: 400
            height: 200
            color: "#1E1E1E"
            radius: 10
            anchors.centerIn: parent


            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Text {
                    text: "Введіть назву департаменту"
                    font.pixelSize: 20
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle {
                    width: parent.width
                    height: 40
                    radius: 5
                    border.color: "#555555"
                    border.width: 1
                    color: "#2E2E2E"
                    clip: true

                    TextInput {
                        id: departmentNameInput
                        anchors.fill: parent
                        padding: 4
                        font.pixelSize: 18
                        color: "#FFFFFF"
                        selectByMouse: true
                    }
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: "Фільтрувати"
                        font.pixelSize: 22
                        font.bold: true
                        onClicked: {
                            page = 1
                            filterMode = 2
                            departmentNumber = dbManager.getDepartmentNumber(departmentNameInput.text)
                            updateTasksView()

                            departmentNameInput.text = ""
                            departmentDialog.visible = false
                            rootRect.visible = true
                        }
                    }

                    Button {
                        text: "Скасувати"
                        font.pixelSize: 22
                        font.bold: true
                        onClicked: {
                            departmentNameInput.text = ""
                            departmentDialog.visible = false
                            rootRect.visible = true
                        }
                    }
                }
            }
        }
    }
}
