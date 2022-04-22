import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 132
    height: 45
    clip: true

    property int rx: 0
    property int ry: 0
    property int rz: 0
    property int px: 0
    property int py: 0
    property int pz: 0

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Label {
        x: 0; y: 0
        width: 47
        height: 20
        font.family: fixedFont.name
        color: "#00DD00"
        text: qsTr("rotate:")
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        x: 52; y: 0
        width: 80; height: 20
        color: "#00DD00"
        font.family: fixedFont.name
        text: rx + " " + ry + " " + rz
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        x: 0; y: 25
        width: 47; height: 20
        font.family: fixedFont.name
        color: "#00DD00"
        text: qsTr("offset:")
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        x: 52; y: 25
        width: 80; height: 20
        font.family: fixedFont.name
        color: "#00DD00"
        text: px + " " + py + " " + pz
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:2}D{i:3}D{i:4}
}
##^##*/
