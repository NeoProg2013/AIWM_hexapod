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


    StatusImage {
        x: 280
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x01) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x01) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/FatalError.svg"
    }
    StatusImage {
        x: 240
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x02) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x02) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/InternalError.svg"
    }
    StatusImage {
        x: 200
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x04) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x04) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/VoltageError.svg"
    }
    StatusImage {
        x: 160
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x08) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x08) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/SyncError.svg"
    }
    StatusImage {
        x: 120
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x10) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x10) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/MathError.svg"
    }
    StatusImage {
        x: 80
        y: 0
        width: 35
        height: 35
        imageColor: isTimerVisible && (systemStatus & 0x20) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (systemStatus & 0x20) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/I2CError.svg"
    }
    StatusImage {
        x: 40
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/FatalError.svg"
        visible: false
    }
    StatusImage {
        x: 0
        y: 0
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/MathError.svg"
        visible: false
    }


    StatusImage {
        x: 280
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x01) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x01) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/MotionCoreError.svg"
    }
    StatusImage {
        x: 240
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x02) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x02) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/ServoDriverError.svg"
    }
    StatusImage {
        x: 200
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x04) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x04) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/SystemMonitorError.svg"
    }
    StatusImage {
        x: 160
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x08) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x08) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/DisplayError.svg"
    }
    StatusImage {
        x: 120
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x10) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x10) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/MPU6050Error.svg"
    }
    StatusImage {
        x: 80
        y: 40
        width: 35
        height: 35
        imageColor: isTimerVisible && (moduleStatus & 0x20) ? "#CC0000" : "#222222"
        borderColor: isTimerVisible && (moduleStatus & 0x20) ? "#CC0000" : "#222222"
        imageSrc: "qrc:/images/status/StepDetectorError.svg"
    }
    StatusImage {
        x: 40
        y: 40
        width: 35
        height: 35
        imageColor: "#CC0000"
        borderColor: "#CC0000"
        //imageSrc: "qrc:/images/status/FatalError.svg"
        //visible: isTimerVisible && (moduleStatus & 0x40)
        visible: false
    }
    StatusImage {
        x: 0
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

/*##^##
Designer {
    D{i:0;formeditorColor:"#000000";formeditorZoom:1.75}
}
##^##*/
