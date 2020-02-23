import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
	id: root
	width: 500
	height: 888
	clip: true

	signal showControlPage
	signal showTerminalPage

	Connections {
		target: CppCore
		onFrameReceived: {
			labelText.text = ""
			labelText.color = "#FFFFFF"
			connectButton.visible = true
			progressBar.visible = false
			terminalButton.visible = true
			timeoutTimer.stop()
			showControlPage()
		}
	}

	Timer {
		id: timeoutTimer
		interval: 5000
		repeat: false
		onTriggered: {
			errorLabel.visible = true
			labelText.visible = false
			connectButton.visible = true
			progressBar.visible = false
			terminalButton.visible = true

			CppCore.stopCommunication()
		}
	}

	Label {
		id: labelText
		height: 30
		text: ""
		visible: false
		anchors.bottom: progressBar.top
		anchors.bottomMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 10
		anchors.right: parent.right
		anchors.rightMargin: 10
		font.pointSize: 14
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
	}

	Button {
		id: connectButton
		width: 140
		height: 150
		text: qsTr("")
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.verticalCenter: parent.verticalCenter
		background: Item {
			Image {
				id: buttonImage
				visible: true
				anchors.fill: parent
				source: "qrc:/images/powerOn.svg"
				sourceSize.width: parent.width
				sourceSize.height: parent.height
			}
			ColorOverlay {
				anchors.fill: parent
				smooth: true
				antialiasing: true
				source: buttonImage
				color: "#00FFFF"
			}
		}
		onClicked: {
			labelText.text = "Подключение к устройству..."
			labelText.color = "#FFFFFF"
			labelText.visible = true
			connectButton.visible = false
			progressBar.visible = true
			terminalButton.visible = false

			CppCore.runCommunication()
			timeoutTimer.start()
		}
	}

	ProgressBar {
		id: progressBar
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.verticalCenter: parent.verticalCenter
		visible: false
		indeterminate: true
	}

	Label {
		y: 863
		width: 120
		height: 20
		text: qsTr("Версия: 1.00")
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 5
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignLeft
	}

	Label {
		id: errorLabel
		height: 30
		color: "#FF0000"
		text: "Ошибка при подключении к устройству"
		visible: false
		anchors.top: connectButton.bottom
		anchors.topMargin: 20
		anchors.right: parent.right
		anchors.rightMargin: 10
		anchors.left: parent.left
		anchors.leftMargin: 10
		font.pointSize: 14
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
	}

	Button {
		id: terminalButton
		x: 409
		y: 835
		text: qsTr("Терминал")
		anchors.right: parent.right
		anchors.rightMargin: 10
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 5
		onClicked: {
			CppTerminal.runCommunication()
			showTerminalPage()
		}
	}

	Text {
		color: "#00ffff"
		text: "AIWM CONTROL"
		anchors.top: parent.top
		anchors.topMargin: 15
		anchors.right: parent.right
		anchors.rightMargin: 15
		anchors.left: parent.left
		anchors.leftMargin: 15
		font.pixelSize: 40
		fontSizeMode: Text.Fit
		verticalAlignment: Text.AlignTop
		horizontalAlignment: Text.AlignHCenter
	}
}

/*##^## Designer {
	D{i:2;anchors_y:407}D{i:5;anchors_x:5}D{i:8;anchors_width:100;anchors_x:5;anchors_y:863}
D{i:9;anchors_y:2}D{i:12;anchors_x:239;anchors_y:15}
}
 ##^##*/

