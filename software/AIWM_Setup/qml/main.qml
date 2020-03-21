import QtQuick 2.14
import QtQuick.Controls 2.12

ApplicationWindow {
	id: applicationWindow
	visible: true
	width: 1200
	height: 690
	color: "#000000"

	TabBar {
		id: tabBar
		height: 48
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.left: parent.left

		TabButton {
			text: qsTr("Сервоприводы")
		}

		TabButton {
			text: qsTr("Кинематика")
		}

		TabButton {
			text: qsTr("Терминал")
		}
	}

	SwipeView {
		id: swipeView
		clip: true
		interactive: false
		anchors.top: tabBar.bottom
		anchors.topMargin: 5
		currentIndex: tabBar.currentIndex
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.left: parent.left
		Item {
			ServoConfigPage {}
		}
		Item {
			Label {
				text: "2"
				color: "#FFFFFF"
			}
		}
		Item {
			Label {
				text: "3"
				color: "#FFFFFF"
			}
		}
	}
}

/*##^##
Designer {
	D{i:1;anchors_width:571;anchors_x:80;anchors_y:294}D{i:5;anchors_height:695;anchors_width:200;anchors_x:85;anchors_y:53}
}
##^##*/

