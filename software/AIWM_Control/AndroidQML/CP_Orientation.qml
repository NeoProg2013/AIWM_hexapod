import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 250
    height: 45
    clip: true

    property int rx: 1000
    property int ry: 1000
    property int rz: 1000
    property int px: 1000
    property int py: 1000
    property int pz: 1000

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }


    Label {
        color: "#00DD00"
        font.family: fixedFont.name
        text: "[" + rx + " " + ry + " " + rz + "] / ["+ px + " " + py + " " + pz + "]"
        anchors.fill: parent
        font.pixelSize: 10
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:2}
}
##^##*/
