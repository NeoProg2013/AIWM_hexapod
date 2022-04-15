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

    function sendMotionCmd() {
        CppSwlpService.sendMotionCommand(joystick.speed, joystick.distance, joystick.curvature, joystick.stepHeight,
                                         surfaceCtrl.px, surfaceCtrl.py, surfaceCtrl.pz,
                                         surfaceCtrl.rx, surfaceCtrl.ry, surfaceCtrl.rz,
                                         surfaceCtrl.isStabEnabled)
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
                    sendMotionCmd()
                }
            }
        }
        Item {
            CP_ScriptsPanel {
                id: scriptPanel
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.top: parent.top
            }


            CP_SurfaceCtrl {
                id : surfaceCtrl
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: scriptPanel.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 5
                onParametersChanged: {
                    sendMotionCmd()
                }
            }
        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:6}
}
##^##*/
