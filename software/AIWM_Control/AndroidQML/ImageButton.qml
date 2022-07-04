import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Button {

    signal buttonClicked
    signal buttonPressed
    signal buttonReleased

    property string imageSrc: ""
    property string imageColor: "#FFFFFF"
    property string borderColor: "#AAAAAA"
    property int imageRotate: 0

    id: root
    width: 80
    height: 80
    background: Item {

        anchors.fill: root

        Rectangle {
            id: frameRectangle
            anchors.fill: parent
            color: "#00000000"
            border.color: borderColor
            border.width: 2
        }
        Image {
            id: buttonImage
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: (parent.width > parent.height) ? (parent.height * 0.75) : (parent.width * 0.75)
            height: width
            sourceSize.width: width
            sourceSize.height: width
            source: imageSrc
        }
        ColorOverlay {
            anchors.fill: buttonImage
            smooth: true
            antialiasing: true
            source: buttonImage
            color: imageColor
            rotation: imageRotate
        }
    }

    onClicked: {
        root.buttonClicked()
    }
    onPressedChanged: {
        if (pressed) {
            frameRectangle.border.color = borderColor
            root.buttonPressed()
        } else {
            frameRectangle.border.color = "#AAAAAA"
            root.buttonReleased()
        }
    }
}
