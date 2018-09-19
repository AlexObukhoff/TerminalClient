import QtQuick 2.6

Row {
	z: 10

	anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 17 }

	spacing: 10

	Rectangle {
		width: 20
		height: 20
		rotation: 200

		color: "white"

		border.width : 2
		border.color : "white"


		RotationAnimation on rotation {
			loops: Animation.Infinite
			from: 0
			to: 360
			duration: 300
		}
	}

	Rectangle {
		width: 20
		height: 20
		rotation: 200

		color: "blue"
		border.width : 2
		border.color : "white"

		RotationAnimation on rotation {
			loops: Animation.Infinite
			from: 0
			to: 360
			duration: 500
		}
	}

	Rectangle {
		width: 20
		height: 20
		rotation: 200

		color: "red"

		border.width : 2
		border.color : "white"

		RotationAnimation on rotation {
			loops: Animation.Infinite
			from: 0
			to: 360
			duration: 1000
		}
	}
}

