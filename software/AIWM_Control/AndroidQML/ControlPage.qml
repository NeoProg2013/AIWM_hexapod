import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Extras 1.4


Item {
    id: root
    width: 400
    height: 600
    clip: true

    property int systemStatus: 0xFF
    property int moduleStatus: 0xFF
    property int batteryCharge: 100
    property int batteryVoltage: 12600

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Connections {
        target: CppSwlpService
        function onSystemStatusUpdated(newSystemStatus, newModuleStatus) {
            systemStatus = newSystemStatus
            moduleStatus = newModuleStatus
        }
        function onBatteryStatusUpdated(newBatteryCharge, newBatteryVoltage) {
            batteryCharge = newBatteryCharge
            batteryVoltage = newBatteryVoltage
        }
        function onConnectionClosed() {
            systemStatus = 0xFF
            moduleStatus = 0xFF
        }
    }


    StreamWidget {
        id: streamWidget
        anchors.bottom: controlsSwipeView.top
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        Label {
            width: 90
            height: 17
            font.family: fixedFont.name
            text: batteryCharge + "% (" + batteryVoltage / 1000.0 + "V)"
            color: (batteryCharge < 20) ? "#DD0000" : ((batteryCharge < 60) ? "#DDDD00" : "#00DD00")
            anchors.right: parent.right
            anchors.top: parent.top
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 5
            anchors.rightMargin: 5
        }
        CP_StatusPanel {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            systemStatus: root.systemStatus
            moduleStatus: root.moduleStatus
        }
    }

    SwipeView {
        id: controlsSwipeView
        orientation: Qt.Vertical
        clip: true
        height: 270
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: 1

        Item {
            CP_Joystick {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
        Item {
            CP_ScriptsPanel {
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.top: parent.top
            }
/*

            Label {
                id: labelRotate
                y: 90
                width: 85
                height: 17
                text: qsTr("Вращение")
                anchors.left: parent.left
                horizontalAlignment: Text.AlignHCenter
                anchors.leftMargin: 0
            }

            Slider {
                id: sliderRotateX
                width: 25
                anchors.left: parent.left
                anchors.top: labelRotate.bottom
                anchors.bottom: parent.bottom
                stepSize: 1
                to: 15
                anchors.leftMargin: 0
                anchors.bottomMargin: 0
                anchors.topMargin: 5
                orientation: Qt.Vertical
                value: 0
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
                anchors.bottomMargin: 0
                anchors.topMargin: 5
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
                anchors.leftMargin: 5
                value: 0
                orientation: Qt.Vertical
                anchors.bottomMargin: 0
                anchors.topMargin: 5
            }

            Label {
                id: labelPoint
                y: 90
                width: 85
                height: 17
                text: qsTr("Сдвиг")
                anchors.right: parent.right
                horizontalAlignment: Text.AlignHCenter
                anchors.rightMargin: 0
            }

            Slider {
                id: sliderPointX
                width: 25
                anchors.right: sliderPointY.left
                anchors.top: labelRotate.bottom
                anchors.bottom: parent.bottom
                to: 150
                stepSize: 1
                anchors.rightMargin: 5
                value: 0
                orientation: Qt.Vertical
                anchors.bottomMargin: 0
                anchors.topMargin: 5
            }

            Slider {
                id: sliderPointY
                y: 70
                width: 25
                anchors.right: sliderPointZ.left
                anchors.top: labelRotate.bottom
                anchors.bottom: parent.bottom
                to: 150
                stepSize: 1
                anchors.rightMargin: 5
                value: 0
                orientation: Qt.Vertical
                anchors.bottomMargin: 0
                anchors.topMargin: 5
            }

            Slider {
                id: sliderPointZ
                x: 355
                y: 70
                width: 25
                anchors.right: parent.right
                anchors.top: labelRotate.bottom
                anchors.bottom: parent.bottom
                to: 150
                stepSize: 1
                anchors.rightMargin: 0
                value: 0
                orientation: Qt.Vertical
                anchors.bottomMargin: 0
                anchors.topMargin: 5
            }*/

        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:6}D{i:3}
}
##^##*/
