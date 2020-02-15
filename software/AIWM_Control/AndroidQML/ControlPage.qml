import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Item {

	property real systemStatus: 0xFFFFFFFF
	property real batteryVoltage: 0

	id: root
	width: 500
	height: 888
	clip: true

	Connections {
		target: CppCore
		onSystemStatusUpdatedSignal: {
			systemStatus = newSystemStatus
		}
		onSystemVoltageUpdatedSignal: {
			batteryVoltage = battery / 10.0
		}
	}

	Item {
		id: joystickItem
		y: 653
		width: 325
		height: 270
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 20

		ImageButton {
			x: 110
			y: 55
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowDoubleUp.svg"
			onButtonPressed: {
				CppCore.sendDirectMoveCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 110
			y: 220
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowDoubleDown.svg"
			onButtonPressed: {
				CppCore.sendReverseMoveCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 110
			y: 110
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowUp.svg"
			onButtonPressed: {
				CppCore.sendDirectMoveSlowCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 0
			y: 110
			width: 105
			height: 105
			imageSrc: "qrc:/images/arrowLeft.svg"
			onButtonPressed: {
				CppCore.sendShiftLeftCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 220
			y: 110
			width: 105
			height: 105
			imageSrc: "qrc:/images/arrowRight.svg"
			onButtonPressed: {
				CppCore.sendShiftRightCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 110
			y: 165
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowDown.svg"
			onButtonPressed: {
				CppCore.sendReverseMoveSlowCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 0
			y: 0
			width: 105
			height: 50
			imageSrc: "qrc:/images/swordLeft.svg"
			onButtonPressed: {
				CppCore.sendAttackLeftCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 220
			y: 0
			width: 105
			height: 50
			imageSrc: "qrc:/images/swordRight.svg"
			onButtonPressed: {
				CppCore.sendAttackRightCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 220
			y: 220
			width: 105
			height: 50
			imageSrc: "qrc:/images/getUp.svg"
			onButtonClicked: {
				CppCore.sendGetUpCommand()
			}
		}

		ImageButton {
			x: 0
			y: 220
			width: 105
			height: 50
			imageSrc: "qrc:/images/getDown.svg"
			onButtonClicked: {
				CppCore.sendGetDownCommand()
			}
		}

		ImageButton {
			x: 110
			y: 0
			width: 105
			height: 50
			imageSrc: "qrc:/images/run.svg"
			onButtonPressed: {
				CppCore.sendRunCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 0
			y: 55
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowRotateLeft.svg"
			onButtonPressed: {
				CppCore.sendRotateLeftCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			x: 220
			y: 55
			width: 105
			height: 50
			imageSrc: "qrc:/images/arrowRotateRight.svg"
			onButtonPressed: {
				CppCore.sendRotateRightCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}
	}

	Item {
		id: voltage
		height: 22
		anchors.right: parent.right
		anchors.rightMargin: 10
		anchors.top: errorStatus.bottom
		anchors.topMargin: 10
		anchors.left: parent.left
		anchors.leftMargin: 10

		Label {
			width: 110
			height: 22
			anchors.left: parent.left
			anchors.top: parent.top
			font.pointSize: 12
			text: qsTr("Battery charge:")
			horizontalAlignment: Text.AlignRight
			verticalAlignment: Text.AlignVCenter
		}

		ProgressBar {
			id: batteryVoltageProgressBar
			width: 165
			height: 22
			anchors.top: parent.top
			anchors.right: parent.right
			anchors.rightMargin: 55
			anchors.left: parent.left
			anchors.leftMargin: 115
			value: Math.round(((batteryVoltage - 5.6) / (8.4 - 5.6)) * 100.0)
			to: 100
		}

		Label {
			width: 50
			height: 22
			text: batteryVoltageProgressBar.value + "%"
			anchors.top: parent.top
			anchors.right: parent.right
			font.pointSize: 12
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
		}
	}

	GridLayout {
		id: errorStatus
		height: 175
		columnSpacing: 4
		rowSpacing: 4
		anchors.right: parent.horizontalCenter
		anchors.rightMargin: 2
		anchors.top: parent.top
		anchors.topMargin: 10
		anchors.left: parent.left
		anchors.leftMargin: 10
		rows: 4
		columns: 2

		StatusLabel {
			text: "I2C bus\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0080
		}

		StatusLabel {
			text: "Math\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0040
		}

		StatusLabel {
			text: "Sync\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0020
		}

		StatusLabel {
			text: "Voltage\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0010
		}

		StatusLabel {
			text: "Memory\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0008
		}

		StatusLabel {
			text: "Configuration\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0004
		}

		StatusLabel {
			text: "Internal\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0002
		}

		StatusLabel {
			text: "Emergency\nmode"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0001
		}
	}

	GridLayout {
		id: moduleStatus
		height: 175
		rowSpacing: 4
		columnSpacing: 4
		anchors.left: parent.horizontalCenter
		anchors.leftMargin: 2
		anchors.top: parent.top
		anchors.topMargin: 10
		anchors.right: parent.right
		anchors.rightMargin: 10
		rows: 4
		columns: 2

		StatusLabel {
			text: "Reserved"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x8000
			//deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Reserved"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x4000
			//deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "GUI"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x2000
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Monitoring"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x1000
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Movement\nengine"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0800
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Limbs\ndriver"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0400
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Servo\ndriver"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0200
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "VEEPROM"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x0100
			deactiveColor: "#00DD00"
		}
	}

	RowLayout {
		height: 50
		anchors.top: voltage.bottom
		anchors.topMargin: 10
		anchors.left: parent.left
		anchors.leftMargin: 10
		anchors.right: parent.right
		anchors.rightMargin: 10

		Switch {
			id: element
			text: qsTr("Front sensor")
			checked: true
			onCheckedChanged: {

				if (checked) {
					if (!CppCore.sendEnableFrontDistanceSensorCommand()) {
						checked = false
					}
				} else {
					if (!CppCore.sendDisableFrontDistanceSensorCommand()) {
						checked = true
					}
				}
			}
		}

		ImageButton {
			Layout.fillHeight: true
			Layout.fillWidth: true
			imageSrc: "qrc:/images/rotateX.svg"
			onButtonPressed: {
				CppCore.sendRotateXCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			Layout.fillWidth: true
			imageSrc: "qrc:/images/rotateZ.svg"
			Layout.fillHeight: true
			onButtonPressed: {
				CppCore.sendRotateZCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}

		ImageButton {
			imageSrc: "qrc:/images/dance.svg"
			Layout.fillHeight: true
			Layout.fillWidth: true

			onButtonPressed: {
				CppCore.sendDanceCommand()
			}
			onButtonReleased: {
				CppCore.sendStopMoveCommand()
			}
		}
	}
}
