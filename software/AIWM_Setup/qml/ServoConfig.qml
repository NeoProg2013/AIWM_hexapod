import QtQuick 2.14
import QtQuick.Controls 2.12

Frame {
	width: 190
	height: 205
	bottomPadding: 5
	topPadding: 5
	padding: 5

	property int servoType: 0
	property int servoDirection: 0
	property int servoPwmChannel: 12
	property int servoLogicZero: 0
	property int servoMinAngle: 0
	property int servoMaxAngle: 0

	property int fontPixelSize: 11

	FontLoader {
		id: fixedFont
		source: "qrc:/fonts/RobotoMono-Regular.ttf"
	}

	Label {
		x: 0
		y: 0
		width: 60
		height: 35
		text: qsTr("Привод:")
		horizontalAlignment: Text.AlignRight
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	ComboBox {
		x: 65
		y: 0
		width: 115
		height: 35
		currentIndex: servoType
		model: ["DS3218MG"]
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	Label {
		x: 0
		y: 40
		width: 60
		height: 35
		text: qsTr("Вращение:")
		horizontalAlignment: Text.AlignRight
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	ComboBox {
		x: 65
		y: 40
		width: 115
		height: 35
		currentIndex: servoDirection
		model: ["Прямое", "Обратное"]
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	Label {
		x: 0
		y: 80
		width: 60
		height: 35
		text: qsTr("PWM CH:")
		horizontalAlignment: Text.AlignRight
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	ComboBox {
		x: 65
		y: 80
		width: 115
		height: 35
		currentIndex: servoPwmChannel
		model: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17"]
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}
	Label {
		x: 0
		y: 120
		width: 60
		height: 35
		text: qsTr("Лог. ноль:")
		horizontalAlignment: Text.AlignRight
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	TextInput {
		x: 65
		y: 120
		width: 115
		height: 35
		text: servoLogicZero
		color: "#FFFFFF"
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
		validator: RegularExpressionValidator {
			regularExpression: /[0-9][0-9][0-9]/
		}
	}

	Label {
		x: 0
		y: 160
		width: 60
		height: 35
		text: qsTr("Диапазон:")
		horizontalAlignment: Text.AlignRight
		verticalAlignment: Text.AlignVCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
	}

	TextInput {
		x: 65
		y: 160
		width: 55
		height: 35
		text: servoMinAngle
		color: "#FFFFFF"
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
		validator: RegularExpressionValidator {
			regularExpression: /[0-9][0-9][0-9]/
		}
	}

	TextInput {
		x: 125
		y: 160
		width: 55
		height: 35
		text: servoMaxAngle
		color: "#FFFFFF"
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		font.family: fixedFont.family
		font.pixelSize: fontPixelSize
		validator: RegularExpressionValidator {
			regularExpression: /[0-9][0-9][0-9]/
		}
	}
}
