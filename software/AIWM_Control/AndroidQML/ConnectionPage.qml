import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    width: 400
    height: 600
    clip: true

    function reset() { // Call when SWLP connection closed
        connectButton.visible = true
    }

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Item {
        anchors.fill: parent
        Button {
            id: connectButton
            width: 140
            height: 150
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
                connectButton.visible = !CppCore.startCommunication();
            }
        }
        Label {
            height: 30
            text: "Подключение к устройству..."
            visible: !connectButton.visible
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
        ProgressBar {
            id: progressBar
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            visible: !connectButton.visible
            indeterminate: true
        }
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
