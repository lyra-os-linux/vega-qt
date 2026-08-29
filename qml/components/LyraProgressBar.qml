import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: control
    property real from: 0
    property real to: 100
    property real value: 0
    readonly property real fraction: Math.max(0, Math.min(1,
        (value - from) / Math.max(1, to - from)))

    implicitHeight: 8
    radius: height / 2
    color: Qt.alpha(Kirigami.Theme.textColor, 0.14)
    clip: true

    Rectangle {
        width: control.width * control.fraction
        height: control.height
        radius: control.radius
        color: "#2777c7"
    }
}
