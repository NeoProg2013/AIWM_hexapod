import QtQuick 2.12
import QtQuick.Controls 2.15


Item {
    id: root
    width: 400
    height: 300
    clip: true

    property int rx: 0
    property int ry: 0
    property int rz: 0
    property bool isStabEnabled: false

    signal parametersChanged


    Label {
        id: labelTitle
        width: 85; height: 20
        text: qsTr("Вращение")
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
        width: 25; height: 20
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
        to: 360
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 0
        onValueChanged: {
            ry = value
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
        maxValue: 15
        onParametersChanged: {
            rx = roundJoystick.posX
            rz = roundJoystick.posZ
            root.parametersChanged()
        }
    }

    Switch {
        x: 306; y: 252
        text: qsTr("СТАБ")
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onCheckedChanged: {
            isStabEnabled = checked
            parametersChanged()
        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:1}D{i:2}D{i:5}D{i:6}
}
##^##*/
