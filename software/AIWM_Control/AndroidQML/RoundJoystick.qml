import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Extras 1.4
import QtQuick.Shapes 1.15

Rectangle {
    id: root
    clip: true
    width: 150; height: 150
    color: "transparent"

    property int posX: 0
    property int posZ: 0
    property int maxValue: root.width

    signal parametersChanged

    function move(x, z) {
        dragItem.x = x + root.width / 2 - dragItem.width / 2
        dragItem.y = root.height / 2 - dragItem.height / 2 - z
        dragMouseArea.positionChanged(0, 0)
    }

    Shape {
        anchors.fill: parent
        smooth: true
        antialiasing: true
        ShapePath {
            id: shapePath
            fillColor: "transparent"
            strokeColor: "white"
            strokeWidth: 1

            property int padding: 5


            startX: shapePath.padding
            startY: root.height / 2 + shapePath.padding
            PathArc {
                x: root.width - shapePath.padding
                y: root.height / 2
                radiusX: 1; radiusY: 1
                useLargeArc: true
            }
            PathArc {
                x: shapePath.padding
                y: root.height / 2 - shapePath.padding
                radiusX: 1; radiusY: 1
                useLargeArc: true
            }
        }
    }

    ColorOverlay {
        id: dragItem
        width: 50; height: 50
        source: joystickImage
        color: "#FFFFFF"
        smooth: true
        antialiasing: true

        x: (root.width - dragItem.width) / 2
        y: (root.height - dragItem.height) / 2

        MouseArea {
            id: dragMouseArea
            anchors.fill: parent
            drag.target: dragItem
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.maximumX: root.width - dragItem.width
            drag.minimumY: 0
            drag.maximumY: root.height - dragItem.height

            onPositionChanged: {
                var x = dragItem.x - root.width / 2 + dragItem.width / 2
                var y = -(dragItem.y - root.height / 2 + dragItem.height / 2)
                var vectorLen = Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2))

                var optimalVertorX = (root.width / 2 + shapePath.padding) * Math.cos(Math.PI / 4) - dragItem.width / 2
                var optimalVertorY = (root.height / 2 + shapePath.padding) * Math.sin(Math.PI / 4) - dragItem.height / 2
                var optimalVertorLen = Math.sqrt(Math.pow(optimalVertorX, 2) + Math.pow(optimalVertorY, 2))
                if (vectorLen > optimalVertorLen) {
                    var k = optimalVertorLen / vectorLen
                    x = x * k
                    y = y * k
                    vectorLen = Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2))

                    dragItem.x = x + root.width / 2 - dragItem.width / 2
                    dragItem.y = root.height / 2 - dragItem.height / 2 - y
                }

                var maxRotateAngleFactor = maxValue / (root.width / 2 - shapePath.padding - dragItem.width / 2)
                posX = Math.round(maxRotateAngleFactor * x)
                posZ = Math.round(maxRotateAngleFactor * y)

                parametersChanged()
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
