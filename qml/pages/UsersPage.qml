import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        Kirigami.Heading { Layout.fillWidth: true; text: "Usuários"; level: 1 }
        Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshUsers() }
        Controls.Button { text: "Novo usuário"; icon.name: "list-add-user"; enabled: false }
    }
    Controls.ScrollView {
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; Layout.topMargin: 0
        GridView {
            cellWidth: Math.max(280, width / 2); cellHeight: 150; model: systemBackend.users
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property var modelData
                width: GridView.view.cellWidth - 8; height: GridView.view.cellHeight - 8
                contentItem: RowLayout {
                    Kirigami.Icon { source: "user-identity"; implicitWidth: 48; implicitHeight: 48 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { text: modelData.fullName || modelData.username; font.bold: true; font.pixelSize: 17 }
                        Controls.Label { text: modelData.username; color: window.secondaryText }
                        Controls.Label { text: modelData.admin ? "Administrador" : "Usuário padrão"; color: modelData.admin ? Kirigami.Theme.positiveTextColor : window.secondaryText }
                        Controls.Label { Layout.fillWidth: true; text: modelData.groups; elide: Text.ElideRight; color: window.secondaryText }
                    }
                }
            }
        }
    }
}
