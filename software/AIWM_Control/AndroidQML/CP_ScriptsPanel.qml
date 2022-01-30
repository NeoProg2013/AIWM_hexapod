import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 400
    height: 85
    clip: true

    RowLayout {
        height: 40
        anchors.right: parent.right
        anchors.left: parent.left

        ImageButton {
            Layout.fillHeight: true
            Layout.fillWidth: true
            imageSrc: "qrc:/images/getUp.svg"
            onButtonPressed: {
                CppSwlpService.sendGetUpCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            Layout.fillWidth: true
            imageSrc: "qrc:/images/getDown.svg"
            Layout.fillHeight: true
            onButtonPressed: {
                CppSwlpService.sendGetDownCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: qsTr("qrc:/images/pushPull.svg")
            Layout.fillHeight: true
            Layout.fillWidth: true
            onButtonPressed: {
                CppSwlpService.sendPushPullCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            Layout.fillWidth: true
            imageSrc: "qrc:/images/upDown.svg"
            Layout.fillHeight: true
            onButtonPressed: {
                CppSwlpService.sendUpDownCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }
    }

    RowLayout {
        y: 45
        height: 40
        anchors.left: parent.left
        anchors.right: parent.right

        ImageButton {
            imageSrc: "qrc:/images/square.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppSwlpService.sendSquareCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: "qrc:/images/swayX.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppSwlpService.sendXSwayCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: "qrc:/images/rotateXY.svg"
            imageRotate: 90
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppSwlpService.sendXYRotateCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: "qrc:/images/rotateX.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppSwlpService.sendXRotateCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }

        ImageButton {
            imageSrc: "qrc:/images/rotateZ.svg"
            Layout.fillHeight: true
            Layout.fillWidth: true

            onButtonPressed: {
                CppSwlpService.sendZRotateCommand()
            }
            onButtonReleased: {
                CppSwlpService.sendStopMoveCommand()
            }
        }
    }
}
