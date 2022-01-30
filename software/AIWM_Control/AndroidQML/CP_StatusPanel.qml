import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 315
    height: 75
    clip: true

    property int systemStatus: 0xFF
    property int moduleStatus: 0xFF
    property bool isTimerVisible: false

    Timer {
        interval: 500
        repeat: true
        running: true
        onTriggered: {
            isTimerVisible = !isTimerVisible
        }
    }


    ImageButton {
        x: 0
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/FatalError.svg"
        visible: isTimerVisible && (systemStatus & 0x01)
    }
    ImageButton {
        x: 40
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/InternalError.svg"
        visible: isTimerVisible && (systemStatus & 0x02)
    }
    ImageButton {
        x: 80
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/VoltageError.svg"
        visible: isTimerVisible && (systemStatus & 0x04)
    }
    ImageButton {
        x: 120
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/SyncError.svg"
        visible: isTimerVisible && (systemStatus & 0x08)
    }
    ImageButton {
        x: 160
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/MathError.svg"
        visible: isTimerVisible && (systemStatus & 0x10)
    }
    ImageButton {
        x: 200
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/I2CError.svg"
        visible: isTimerVisible && (systemStatus & 0x20)
    }
    ImageButton {
        x: 240
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/FatalError.svg"
        visible: false
    }
    ImageButton {
        x: 280
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/MathError.svg"
        visible: false
    }


    ImageButton {
        x: 0
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/MotionCoreError.svg"
        visible: isTimerVisible && (moduleStatus & 0x01)
    }
    ImageButton {
        x: 40
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/ServoDriverError.svg"
        visible: isTimerVisible && (moduleStatus & 0x02)
    }
    ImageButton {
        x: 80
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/SystemMonitorError.svg"
        visible: isTimerVisible && (moduleStatus & 0x04)
    }
    ImageButton {
        x: 120
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/DisplayError.svg"
        visible: isTimerVisible && (moduleStatus & 0x08)
    }
    ImageButton {
        x: 160
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/MPU6050Error.svg"
        visible: isTimerVisible && (moduleStatus & 0x10)
    }
    ImageButton {
        x: 200
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        imageSrc: "qrc:/images/status/StepDetectorError.svg"
        visible: isTimerVisible && (moduleStatus & 0x20)
    }
    ImageButton {
        x: 240
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/FatalError.svg"
        //visible: isTimerVisible && (moduleStatus & 0x40)
        visible: false
    }
    ImageButton {
        x: 280
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/MathError.svg"
        //visible: isTimerVisible && (moduleStatus & 0x80)
        visible: false
    }
}
