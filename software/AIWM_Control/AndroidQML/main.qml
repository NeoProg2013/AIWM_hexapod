import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
	visible: true
	width: 500
	height: 888
	color: "#000000"

	onClosing: {
		if (swipeView.currentIndex != 0) {
			close.accepted = false
			CppCore.stopCommunication()
			swipeView.currentIndex = 0
		} else {
			close.accepted = true
		}
	}

	SwipeView {
		id: swipeView
		anchors.fill: parent
		currentIndex: 0
		interactive: false

		ConnectionPage {
			onShowControlPage: {
				swipeView.currentIndex = 1
			}
		}
		ControlPage {
		}
	}
}
