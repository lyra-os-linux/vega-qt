import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    spacing: Kirigami.Units.largeSpacing
    Kirigami.Heading { Layout.margins: 28; Layout.bottomMargin: 0; text: "Serviços"; level: 1 }
    RowLayout {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.Label {
            Layout.fillWidth: true
            text: allServices.checked ? "Todos os serviços disponíveis no sistema" : "Serviços essenciais gerenciados pelo vegad"
            color: window.secondaryText
        }
        Controls.Button {
            id: allServices
            text: "Todos os serviços"
            icon.name: "view-list-details"
            checkable: true
            checked: systemBackend.showingAllServices
            onToggled: systemBackend.refreshServices(checked)
        }
        Controls.Button {
            text: "Atualizar"; icon.name: "view-refresh"
            onClicked: systemBackend.refreshServices(allServices.checked)
        }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.bottomMargin: 28
        ListView {
            clip: true
            spacing: Kirigami.Units.smallSpacing
            model: systemBackend.services
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
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
                        Controls.Label { text: modelData.description || modelData.name; color: window.secondaryText; elide: Text.ElideRight; Layout.fillWidth: true }
                    }
                    Controls.Label { text: modelData.active ? "Em execução" : "Parado"; color: modelData.active ? Kirigami.Theme.positiveTextColor : window.secondaryText }
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
