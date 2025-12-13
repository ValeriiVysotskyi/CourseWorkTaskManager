import QtQuick 2.15
import QtQuick.Controls 2.15
import QtMultimedia

Dialog {
    id: watchingMemeDlg
    modal: true
    width: parent.width - 120
    height: parent.height - 120
    x: (parent.width - width)/2
    y: (parent.height - height)/2

    onClosed:{
        player.stop()
    }

    MediaPlayer {
        id: player
        source: "qrc:/MemeVideo.mp4"

        audioOutput: AudioOutput {
            id: audio
            volume: 0.5
        }

        videoOutput: videoOutput
    }

    Rectangle {
        id: dlgRect
        anchors.fill: parent
        color: "#1e1e1e"
        radius: 10

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "Це діалогове вікно створене, щоб покращити Ваш настрій -_-"
                color: "#ffffff"
                font.pixelSize: 22
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
            }

            VideoOutput {
                id: videoOutput
                width: parent.width
                height: parent.height * 0.80
                fillMode: VideoOutput.PreserveAspectFit
            }

            Row {
                width: parent.width
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "ДИВИТИСЬ"
                    width: parent.width * 0.22
                    height: dlgRect.height * 0.08
                    font.pixelSize: 22
                    onClicked: player.play()
                }

                Button {
                    text: "ПАУЗА"
                    width: parent.width * 0.22
                    height: dlgRect.height * 0.08
                    font.pixelSize: 22
                    onClicked: player.pause()
                }

                Button {
                    text: "СТОП"
                    width: parent.width * 0.22
                    height: dlgRect.height * 0.08
                    font.pixelSize: 22
                    onClicked: player.stop()
                }

                Button {
                    text: "ЗАКРИТИ"
                    width: parent.width * 0.22
                    height: dlgRect.height * 0.08
                    font.pixelSize: 22
                    onClicked: {
                        watchingMemeDlg.close()
                    }
                }
            }
        }
    }
}
