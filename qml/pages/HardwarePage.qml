import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    contentWidth: availableWidth
    ColumnLayout {
        width: parent.width; spacing: Kirigami.Units.largeSpacing
        RowLayout {
            Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
            Kirigami.Heading { Layout.fillWidth: true; text: qsTr("Hardware and Kernel"); level: 1 }
            Controls.Button { text: qsTr("Refresh"); icon.name: "view-refresh"; onClicked: systemBackend.refreshHardware() }
        }
        GridLayout {
            Layout.fillWidth: true; Layout.margins: 28; Layout.topMargin: 0; columns: 3
            Repeater {
                model: [
                    { title: qsTr("Processor"), value: systemBackend.hardware.cpu || qsTr("Unavailable"), icon: "cpu" },
                    { title: qsTr("Graphics"), value: systemBackend.hardware.gpu || qsTr("Unavailable"), icon: "video-display" },
                    { title: qsTr("Memory"), value: systemBackend.hardware.ram || qsTr("Unavailable"), icon: "memory" }
                ]
                delegate: Kirigami.AbstractCard {
                    background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                    required property var modelData
                    Layout.fillWidth: true; Layout.minimumHeight: 145
                    contentItem: ColumnLayout {
                        Kirigami.Icon { source: modelData.icon; implicitWidth: 32; implicitHeight: 32 }
                        Controls.Label { text: modelData.title; color: window.secondaryText }
                        Controls.Label { Layout.fillWidth: true; text: modelData.value; font.bold: true; wrapMode: Text.WordWrap }
                    }
                }
            }
        }
        Kirigami.Heading { Layout.leftMargin: 28; text: qsTr("Installed kernels"); level: 2 }
        Repeater {
            model: systemBackend.kernels
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property string modelData
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                contentItem: RowLayout {
                    Kirigami.Icon { source: "computer"; implicitWidth: 24; implicitHeight: 24 }
                    Controls.Label { Layout.fillWidth: true; text: modelData; font.bold: true }
                    Controls.Label { text: qsTr("Installed"); color: Kirigami.Theme.positiveTextColor }
                }
            }
        }
    }
}
