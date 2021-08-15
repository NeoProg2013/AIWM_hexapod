import QtQuick 2.12
import QtQuick.Controls 2.5

Label {
    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    id: root
    width: 85
    height: 55
    clip: true
    color: "#888888"
    text: "TEXT"
    font.family: fixedFont.name
    font.pointSize: 8
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
    background: Rectangle {
        anchors.fill: parent
        color: "#00000000"
        border.color: "#555555"
        border.width: 1
        radius: 1

        Rectangle {
            width: 50
            height: 1
            anchors.horizontalCenter: parent.horizontalCenter
            color: root.color
            border.width: 0
        }
    }
}
