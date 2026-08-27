import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        ColumnLayout {
            Layout.fillWidth: true; spacing: 0
            Kirigami.Heading { text: "Bluetooth"; level: 1 }
            Controls.Label { text: systemBackend.bluetoothStatus.controller || "Nenhum adaptador detectado"; color: window.secondaryText }
        }
        Controls.Label {
            horizontalAlignment: Text.AlignRight
            text: systemBackend.bluetoothStatus.powered ? "Ativado" : "Desativado"
            font.bold: true
            color: systemBackend.bluetoothStatus.powered ? Kirigami.Theme.positiveTextColor : window.secondaryText
        }
        Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshBluetooth() }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; Layout.topMargin: 0
        ListView {
            clip: true; spacing: 6; model: systemBackend.bluetoothDevices
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property var modelData
                width: ListView.view.width
                contentItem: RowLayout {
                    Kirigami.Icon { source: modelData.icon || "bluetooth"; implicitWidth: 34; implicitHeight: 34 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignLeft; text: modelData.name || modelData.address; font.bold: true }
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignLeft; text: modelData.address; color: window.secondaryText }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 150
                        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: modelData.rssi ? modelData.rssi + " dBm" : "Sem sinal"; font.bold: true }
                        Controls.Label {
                            Layout.fillWidth: true; horizontalAlignment: Text.AlignRight
                            text: modelData.connected ? "Conectado" : (modelData.paired ? "Pareado" : "Disponível")
                            color: modelData.connected ? Kirigami.Theme.positiveTextColor : window.secondaryText
                        }
                    }
                }
            }
            Kirigami.PlaceholderMessage { anchors.centerIn: parent; visible: systemBackend.bluetoothDevices.length === 0; text: "Nenhum dispositivo encontrado"; icon.name: "bluetooth" }
        }
    }
}
