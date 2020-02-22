import QtQuick 2.12
import QtQuick.Controls 2.5
import QtDataVisualization 1.3

ApplicationWindow {
	id: applicationWindow
	visible: true
	width: 1000
	height: 700
	title: qsTr("Graph3D")

	ListModel {
		id: scatterData
	}
	ListModel {
		id: scatterWrongData
	}
	ListModel {
		id: scatterRect
	}

	Pane {
		id: scatterPane
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.bottom: parent.bottom
		anchors.top: parent.top

		Scatter3D {
			id: scatter3D
			selectionMode: AbstractGraph3D.SelectionItem
			anchors.fill: parent
			measureFps: true
			aspectRatio: 1.0
			shadowQuality: Scatter3D.ShadowQualityNone

			axisX: ValueAxis3D {
				min: -220
				max: 220
				segmentCount: 20
				title: "X"
				titleVisible: true
			}
			axisY: ValueAxis3D {
				min: -220
				max: 220
				segmentCount: 20
				title: "Y"
				titleVisible: true
			}
			axisZ: ValueAxis3D {
				min: -220
				max: 220
				segmentCount: 20
				title: "Z"
				titleVisible: true
			}
			theme: Theme3D {
				type: Theme3D.ThemeIsabelle
				windowColor: applicationWindow.color
				font.family: "Segoe UI"
				font.pointSize: 10
				gridEnabled: true
				gridLineColor: "green"
				labelBackgroundEnabled: false
				labelBorderEnabled: false
				labelTextColor: "white"
			}
			Scatter3DSeries {
				itemSize: 0.2
				baseColor: "red"
				mesh: Scatter3DSeries.MeshArrow
				meshSmooth: true
				ItemModelScatterDataProxy {
					yPosRole: "y"
					xPosRole: "x"
					zPosRole: "z"
					itemModel: ListModel {
						ListElement {
							x: 0
							y: 0
							z: 0
						}
					}
				}
			}
			Scatter3DSeries {
				itemSize: 0.05
				baseColor: "blue"
				meshSmooth: true
				ItemModelScatterDataProxy {
					yPosRole: "y"
					xPosRole: "x"
					zPosRole: "z"
					itemModel: scatterData
				}
			}
			Scatter3DSeries {
				itemSize: 0.05
				baseColor: "red"
				meshSmooth: true
				ItemModelScatterDataProxy {
					yPosRole: "y"
					xPosRole: "x"
					zPosRole: "z"
					itemModel: scatterWrongData
				}
			}
			Scatter3DSeries {
				itemSize: 0.02
				baseColor: "green"
				meshSmooth: true
				ItemModelScatterDataProxy {
					yPosRole: "y"
					xPosRole: "x"
					zPosRole: "z"
					itemModel: scatterRect
				}
			}
		}
	}

	Pane {
		id: controlsPane
		height: 70
		padding: 10
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.right: parent.right

		Label {
			width: 37
			height: 50
			text: qsTr("FPS:")
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignRight
		}

		Label {
			x: 42
			y: 0
			width: 59
			height: 50
			text: Math.round(scatter3D.currentFps)
			verticalAlignment: Text.AlignVCenter
		}

		ComboBox {
			id: comboBox
			x: 106
			y: 0
			width: 180
			height: 50
			currentIndex: 0
			model: ["XYZ_LINEAR", "XZ_ARC_Y_LINEAR", "XZ_ARC_Y_SINUS", "XZ_ELLIPTICAL_Y_SINUS", "PATH_YZ_ELLIPTICAL_X_LINEAR"]
		}

		Button {

			property real x0: 0
			property real y0: 0
			property real z0: 0

			property real x1: 0
			property real y1: 0
			property real z1: 0

			x: 644
			y: 0
			width: 126
			height: 50
			text: qsTr("Расчитать")
			onClicked: {

				x0 = beginPointX.text
				y0 = beginPointY.text
				z0 = beginPointZ.text

				x1 = endPointX.text
				y1 = endPointY.text
				z1 = endPointZ.text

				var alg = comboBox.currentIndex

				drawRectangle(x0, y0, z0, x1, y1, z1)
				CppCore.calculateRequest(x0, y0, z0, x1, y1, z1, alg)
			}
		}

		Button {
			x: 775
			y: 0
			width: 126
			height: 50
			text: qsTr("Очистить")
			onClicked: {
				scatterRect.clear()
				scatterData.clear()
				scatterWrongData.clear()
			}
		}

		TextField {
			id: beginPointX
			x: 291
			y: 0
			width: 50
			height: 50
			text: qsTr("135")
			horizontalAlignment: Text.AlignHCenter
		}

		TextField {
			id: beginPointY
			x: 346
			y: 0
			width: 50
			height: 50
			text: qsTr("-35")
			horizontalAlignment: Text.AlignHCenter
		}

		TextField {
			id: beginPointZ
			x: 401
			y: 0
			width: 50
			height: 50
			text: qsTr("70")
			horizontalAlignment: Text.AlignHCenter
		}

		TextField {
			id: endPointX
			x: 479
			y: 0
			width: 50
			height: 50
			text: qsTr("220")
			horizontalAlignment: Text.AlignHCenter
		}

		TextField {
			id: endPointY
			x: 534
			y: 0
			width: 50
			height: 50
			text: qsTr("-60")
			horizontalAlignment: Text.AlignHCenter
		}

		TextField {
			id: endPointZ
			x: 589
			y: 0
			width: 50
			height: 50
			text: qsTr("0")
			horizontalAlignment: Text.AlignHCenter
		}
	}

	Connections {

		target: CppCore
		onPointCalculated: {

			if (isSuccess) {

				scatterData.append({
									   "x": x,
									   "y": y,
									   "z": z
								   })
			} else {

				scatterWrongData.append({
											"x": x,
											"y": y,
											"z": z
										})
			}
		}
	}

	function drawRectangle(x0, y0, z0, x1, y1, z1) {
		var i = 0
		if (x0 < x1) {
			for (i = x0; i <= x1; ++i) {

				scatterRect.append({
									   "x": i.toString(),
									   "y": y0.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y1.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y0.toString(),
									   "z": z1.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y1.toString(),
									   "z": z1.toString()
								   })
			}
		} else {
			for (i = x1; i <= x0; ++i) {

				scatterRect.append({
									   "x": i.toString(),
									   "y": y0.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y1.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y0.toString(),
									   "z": z1.toString()
								   })
				scatterRect.append({
									   "x": i.toString(),
									   "y": y1.toString(),
									   "z": z1.toString()
								   })
			}
		}

		if (y0 < y1) {
			for (i = y0; i <= y1; ++i) {

				scatterRect.append({
									   "x": x0.toString(),
									   "y": i.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": i.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": x0.toString(),
									   "y": i.toString(),
									   "z": z1.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": i.toString(),
									   "z": z1.toString()
								   })
			}
		} else {
			for (i = y1; i <= y0; ++i) {

				scatterRect.append({
									   "x": x0.toString(),
									   "y": i.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": i.toString(),
									   "z": z0.toString()
								   })
				scatterRect.append({
									   "x": x0.toString(),
									   "y": i.toString(),
									   "z": z1.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": i.toString(),
									   "z": z1.toString()
								   })
			}
		}

		if (z0 < z1) {
			for (i = z0; i <= z1; ++i) {

				scatterRect.append({
									   "x": x0.toString(),
									   "y": y0.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": y0.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x0.toString(),
									   "y": y1.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": y1.toString(),
									   "z": i.toString()
								   })
			}
		} else {
			for (i = z1; i <= z0; ++i) {

				scatterRect.append({
									   "x": x0.toString(),
									   "y": y0.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": y0.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x0.toString(),
									   "y": y1.toString(),
									   "z": i.toString()
								   })
				scatterRect.append({
									   "x": x1.toString(),
									   "y": y1.toString(),
									   "z": i.toString()
								   })
			}
		}
	}
}
