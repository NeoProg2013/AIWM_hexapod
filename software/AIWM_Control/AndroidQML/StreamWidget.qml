import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    width: 640
    height: 480
    clip: true

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Connections {
        target: CppCore
        function onStreamServiceFrameReceived() {
            streamFrame.updateSourceImage()
            noiseImage.visible = false
            streamFrame.visible = true
        }
        function onStreamServiceBadFrameReceived() {
            noiseImage.visible = true
            streamFrame.visible = false
        }
        function onStreamServiceConnectionClosed() {
            noiseImage.visible = true
            streamFrame.visible = false
        }
    }


    /*Timer {
        id: connectionTimer
        repeat: false
        interval: 1000
        running: true
        onTriggered: {
            CppCore.stopStreamService()
            CppCore.runStreamService()
        }
    }*/
    Button {
        visible: noiseImage.visible
        z: 1
        text: "ПОДКЛЮЧИТЬСЯ К КАМЕРЕ"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.family: fixedFont.name
        onClicked: {
            CppCore.stopStreamService()
            CppCore.runStreamService()
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
        source: "image://streamFrameProvider/1"
        fillMode: Image.PreserveAspectFit
        verticalAlignment: Qt.AlignTop

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
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:2;anchors_y:407}D{i:5;anchors_x:5}
}
##^##*/
