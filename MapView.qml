import QtQuick 6.0
import QtLocation 6.0
import QtPositioning 6.0

Item {
    id: root

    signal coordinateClicked(double latitude, double longitude)

    // Add these missing functions
    function setCenter(lat, lon) {
        map.center = QtPositioning.coordinate(lat, lon)
    }

    function setZoomLevel(zoom) {
        map.zoomLevel = zoom
    }

    Map {
        id: map
        anchors.fill: parent

        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.highdpi_tiles"
                value: "false"
            }
        }

        center: QtPositioning.coordinate(39.8, -98.6)
        zoomLevel: 4
        minimumZoomLevel: 2
        maximumZoomLevel: 18

        MouseArea {
            anchors.fill: parent
            property point lastPan

            onPressed: function(mouse) {
                lastPan = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    var dx = mouse.x - lastPan.x
                    var dy = mouse.y - lastPan.y
                    map.pan(-dx, -dy)
                    lastPan = Qt.point(mouse.x, mouse.y)
                }
            }

            onClicked: function(mouse) {
                var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                root.coordinateClicked(coord.latitude, coord.longitude)
            }

            onWheel: function(wheel) {
                if (wheel.angleDelta.y > 0) {
                    map.zoomLevel = Math.min(map.zoomLevel + 0.5, map.maximumZoomLevel)
                } else {
                    map.zoomLevel = Math.max(map.zoomLevel - 0.5, map.minimumZoomLevel)
                }
            }
        }

        MapQuickItem {
            id: clickMarker
            visible: false
            anchorPoint.x: 12
            anchorPoint.y: 12

            sourceItem: Rectangle {
                width: 24
                height: 24
                radius: 12
                color: "red"
                border.color: "white"
                border.width: 2
            }
        }
    }

    function showMarker(lat, lon) {
        clickMarker.coordinate = QtPositioning.coordinate(lat, lon)
        clickMarker.visible = true
    }
}
