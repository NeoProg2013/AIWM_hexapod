import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 400
    height: 600
    color: "#000000"

    onClosing: {
        if (swipeView.currentIndex === 1) {
            close.accepted = false
            CppCore.stopCommunication()
        } else {
            close.accepted = true
        }
    }

    Connections {
        target: CppCore
        function onSwlpFrameReceived() {
            swipeView.currentIndex = 1
        }
        function onSwlpConnectionClosed() {
            swipeView.currentIndex = 0
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: 0
        interactive: true
        onCurrentIndexChanged: {
            if (currentIndex == 0) {
                controlPage.reset()
            }
            if (currentIndex == 1) {
                connectionPage.reset()
            }
        }

        ConnectionPage {
            id: connectionPage
        }
        ControlPage {
            id: controlPage
        }
    }
}

/*##^## Designer {
    D{i:2;anchors_height:200;anchors_x:5;anchors_y:5}D{i:5;anchors_x:427}
}
 ##^##*/

