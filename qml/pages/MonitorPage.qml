import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        Kirigami.Heading { Layout.fillWidth: true; text: "Monitor do Sistema"; level: 1 }
        Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshMonitor() }
    }
    GridLayout {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; columns: 3
        MetricCard { title: "CPU"; value: Number(systemBackend.metrics.cpu || 0).toFixed(1) + "%"; progress: Number(systemBackend.metrics.cpu || 0) }
        MetricCard { title: "Memória"; value: Number(systemBackend.metrics.memory || 0).toFixed(1) + "%"; progress: Number(systemBackend.metrics.memory || 0) }
        MetricCard { title: "GPU"; value: Number(systemBackend.metrics.gpu || 0) >= 0 ? Number(systemBackend.metrics.gpu).toFixed(1) + "%" : "Indisponível"; progress: Math.max(0, Number(systemBackend.metrics.gpu || 0)) }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; Layout.topMargin: 0
        ListView {
            clip: true; spacing: 3; model: systemBackend.processes
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property var modelData
                width: ListView.view.width
                contentItem: RowLayout {
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignLeft; text: modelData.name; font.bold: true }
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignLeft; text: modelData.user + "  •  PID " + modelData.pid; color: Kirigami.Theme.disabledTextColor }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 170
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: Number(modelData.cpu).toFixed(1) + "% CPU"; font.bold: true }
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: modelData.state; color: Kirigami.Theme.disabledTextColor }
                    }
                }
            }
        }
    }
    component MetricCard: Kirigami.AbstractCard {
        background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
        required property string title; required property string value; required property real progress
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignLeft; text: title; color: Kirigami.Theme.disabledTextColor }
            Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: value; font.bold: true; font.pixelSize: 20 }
            Controls.ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: progress }
        }
    }
}
