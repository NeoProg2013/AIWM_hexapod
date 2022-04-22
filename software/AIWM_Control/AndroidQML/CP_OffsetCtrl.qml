import QtQuick 2.12
import QtQuick.Controls 2.15


Item {
    id: root
    width: 400
    height: 300
    clip: true

    property int px: 0
    property int py: 0
    property int pz: 0

    signal parametersChanged


    Label {
        id: labelTitle
        width: 85; height: 20
        text: qsTr("Смещение")
        anchors.left: parent.left
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        anchors.leftMargin: 0
        anchors.topMargin: 0
    }

    Label {
        id: labelX
        width: 25; height: 20
        text: roundJoystick.posX
        anchors.left: parent.left
        anchors.top: labelTitle.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.leftMargin: 0
        anchors.topMargin: 5
    }

    Label {
        id: labelY
        width: 25
        height: 20
        text: sliderY.value
        anchors.left: parent.left
        anchors.top: labelTitle.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.topMargin: 5
        anchors.leftMargin: 30
    }

    Slider {
        id: sliderY
        y: 70
        width: 25
        anchors.left: roundJoystick.right
        anchors.top: roundJoystick.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        anchors.bottomMargin: 0
        stepSize: 1
        from: 120
        to: -120
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 0
        onValueChanged: {
            py = value
            parametersChanged()
        }
    }

    Label {
        id: labelZ
        width: 25; height: 20
        text: roundJoystick.posZ
        anchors.left: parent.left
        anchors.top: labelTitle.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.leftMargin: 60
        anchors.topMargin: 5
    }

    Button {
        width: 140; height: 45
        text: qsTr("Сброс")
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 90
        anchors.topMargin: 0
        onClicked: {
            sliderY.value = 0;
            roundJoystick.move(0, 0)
        }
    }

    RoundJoystick {
        id: roundJoystick
        y: 150
        width: 200; height: 200
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        maxValue: 120
        onParametersChanged: {
            px = roundJoystick.posX
            pz = roundJoystick.posZ
            root.parametersChanged()
        }
    }

    Button {
        x: 260; y: 255
        width: 140; height: 45
        text: qsTr("DOWN")
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onClicked: {
            sliderY.value = 0
        }
    }

    Button {
        x: 260; y: 205
        width: 140; height: 45
        text: qsTr("UP")
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        onClicked: {
            sliderY.value = -85
        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:8}D{i:9}
}
##^##*/
