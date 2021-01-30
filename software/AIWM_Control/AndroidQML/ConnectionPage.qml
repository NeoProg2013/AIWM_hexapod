import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    width: 400
    height: 600
    clip: true

    signal showControlPage

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Connections {
        target: CppCore
        function onFrameReceived() {
            labelText.text = ""
            labelText.color = "#FFFFFF"
            connectButton.visible = true
            progressBar.visible = false
            timeoutTimer.stop()
            showControlPage()
        }
    }

    Timer {
        id: timeoutTimer
        interval: 5000
        repeat: false
        onTriggered: {
            errorLabel.visible = true
            labelText.visible = false
            connectButton.visible = true
            progressBar.visible = false
            CppCore.stopCommunication()
        }
    }

    Label {
        id: labelText
        height: 30
        text: "Подключение к устройству..."
        visible: false
        anchors.bottom: progressBar.top
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        font.family: fixedFont.name
        font.pointSize: 14
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    Button {
        id: connectButton
        width: 140
        height: 150
        text: qsTr("")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        background: Item {
            Image {
                id: buttonImage
                visible: true
                anchors.fill: parent
                source: "qrc:/images/powerOn.svg"
                sourceSize.width: parent.width
                sourceSize.height: parent.height
            }
            ColorOverlay {
                anchors.fill: parent
                smooth: true
                antialiasing: true
                source: buttonImage
                color: "#00FFFF"
            }
        }
        onClicked: {
            errorLabel.visible = false
            labelText.visible = true
            connectButton.visible = false
            progressBar.visible = true

            if (!CppCore.runCommunication()) {
                errorLabel.visible = true
                labelText.visible = false
                connectButton.visible = true
                progressBar.visible = false
                CppCore.stopCommunication()
                return
            }
            timeoutTimer.start()
        }
    }

    ProgressBar {
        id: progressBar
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        indeterminate: true
    }

    Label {
        y: 863
        width: 120
        height: 20
        font.family: fixedFont.name
        text: qsTr("Версия: 1.01")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
    }

    Label {
        id: errorLabel
        height: 30
        text: "Ошибка при подключении к устройству"
        color: "#FF0000"
        visible: false
        anchors.top: connectButton.bottom
        anchors.topMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        font.family: fixedFont.name
        font.pointSize: 14
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        color: "#00ffff"
        text: "AIWM CONTROL"
        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.right: parent.right
        anchors.rightMargin: 15
        anchors.left: parent.left
        anchors.leftMargin: 15
        font.family: fixedFont.name
        font.pixelSize: 40
        fontSizeMode: Text.Fit
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignHCenter
    }
}

/*##^## Designer {
    D{i:2;anchors_y:407}D{i:5;anchors_x:5}D{i:8;anchors_width:100;anchors_x:5;anchors_y:863}
D{i:9;anchors_y:2}D{i:12;anchors_x:239;anchors_y:15}
}
 ##^##*/

