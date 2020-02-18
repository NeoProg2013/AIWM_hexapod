import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Item {

    property int systemStatus: 0xFF
    property int moduleStatus: 0xFF
    property real cellVoltage1: 0
    property real cellVoltage2: 0
    property real cellVoltage3: 0
    property real batteryVoltage: 0
    property int batteryCharge: 0

    id: root
    width: 500
    height: 888
    clip: true

    Connections {
        target: CppCore
        onSystemStatusUpdated: {
            systemStatus = newSystemStatus
        }
        onModuleStatusUpdated: {
            moduleStatus = newModuleStatus
        }
        onVoltageValuesUpdated: {
            cellVoltage1 = newCellVoltage1 / 1000.0
            cellVoltage2 = newCellVoltage2 / 1000.0
            cellVoltage3 = newCellVoltage3 / 1000.0
            batteryVoltage = newBatteryVoltage / 1000.0
        }
        onBatteryChargeUpdated: {
            batteryCharge = newBatteryCharge
        }
    }

    Item {
        id: joystickItem
        y: 653
        width: 325
        height: 270
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        ImageButton {
            x: 110
            y: 55
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowDoubleUp.svg"
            onButtonPressed: {
                CppCore.sendDirectMoveCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 110
            y: 220
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowDoubleDown.svg"
            onButtonPressed: {
                CppCore.sendReverseMoveCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 110
            y: 110
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowUp.svg"
            onButtonPressed: {
                CppCore.sendDirectMoveSlowCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 0
            y: 110
            width: 105
            height: 105
            imageSrc: "qrc:/images/arrowLeft.svg"
            onButtonPressed: {
                CppCore.sendShiftLeftCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 220
            y: 110
            width: 105
            height: 105
            imageSrc: "qrc:/images/arrowRight.svg"
            onButtonPressed: {
                CppCore.sendShiftRightCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 110
            y: 165
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowDown.svg"
            onButtonPressed: {
                CppCore.sendReverseMoveSlowCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 0
            y: 0
            width: 105
            height: 50
            imageSrc: "qrc:/images/swordLeft.svg"
            onButtonPressed: {
                CppCore.sendAttackLeftCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 220
            y: 0
            width: 105
            height: 50
            imageSrc: "qrc:/images/swordRight.svg"
            onButtonPressed: {
                CppCore.sendAttackRightCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 220
            y: 220
            width: 105
            height: 50
            imageSrc: "qrc:/images/getUp.svg"
            onButtonClicked: {
                CppCore.sendGetUpCommand()
            }
        }

        ImageButton {
            x: 0
            y: 220
            width: 105
            height: 50
            imageSrc: "qrc:/images/getDown.svg"
            onButtonClicked: {
                CppCore.sendGetDownCommand()
            }
        }

        ImageButton {
            x: 110
            y: 0
            width: 105
            height: 50
            imageSrc: "qrc:/images/run.svg"
            onButtonPressed: {
                CppCore.sendRunCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 0
            y: 55
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowRotateLeft.svg"
            onButtonPressed: {
                CppCore.sendRotateLeftCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            x: 220
            y: 55
            width: 105
            height: 50
            imageSrc: "qrc:/images/arrowRotateRight.svg"
            onButtonPressed: {
                CppCore.sendRotateRightCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }
    }

    Item {
        id: voltage
        height: 22
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: errorStatus.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10

        Label {
            width: 110
            height: 22
            anchors.left: parent.left
            anchors.top: parent.top
            font.pointSize: 12
            text: qsTr("Battery charge:")
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        ProgressBar {
            id: batteryVoltageProgressBar
            width: 165
            height: 22
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: 55
            anchors.left: parent.left
            anchors.leftMargin: 115
            value: batteryCharge
            to: 100
        }

        Label {
            width: 50
            height: 22
            text: batteryVoltageProgressBar.value + "%"
            anchors.top: parent.top
            anchors.right: parent.right
            font.pointSize: 12
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    GridLayout {
        id: errorStatus
        height: 175
        columnSpacing: 4
        rowSpacing: 4
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        rows: 4
        columns: 2

        StatusLabel {
            text: "Connection\nlost"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x80
        }

        StatusLabel {
            text: "Reserved"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x40
        }

        StatusLabel {
            text: "Math\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x20
        }

        StatusLabel {
            text: "Sync\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x10
        }

        StatusLabel {
            text: "Voltage\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x08
        }

        StatusLabel {
            text: "Memory\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x04
        }

        StatusLabel {
            text: "Configuration\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x02
        }

        StatusLabel {
            text: "Fatal\nerror"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: systemStatus & 0x01
        }
    }

    GridLayout {
        id: moduleStatus
        height: 175
        rowSpacing: 4
        columnSpacing: 4
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 2
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        rows: 4
        columns: 2

        StatusLabel {
            text: "Reserved"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x80
            //deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Reserved"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x40
            //deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Reserved"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x20
            //deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "System\nmonitor"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x10
            deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Movement\nengine"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x08
            deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Limbs\ndriver"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x04
            deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Servo\ndriver"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x02
            deactiveColor: "#00DD00"
        }

        StatusLabel {
            text: "Configurator"
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40
            Layout.preferredWidth: 118
            Layout.fillWidth: true
            isActive: moduleStatus & 0x01
            deactiveColor: "#00DD00"
        }
    }

    RowLayout {
        height: 50
        anchors.top: voltage.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        ImageButton {
            Layout.fillHeight: true
            Layout.fillWidth: true
            imageSrc: "qrc:/images/rotateX.svg"
            onButtonPressed: {
                CppCore.sendRotateXCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            Layout.fillWidth: true
            imageSrc: "qrc:/images/rotateZ.svg"
            Layout.fillHeight: true
            onButtonPressed: {
                CppCore.sendRotateZCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: "qrc:/images/dance.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppCore.sendDanceCommand()
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
            }
        }

        ImageButton {
            property bool isCommandSended: false
            imageSrc: "qrc:/images/light.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                if (!isCommandSended) {
                    CppCore.sendSwitchLightCommand()
                }
                isCommandSended = true
            }
            onButtonReleased: {
                CppCore.sendStopMoveCommand()
                isCommandSended = false
            }
        }
    }
}
