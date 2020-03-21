import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Item {

	function resetTerminalInterface() {
		terminalLog.text = ""
		messageText.text = ""
	}

	Connections {
		target: CppTerminal
		onMessageReceived: {
			terminalLog.text += message
		}
	}

	FontLoader {
		id: fontFromResources
		source: "qrc:/fonts/RobotoMono-Regular.ttf"
	}

	Flickable {
		id: logFlickable
		clip: true
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 53
		anchors.top: parent.top
		anchors.topMargin: 58
		anchors.right: parent.right
		anchors.rightMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 5
		contentHeight: terminalLog.contentHeight
		contentWidth: terminalLog.contentWidth

		Label {
			id: terminalLog
			font.family: fontFromResources.name
			color: "#FFFFFF"
			text: "TERMINAL LOG STARTED\n\n"
			onContentHeightChanged: {
				logFlickable.contentX = 0
				logFlickable.contentY = (contentHeight > logFlickable.height) ? -(logFlickable.height - contentHeight) : 0
			}
		}
	}

	FocusScope {
		id: messageEditBlock
		height: 35
		anchors.verticalCenter: sendButton.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 80
		anchors.left: parent.left
		anchors.leftMargin: 5

		Rectangle {
			anchors.fill: parent
		}

		TextInput {
			id: messageText
			clip: true
			anchors.fill: parent
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			font.family: fontFromResources.name
			selectByMouse: true
			Keys.onReturnPressed: {
				terminalLog.text += "TERMINAL: " + text + "\n"
				CppTerminal.sendMessage(text)
				messageText.text = ""
			}
		}
	}

	Button {
		id: sendButton
		width: 70
		height: 48
		text: "SEND"
		clip: true
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 2
		anchors.right: parent.right
		anchors.rightMargin: 5
		onClicked: {
			terminalLog.text += "TERMINAL: " + messageText.text + "\n"
			CppTerminal.sendMessage(messageText.text)
			messageText.text = ""
		}
	}

	Button {
		width: 70
		height: 48
		text: "CLEAR"
		anchors.top: parent.top
		anchors.topMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 5
		clip: true
		onClicked: {
			terminalLog.text = ""
		}
	}

	Button {
		y: 1
		width: 100
		height: 48
		text: "COPY LOG"
		anchors.left: parent.left
		anchors.leftMargin: 80
		anchors.top: parent.top
		anchors.topMargin: 5
		clip: true
		onClicked: {
			CppClipboard.insertText(terminalLog.text)
		}
	}
}

/*##^## Designer {
	D{i:0;autoSize:true;height:480;width:640}D{i:9;anchors_x:5;anchors_y:0}D{i:10;anchors_x:80;anchors_y:0}
}
 ##^##*/

