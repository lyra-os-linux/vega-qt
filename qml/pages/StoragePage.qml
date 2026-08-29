import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"

ColumnLayout {
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        Kirigami.Heading { Layout.fillWidth: true; text: "Armazenamento"; level: 1 }
        Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshStorage() }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; Layout.topMargin: 0
        ListView {
            clip: true; spacing: Kirigami.Units.smallSpacing; model: systemBackend.volumes
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property var modelData
                width: ListView.view.width
                contentItem: RowLayout {
                    Kirigami.Icon { source: modelData.removable ? "drive-removable-media" : "drive-harddisk"; implicitWidth: 34; implicitHeight: 34 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { text: modelData.name || modelData.path; font.bold: true }
                        Controls.Label { text: (modelData.model || modelData.fsType) + "  •  " + (modelData.mountpoint || "Não montado"); color: window.secondaryText }
                        LyraProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: modelData.percent }
                    }
                    ColumnLayout {
                        Controls.Label { text: modelData.percent + "%"; font.bold: true }
                        Controls.Label { text: modelData.used + " / " + modelData.size; color: window.secondaryText }
                    }
                }
            }
        }
    }
}
