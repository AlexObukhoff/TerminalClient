import QtQuick 1.1
import Qt.labs.particles 1.0

Rectangle {
		width: 1280
		height: 1024
		color: "transparent"

		Particles {
				y: 0
				width: parent.width
				height: parent.height
				source: "../skins/default/images/bonus/snow.png"
				lifeSpan: 3000
				count: 100
				angle: 120
				angleDeviation: 36
				velocity: 30
				velocityDeviation: 10
				ParticleMotionWander {
						xvariance: 30
						pace: 100
				}
		}

		Particles {
				y: 0
				width: parent.width
				height: parent.height
				source: "../skins/default/images/bonus/snow2.png"
				lifeSpan: 5000
				count: 50
				angle: 70
				angleDeviation: 36
				velocity: 40
				velocityDeviation: 10
				ParticleMotionWander {
						xvariance: 30
						pace: 100
				}
		}

		Particles {
				y: 0
				width: parent.width
				height: parent.height
				source: "../skins/default/images/bonus/snow3.png"
				lifeSpan: 8000
				count: 30
				angle: 70
				angleDeviation: 36
				velocity: 50
				velocityDeviation: 10
				ParticleMotionWander {
						xvariance: 30
						pace: 100
				}
		}
}

