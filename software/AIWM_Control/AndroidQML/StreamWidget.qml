import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    width: 640
    height: 480
    clip: true

    property int fpsCounter: 0

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Connections {
        target: CppCore
        function onStreamServiceFrameReceived() {
            ++fpsCounter
            streamFrame.updateSourceImage()
            noiseImage.visible = false
            streamFrame.visible = true
            connectButton.visible = false
            progressBar.visible = false
        }
        function onStreamServiceBadFrameReceived() {
            noiseImage.visible = true
            streamFrame.visible = false
        }
        function onStreamServiceConnectionClosed() {
            connectButton.visible = true
            progressBar.visible = false
            noiseImage.visible = true
            streamFrame.visible = false
        }
        function onStreamServiceIpAddressUpdate(ipAddress) {
            ipAddressLabel.text = "IP: " + ipAddress
        }
    }

    Timer {
        running: true
        interval: 1000
        repeat: true
        onTriggered: {
            fpsLabel.text = "FPS: " + fpsCounter
            fpsCounter = 0
        }
    }

    ProgressBar {
        id: progressBar
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: false
        indeterminate: true
        z: 1
    }

    Button {
        id: connectButton
        visible: true
        z: 1
        text: "ПОДКЛЮЧИТЬСЯ К КАМЕРЕ"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.family: fixedFont.name
        onClicked: {
            connectButton.visible = false
            progressBar.visible = true
            if (!CppCore.runStreamService()) {
                progressBar.visible = false
                connectButton.visible = true
            }
        }
    }

    AnimatedImage {
        id: noiseImage
        visible: true
        anchors.fill: parent
        source: "qrc:/images/noise.gif"
        fillMode: Image.Tile
        verticalAlignment: Qt.AlignTop
    }

    Image {
        id: streamFrame
        visible: false
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
        id: ipAddressLabel
        x: 5
        y: 5
        width: 120
        height: 17
        text: "IP: 255.255.255.255"
    }

    Label {
        id: fpsLabel
        x: 5
        y: 27
        width: 120
        height: 17
        text: "FPS: 0"
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:2;anchors_y:407}D{i:6;anchors_x:5}
}
##^##*/

