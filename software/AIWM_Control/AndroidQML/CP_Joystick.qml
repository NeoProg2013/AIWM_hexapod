import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Extras 1.4

Item {
    id: root
    width: 400
    height: 270
    clip: true

    Rectangle {
        id: joystickItem
        width: 270
        height: 270
        color: "#00000000"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
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
                    CppSwlpService.sendStopMoveCommand()
                }
                onPositionChanged: {
                    var x = dragItem.x - (joystickItem.width / 2 - dragItem.width / 2)
                    var k = (joystickItem.width - dragItem.width) / 2000
                    var curvature = Math.round(x / k)

                    var deadZoneHeight = 20
                    var minDeadZone = (joystickItem.height - dragItem.height - deadZoneHeight) / 2
                    var maxDeadZone = (joystickItem.height - dragItem.height + deadZoneHeight) / 2
                    var distance = 0
                    if (dragItem.y < minDeadZone || dragItem.y > maxDeadZone) {
                        distance = -(dragItem.y * (220.0 / drag.maximumY) - 110.0)
                        CppSwlpService.sendStartMotionCommand(motionSpeed.value, Math.round(distance), Math.round(curvature), stepHeight.value,
                                                              0,0,0,0,0,0)//sliderRotateX.value, sliderRotateY.value, sliderRotateZ.value,
                                                              //sliderPointX.value, sliderPointY.value, sliderPointZ.value)
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
        font.family: fixedFont.name
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        anchors.left: motionSpeed.left
        anchors.right: motionSpeed.right
    }
    Slider {
        id: motionSpeed
        width: 30
        height: 248
        stepSize: 1
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        orientation: Qt.Vertical
        to: 100
        value: 90
    }

    Label {
        height: 17
        text: stepHeight.value
        font.family: fixedFont.name
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        anchors.left: stepHeight.left
        anchors.right: stepHeight.right
    }
    Slider {
        id: stepHeight
        width: 30
        height: 248
        stepSize: 1
        anchors.bottom: parent.bottom
        anchors.right: motionSpeed.left
        orientation: Qt.Vertical
        from: 15
        to: 60
        value: 30
    }

}
