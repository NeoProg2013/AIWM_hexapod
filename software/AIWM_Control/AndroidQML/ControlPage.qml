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

    function resetPage() {
        systemStatus = 0xFF
        moduleStatus = 0xFF
    }

    FontLoader {
        id: fixedFont
        source: "qrc:/fonts/OpenSans-Regular.ttf"
    }

    Connections {
        target: CppCore
        function onSystemStatusUpdated(newSystemStatus) {
            systemStatus = newSystemStatus
        }
        function onModuleStatusUpdated(newModuleStatus) {
            moduleStatus = newModuleStatus
        }
    }

    StreamWidget {
        id: streamWidget
        anchors.bottom: controls.top
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
    }

    SwipeView {
        id: controls
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
            Rectangle {
                id: joystickItem
                width: 270
                height: 270
                color: "#00000000"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                border.color: "#AAAAAA"

                ParallelAnimation {
                    id: animationReturn
                    NumberAnimation {
                        target: dragItem
                        property: "x"
                        to: joystickItem.width / 2 - dragItem.width / 2
                        duration: 100
                    }
                    NumberAnimation {
                        target: dragItem
                        property: "y"
                        to: joystickItem.height / 2 - dragItem.height / 2
                        duration: 100
                    }
                }

                ColorOverlay {
                    id: dragItem
                    width: 100
                    height: 100
                    source: joystickImage
                    color: "#FFFFFF"
                    smooth: true
                    antialiasing: true

                    x: (joystickItem.width - dragItem.width) / 2
                    y: (joystickItem.height - dragItem.height) / 2

                    MouseArea {
                        anchors.fill: parent
                        drag.target: dragItem
                        drag.axis: Drag.XAndYAxis
                        drag.minimumX: 0
                        drag.maximumX: joystickItem.width - dragItem.width
                        drag.minimumY: 0
                        drag.maximumY: joystickItem.height - dragItem.height

                        onReleased: {
                            animationReturn.start()
                            CppCore.sendStopMoveCommand()
                        }
                        onPositionChanged: {

                            var x = dragItem.x - (joystickItem.width / 2 - dragItem.width / 2)

                            var factor = 25
                            var scaled_x = Math.abs(x / factor)
                            var scaled_max_x = (drag.maximumX / 2) / factor

                            var curvature = Math.exp(scaled_x) / Math.exp(scaled_max_x) * 1999
                            curvature = Math.round(curvature)
                            if (x < 0) {
                                curvature = -curvature
                            }

                            var deadZoneHeight = 20
                            var minDeadZone = (joystickItem.height - dragItem.height - deadZoneHeight) / 2
                            var maxDeadZone = (joystickItem.height - dragItem.height + deadZoneHeight) / 2
                            var stepLength = 0
                            if (dragItem.y < minDeadZone || dragItem.y > maxDeadZone) {
                                stepLength = -(dragItem.y * (220.0 / drag.maximumY) - 110.0)
                                CppCore.sendStartMotionCommand(Math.round(stepLength), Math.round(curvature))
                            }
                        }
                    }
                }

                Image {
                    id: joystickImage
                    visible: false
                    source: "qrc:/images/joystick.svg"
                    sourceSize.width: dragItem.width
                    sourceSize.height: dragItem.height
                }
            }

            Label {
                height: 17
                text: motionSpeed.value
                anchors.top: parent.top
                horizontalAlignment: Text.AlignHCenter
                anchors.left: motionSpeed.left
                anchors.right: motionSpeed.right
            }

            Slider {
                id: motionSpeed
                x: 350
                y: 22
                width: 30
                height: 248
                stepSize: 1
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                orientation: Qt.Vertical
                to: 100
                value: 90
                onValueChanged: {
                    CppCore.setMotionSpeed(value)
                }
            }
        }
        Item {
            RowLayout {
                height: 40
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0

                ImageButton {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    imageSrc: "qrc:/images/getUp.svg"
                    onButtonPressed: {
                        CppCore.sendGetUpCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }

                ImageButton {
                    Layout.fillWidth: true
                    imageSrc: "qrc:/images/getDown.svg"
                    Layout.fillHeight: true
                    onButtonPressed: {
                        CppCore.sendGetDownCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }

                ImageButton {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    imageSrc: "qrc:/images/swordLeft.svg"
                    onButtonPressed: {
                        CppCore.sendAttackLeftCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }

                ImageButton {
                    Layout.fillWidth: true
                    imageSrc: "qrc:/images/swordRight.svg"
                    Layout.fillHeight: true
                    onButtonPressed: {
                        CppCore.sendAttackRightCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }
            }

            RowLayout {
                y: 45
                height: 40
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0

                ImageButton {
                    imageSrc: "qrc:/images/arrowPushPull.svg"
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onButtonPressed: {
                        CppCore.sendPushPullCommand()
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
                    imageSrc: "qrc:/images/arrowUpDown.svg"
                    imageRotate: 90
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onButtonPressed: {
                        CppCore.sendUpDownCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }

                ImageButton {
                    imageSrc: "qrc:/images/rotateX.svg"
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onButtonPressed: {
                        CppCore.sendRotateXCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }

                ImageButton {
                    imageSrc: "qrc:/images/rotateZ.svg"
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onButtonPressed: {
                        CppCore.sendRotateZCommand()
                    }
                    onButtonReleased: {
                        CppCore.sendStopMoveCommand()
                    }
                }
            }
            GridLayout {
                id: errorStatus
                y: 95
                width: 188
                height: 180
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                columnSpacing: 4
                rowSpacing: 4
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 0
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
                    text: "I2C\nerror"
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
                y: 95
                height: 180
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                rowSpacing: 4
                columnSpacing: 4
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 0
                rows: 4
                columns: 2

                StatusLabel {
                    text: "Camera"
                    Layout.minimumHeight: 40
                    Layout.maximumHeight: 40
                    Layout.preferredWidth: 118
                    Layout.fillWidth: true
                    isActive: moduleStatus & 0x80
                    deactiveColor: "#00DD00"
                }

                StatusLabel {
                    text: "GUI"
                    Layout.minimumHeight: 40
                    Layout.maximumHeight: 40
                    Layout.preferredWidth: 118
                    Layout.fillWidth: true
                    isActive: moduleStatus & 0x40
                    deactiveColor: "#00DD00"
                }

                StatusLabel {
                    text: "OLED GL"
                    Layout.minimumHeight: 40
                    Layout.maximumHeight: 40
                    Layout.preferredWidth: 118
                    Layout.fillWidth: true
                    isActive: moduleStatus & 0x20
                    deactiveColor: "#00DD00"
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
                    text: "Sequences\nengine"
                    Layout.minimumHeight: 40
                    Layout.maximumHeight: 40
                    Layout.preferredWidth: 118
                    Layout.fillWidth: true
                    isActive: moduleStatus & 0x08
                    deactiveColor: "#00DD00"
                }

                StatusLabel {
                    text: "Motion\ncore"
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
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000"}D{i:3}D{i:13}D{i:4}
}
##^##*/

