import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
	id: applicationWindow
	visible: true
	width: 600
	height: 600
	color: "#000000"

	onClosing: {
		if (swipeView.currentIndex === 2) {
			close.accepted = false
			CppCore.stopCommunication()
			swipeView.currentIndex = 1
		} else if (swipeView.currentIndex === 0) {
			close.accepted = false
			CppTerminal.stopCommunication()
			terminalPage.resetTerminalInterface()
			swipeView.currentIndex = 1
		} else {
			close.accepted = true
		}
	}

	SwipeView {
		id: swipeView
		anchors.fill: parent
		currentIndex: 1
		interactive: false

		TerminalPage {
			id: terminalPage
		}

		ConnectionPage {
			id: connectionPage
			onShowControlPage: {
				swipeView.currentIndex = 2
			}
			onShowTerminalPage: {
				swipeView.currentIndex = 0
			}
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

