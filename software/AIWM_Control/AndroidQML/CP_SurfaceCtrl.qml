import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Extras 1.4


Item {
    id: root
    width: 400
    height: 600
    clip: true

    property int rx: 0
    property int ry: 0
    property int rz: 0

    property int px: 0
    property int py: 0
    property int pz: 0

    property bool isStabEnabled: false

    signal parametersChanged

    Label {
        id: labelRotate
        width: 85
        height: 20
        text: qsTr("Вращение")
        anchors.left: parent.left
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        id: labelRotateX
        width: 25
        height: 20
        text: sliderRotateX.value
        anchors.left: parent.left
        anchors.top: labelRotate.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.topMargin: 5
    }

    Slider {
        id: sliderRotateX
        width: 25
        anchors.left: parent.left
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        stepSize: 1
        to: 15
        from: -15
        anchors.topMargin: 30
        orientation: Qt.Vertical
        value: 0
        onValueChanged: {
            rx = value
            parametersChanged()
        }
    }

    Label {
        id: labelRotateY
        width: 25
        height: 20
        text: sliderRotateY.value
        anchors.left: parent.left
        anchors.top: labelRotate.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.leftMargin: 30
        anchors.topMargin: 5
    }

    Slider {
        id: sliderRotateY
        y: 70
        width: 25
        anchors.left: sliderRotateX.right
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        stepSize: 1
        to: 360
        anchors.leftMargin: 5
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 30
        onValueChanged: {
            ry = value
            parametersChanged()
        }
    }

    Label {
        id: labelRotateZ
        width: 25
        height: 20
        text: sliderRotateZ.value
        anchors.left: parent.left
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        anchors.leftMargin: 60
        anchors.topMargin: 25
    }

    Slider {
        id: sliderRotateZ
        y: 70
        width: 25
        anchors.left: sliderRotateY.right
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        stepSize: 1
        to: 15
        from: -15
        anchors.leftMargin: 5
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 30
        onValueChanged: {
            rz = value
            parametersChanged()
        }
    }

    Label {
        id: labelPoint
        width: 85
        height: 20
        text: qsTr("Сдвиг")
        anchors.right: parent.right
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        id: labelPointX
        x: 315
        width: 25
        height: 20
        text: sliderPointX.value
        anchors.right: parent.right
        anchors.top: labelPoint.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.rightMargin: 60
        anchors.topMargin: 5
    }

    Slider {
        id: sliderPointX
        x: 315
        width: 25
        anchors.right: sliderPointY.left
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        to: 150
        from: -150
        stepSize: 1
        anchors.rightMargin: 5
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 30
        onValueChanged: {
            px = value
            parametersChanged()
        }
    }

    Label {
        id: labelPointY
        x: 345
        width: 25
        height: 20
        text: sliderPointY.value
        anchors.right: parent.right
        anchors.top: labelPoint.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.rightMargin: 30
        anchors.topMargin: 5
    }

    Slider {
        id: sliderPointY
        x: 345
        y: 70
        width: 25
        anchors.right: sliderPointZ.left
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        to: -150
        from: 150
        stepSize: 1
        anchors.rightMargin: 5
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 30
        onValueChanged: {
            py = value
            parametersChanged()
        }
    }

    Label {
        id: labelPointZ
        x: 375
        width: 25
        height: 20
        text: sliderPointZ.value
        anchors.right: parent.right
        anchors.top: labelPoint.bottom
        horizontalAlignment: Text.AlignHCenter
        anchors.topMargin: 5
    }

    Slider {
        id: sliderPointZ
        x: 375
        y: 70
        width: 25
        anchors.right: parent.right
        anchors.top: labelRotate.bottom
        anchors.bottom: parent.bottom
        to: 150
        from: -150
        stepSize: 1
        value: 0
        orientation: Qt.Vertical
        anchors.topMargin: 30
        onValueChanged: {
            pz = value
            parametersChanged()
        }
    }

    Button {
        y: 0
        height: 48
        text: qsTr("Сброс")
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.leftMargin: 90
        anchors.rightMargin: 90
        onClicked: {
            sliderRotateX.value = 0;
            sliderRotateY.value = 0;
            sliderRotateZ.value = 0;
            sliderPointX.value = 0;
            sliderPointY.value = 0;
            sliderPointZ.value = 0;
        }
    }

    Button {
        y: 499
        height: 48
        text: qsTr("DOWN")
        anchors.left: parent.horizontalCenter
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 53
        anchors.leftMargin: 5
        anchors.rightMargin: 90
        onClicked: {
            sliderPointY.value = 0;
        }
    }

    Button {
        y: 499
        height: 48
        text: qsTr("UP")
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.rightMargin: 5
        anchors.bottomMargin: 53
        anchors.leftMargin: 90
        onClicked: {
            sliderPointY.value = -85;
        }
    }

    Switch {
        id: stabSwitch
        y: 446
        width: 220
        height: 48
        text: qsTr("Стабилизация")
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 90
        anchors.leftMargin: 90
        anchors.bottomMargin: 106
        onCheckedChanged: {
            isStabEnabled = checked
            parametersChanged()
        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:16}D{i:17}D{i:18}
}
##^##*/
