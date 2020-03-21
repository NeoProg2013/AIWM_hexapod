import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Button {

	signal buttonClicked

	property string imageSrc: ""
	property string imageColor: "#FFFFFF"

	id: root
	width: 80
	height: 80
	background: Item {

		anchors.fill: root

		Rectangle {
			id: frameRectangle
			anchors.fill: parent
			color: "#2B96CB"
			border.width: 0
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
		}
	}

	onClicked: {
		root.buttonClicked()
	}
	onPressedChanged: {

		if (pressed) {
			frameRectangle.color = "#4D95D1"
		} else {
			frameRectangle.color = "#2B96CB"
		}
	}
}
