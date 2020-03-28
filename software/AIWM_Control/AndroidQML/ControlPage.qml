import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Item {

	property int systemStatus: 0xFF
	property int moduleStatus: 0xFF
	property real cellVoltage1: 0
	property real cellVoltage2: 0
	property real cellVoltage3: 0
	property real batteryVoltage: 0
	property int batteryCharge: 0

	id: root
	width: 500
	height: 888
	clip: true

	FontLoader {
		id: fixedFont
		source: "qrc:/fonts/OpenSans-Regular.ttf"
	}

	Connections {
		target: CppCore
		onSystemStatusUpdated: {
			systemStatus = newSystemStatus
		}
		onModuleStatusUpdated: {
			moduleStatus = newModuleStatus
		}
		onVoltageValuesUpdated: {
			cellVoltage1 = newCellVoltage1 / 1000.0
			cellVoltage2 = newCellVoltage2 / 1000.0
			cellVoltage3 = newCellVoltage3 / 1000.0
			batteryVoltage = newBatteryVoltage / 1000.0
		}
		onBatteryChargeUpdated: {
			batteryCharge = newBatteryCharge
		}
	}

	Rectangle {
		id: joystickItem
		y: 653
		width: 270
		height: 270
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 20
		color: "#00000000"
		border.color: "#AAAAAA"

		ParallelAnimation {
			id: animationReturn
			NumberAnimation {
				target: dragItem
				property: "x"
				to: joystickItem.width / 2 - dragItem.width / 2
				duration: 100
			}
			NumberAnimation {
				target: dragItem
				property: "y"
				to: joystickItem.height / 2 - dragItem.height / 2
				duration: 100
			}
		}

		ColorOverlay {
			id: dragItem
			width: 100
			height: 100
			source: joystickImage
			color: "#FFFFFF"
			smooth: true
			antialiasing: true

			x: joystickItem.width / 2 - dragItem.width / 2
			y: joystickItem.height / 2 - dragItem.height / 2

			MouseArea {
				anchors.fill: parent
				drag.target: dragItem
				drag.axis: Drag.XAndYAxis
				drag.minimumX: 0
				drag.maximumX: joystickItem.width - dragItem.width
				drag.minimumY: 0
				drag.maximumY: joystickItem.height - dragItem.height

				onReleased: {
					animationReturn.start()
					CppCore.sendStopMoveCommand()
				}
				onPositionChanged: {

					var x = dragItem.x - (joystickItem.width / 2 - dragItem.width / 2)

					var factor = 15

					var scaled_x = Math.abs(x / factor)
					var scaled_max_x = (drag.maximumX / 2) / factor

					var curvature = Math.exp(scaled_x) / Math.exp(
								scaled_max_x) * 1999

					curvature = Math.round(curvature)
					if (x < 0) {
						curvature = -curvature
					}

					var stepLength = -(dragItem.y * (240.0 / drag.maximumY) - 120.0)
					CppCore.sendStartMotionCommand(Math.round(stepLength),
												   Math.round(curvature))
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
			font.family: fixedFont.name
			font.pointSize: 10
			text: qsTr("Battery charge:")
			horizontalAlignment: Text.AlignLeft
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
			value: batteryCharge
			to: 100
		}

		Label {
			width: 50
			height: 22
			text: batteryVoltageProgressBar.value + "%"
			anchors.top: parent.top
			anchors.right: parent.right
			font.family: fixedFont.name
			font.pointSize: 10
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
			text: "Connection\nlost"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x80
		}

		StatusLabel {
			text: "Reserved"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x40
		}

		StatusLabel {
			text: "Math\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x20
		}

		StatusLabel {
			text: "Sync\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x10
		}

		StatusLabel {
			text: "Voltage\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x08
		}

		StatusLabel {
			text: "Memory\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x04
		}

		StatusLabel {
			text: "Configuration\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x02
		}

		StatusLabel {
			text: "Fatal\nerror"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: systemStatus & 0x01
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
			isActive: moduleStatus & 0x80
			//deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Reserved"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x40
			//deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Reserved"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x20
			//deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "System\nmonitor"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x10
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Sequences\nengine"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x08
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Motion\ncore"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x04
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Servo\ndriver"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x02
			deactiveColor: "#00DD00"
		}

		StatusLabel {
			text: "Configurator"
			Layout.minimumHeight: 40
			Layout.maximumHeight: 40
			Layout.preferredWidth: 118
			Layout.fillWidth: true
			isActive: moduleStatus & 0x01
			deactiveColor: "#00DD00"
		}
	}

	SwipeView {
		clip: true
		width: 480
		height: 50
		anchors.top: voltage.bottom
		anchors.topMargin: 10
		anchors.left: parent.left
		anchors.leftMargin: 10
		anchors.right: parent.right
		anchors.rightMargin: 10

		RowLayout {

			ImageButton {
				Layout.fillHeight: true
				Layout.fillWidth: true
				imageSrc: "qrc:/images/getUp.svg"
				onButtonPressed: {
					CppCore.sendGetUpCommand()
				}
			}

			ImageButton {
				Layout.fillWidth: true
				imageSrc: "qrc:/images/getDown.svg"
				Layout.fillHeight: true
				onButtonPressed: {
					CppCore.sendGetDownCommand()
				}
			}

			ImageButton {
				imageSrc: "qrc:/images/dance.svg"
				Layout.fillHeight: true
				Layout.fillWidth: true


				/*onButtonPressed: {
					CppCore.sendDanceCommand()
				}
				onButtonReleased: {
					CppCore.sendStopMoveCommand()
				}*/
			}

			ImageButton {
				imageSrc: "qrc:/images/arrowUpDown.svg"
				imageRotate: 90
				Layout.fillHeight: true
				Layout.fillWidth: true

				onButtonPressed: {
					CppCore.sendUpDownCommand()
				}
				onButtonReleased: {
					CppCore.sendStopMoveCommand()
				}
			}
		}

		RowLayout {

			ImageButton {
				Layout.fillHeight: true
				Layout.fillWidth: true
				imageSrc: "qrc:/images/swordLeft.svg"
				onButtonPressed: {
					CppCore.sendAttackLeftCommand()
				}
				onButtonReleased: {
					CppCore.sendStopMoveCommand()
				}
			}

			ImageButton {
				Layout.fillWidth: true
				imageSrc: "qrc:/images/swordRight.svg"
				Layout.fillHeight: true
				onButtonPressed: {
					CppCore.sendAttackRightCommand()
				}
				onButtonReleased: {
					CppCore.sendStopMoveCommand()
				}
			}

			ImageButton {
				imageSrc: "qrc:/images/arrowPushPull.svg"
				Layout.fillHeight: true
				Layout.fillWidth: true

				onButtonPressed: {
					CppCore.sendPushPullCommand()
				}
				onButtonReleased: {
					CppCore.sendStopMoveCommand()
				}
			}
		}
	}
}
