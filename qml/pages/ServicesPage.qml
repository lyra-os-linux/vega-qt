import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    spacing: Kirigami.Units.largeSpacing
    Kirigami.Heading { Layout.margins: 28; Layout.bottomMargin: 0; text: "Serviços"; level: 1 }
    RowLayout {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.Label { Layout.fillWidth: true; text: "Serviços gerenciados pelo vegad"; color: Kirigami.Theme.disabledTextColor }
        Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshServices() }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.bottomMargin: 28
        ListView {
            clip: true
            spacing: Kirigami.Units.smallSpacing
            model: systemBackend.services
            delegate: Kirigami.AbstractCard {
                required property var modelData
                width: ListView.view.width
                contentItem: RowLayout {
                    Kirigami.Icon {
                        source: modelData.active ? "emblem-success" : "emblem-unavailable"
                        implicitWidth: 28; implicitHeight: 28
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { text: modelData.label; font.bold: true }
                        Controls.Label { text: modelData.description || modelData.name; color: Kirigami.Theme.disabledTextColor; elide: Text.ElideRight; Layout.fillWidth: true }
                    }
                    Controls.Label { text: modelData.active ? "Em execução" : "Parado"; color: modelData.active ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor }
                    Controls.Switch { checked: modelData.enabled; enabled: false; text: "Inicialização" }
                }
            }
            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: systemBackend.services.length === 0
                text: "Nenhum serviço retornado"
                explanation: "Verifique se o vegad está disponível."
                icon.name: "system-run"
            }
        }
    }
}
