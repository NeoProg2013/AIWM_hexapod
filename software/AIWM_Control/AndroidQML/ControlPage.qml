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
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
        Label {
            x: 301
            y: 288
            width: 90
            height: 20
            font.family: fixedFont.name
            text: batteryCharge + "% " + batteryVoltage / 1000.0 + "V"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: (batteryCharge < 20) ? "#DD0000" : ((batteryCharge < 60) ? "#DDDD00" : "#00DD00")
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            anchors.rightMargin: 5
        }
        CP_StatusPanel {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 5
            anchors.topMargin: 5
            systemStatus: root.systemStatus
            moduleStatus: root.moduleStatus
        }
        CP_Orientation {
            y: 260
            height: 20
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5
        }
    }

    SwipeView {
        id: controlsSwipeView
        orientation: Qt.Vertical
        clip: true
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 320
        currentIndex: 1

        Item {
            CP_Joystick {
                id: joystick
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                onParametersChanged: {
                    CppSwlpService.setMotionParams(joystick.speed, joystick.distance, joystick.curvature,
                                                   joystick.stepHeight, joystick.isStabEnabled);
                }
            }
        }
        Item {
            CP_OffsetCtrl {
                id : offsetCtrl
                anchors.fill: parent
                anchors.topMargin: 5
                onParametersChanged: {
                    CppSwlpService.setSurfaceParams(offsetCtrl.px, offsetCtrl.py, offsetCtrl.pz,
                                                    rotateCtrl.rx, rotateCtrl.ry, rotateCtrl.rz);
                }
            }
        }
        Item {
            CP_RotateCtrl {
                id : rotateCtrl
                anchors.fill: parent
                anchors.topMargin: 5
                onParametersChanged: {
                    CppSwlpService.setSurfaceParams(offsetCtrl.px, offsetCtrl.py, offsetCtrl.pz,
                                                    rotateCtrl.rx, rotateCtrl.ry, rotateCtrl.rz);
                }
            }
        }
    }

}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000";formeditorZoom:1.25}D{i:3}
}
##^##*/
