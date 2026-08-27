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
            Kirigami.Heading { Layout.fillWidth: true; text: "Hardware e Kernel"; level: 1 }
            Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshHardware() }
        }
        GridLayout {
            Layout.fillWidth: true; Layout.margins: 28; Layout.topMargin: 0; columns: 3
            Repeater {
                model: [
                    { title: "Processador", value: systemBackend.hardware.cpu || "Indisponível", icon: "cpu" },
                    { title: "Gráficos", value: systemBackend.hardware.gpu || "Indisponível", icon: "video-display" },
                    { title: "Memória", value: systemBackend.hardware.ram || "Indisponível", icon: "memory" }
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
        Kirigami.Heading { Layout.leftMargin: 28; text: "Kernels instalados"; level: 2 }
        Repeater {
            model: systemBackend.kernels
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property string modelData
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                contentItem: RowLayout {
                    Kirigami.Icon { source: "computer"; implicitWidth: 24; implicitHeight: 24 }
                    Controls.Label { Layout.fillWidth: true; text: modelData; font.bold: true }
                    Controls.Label { text: "Instalado"; color: Kirigami.Theme.positiveTextColor }
                }
            }
        }
    }
}
