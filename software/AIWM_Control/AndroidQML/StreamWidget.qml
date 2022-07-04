import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    width: 640
    height: 480
    clip: true

    Connections {
        target: CppStreamService
        function onFrameReceived() {
            ++fpsTimer.fpsCounter
            streamFrame.updateSourceImage()
            noiseImage.visible = false
            connectButton.visible = false
        }
        function onConnectionClosed() {
            noiseImage.visible = true
            connectButton.visible = true
        }
    }

    Timer {
        id: fpsTimer
        running: true
        interval: 1000
        repeat: true
        onTriggered: {
            fpsLabel.text = "FPS: " + fpsCounter
            fpsCounter = 0
        }

        property int fpsCounter: 0
    }

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Image {
        id: noiseImage
        anchors.fill: parent
        source: ""
        verticalAlignment: Qt.AlignTop

        Button {
            id: connectButton
            text: "ПОДКЛЮЧИТЬСЯ К КАМЕРЕ"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: fixedFont.name
            onClicked: {
                visible = !CppStreamService.startService();
            }
        }
        ProgressBar {
            id: progressBar
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            visible: !connectButton.visible
            indeterminate: true
        }
    }

    Image {
        id: streamFrame
        visible: !noiseImage.visible
        anchors.fill: parent
        source: ""
        fillMode: Image.PreserveAspectFit
        verticalAlignment: Qt.AlignTop
        smooth: true
        cache: false
        asynchronous: false
        rotation: 90

        property bool flag: false
        function updateSourceImage() {
            if (flag == true) {
                source = "image://streamFrameProvider/1"
            } else {
                source = "image://streamFrameProvider/2"
            }
            flag = !flag
        }
    }

    Label {
        id: fpsLabel
        x: 5
        y: 5
        width: 120
        height: 17
        text: "FPS: --"
        font.family: fixedFont.name
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:2}D{i:6}
}
##^##*/

