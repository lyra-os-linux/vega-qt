import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: page
    property var pendingPackage: null
    property string pendingAction: ""
    spacing: Kirigami.Units.largeSpacing

    RowLayout {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.topMargin: 20
        ColumnLayout {
            Layout.fillWidth: true; spacing: 0
            Kirigami.Heading { text: "Software"; level: 1 }
            Controls.Label { text: systemBackend.softwareStatus; color: window.secondaryText }
        }
        Controls.Button { text: "Verificar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshSoftware() }
    }

    Controls.TabBar {
        id: tabs
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.TabButton { text: "Explorar"; icon.name: "system-search" }
        Controls.TabButton { text: "Atualizações (" + systemBackend.softwareUpdates.length + ")"; icon.name: "system-software-update" }
        Controls.TabButton { text: "Repositórios"; icon.name: "network-server" }
    }

    StackLayout {
        Layout.fillWidth: true; Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                Controls.TextField {
                    id: search; Layout.fillWidth: true; placeholderText: "Buscar pacote ou aplicativo…"
                    onAccepted: systemBackend.searchSoftware(text)
                }
                Controls.Button { text: "Buscar"; icon.name: "search"; enabled: search.text.trim().length >= 2 && !systemBackend.softwareBusy; onClicked: systemBackend.searchSoftware(search.text) }
            }
            Controls.BusyIndicator { Layout.alignment: Qt.AlignHCenter; running: systemBackend.softwareBusy; visible: running }
            PackageList {
                Layout.fillWidth: true; Layout.fillHeight: true
                model: systemBackend.softwareResults
                emptyText: search.text.length < 2 ? "Digite ao menos dois caracteres" : "Nenhum pacote encontrado"
                onActionRequested: (item, action) => { page.pendingPackage = item; page.pendingAction = action; confirmDialog.open() }
            }
        }

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                Controls.Label { Layout.fillWidth: true; text: systemBackend.softwareUpdates.length === 0 ? "O sistema está atualizado" : "Atualizações disponíveis" }
                Controls.Button { text: "Atualizar tudo"; icon.name: "system-software-update"; enabled: systemBackend.softwareUpdates.length > 0; onClicked: { page.pendingAction = "updateAll"; confirmDialog.open() } }
            }
            PackageList {
                Layout.fillWidth: true; Layout.fillHeight: true
                model: systemBackend.softwareUpdates; emptyText: "Nenhuma atualização pendente"; updateMode: true
                onActionRequested: (item, action) => { page.pendingPackage = item; page.pendingAction = action; confirmDialog.open() }
            }
        }

        Controls.ScrollView {
            contentWidth: availableWidth
            ListView {
                clip: true; spacing: Kirigami.Units.smallSpacing; model: systemBackend.repositories
                delegate: Kirigami.AbstractCard {
                    background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                    required property var modelData
                    width: ListView.view.width - 56; x: 28
                    contentItem: RowLayout {
                        Kirigami.Icon { source: "network-server"; implicitWidth: 28; implicitHeight: 28 }
                        Controls.Label { Layout.fillWidth: true; text: modelData.name; font.bold: true }
                        Controls.Switch { text: modelData.enabled ? "Ativo" : "Inativo"; checked: modelData.enabled; onToggled: systemBackend.setRepositoryEnabled(modelData.name, checked) }
                    }
                }
                Kirigami.PlaceholderMessage { anchors.centerIn: parent; visible: systemBackend.repositories.length === 0; text: "Nenhum repositório retornado"; icon.name: "network-server" }
            }
        }
    }

    Controls.ProgressBar {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        visible: systemBackend.transactionProgress > 0 && systemBackend.transactionProgress < 100
        from: 0; to: 100; value: systemBackend.transactionProgress
    }
    Kirigami.InlineMessage {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.bottomMargin: 16
        visible: systemBackend.transactionMessage.length > 0
        text: systemBackend.transactionMessage
        type: systemBackend.transactionProgress === 0 ? Kirigami.MessageType.Warning : Kirigami.MessageType.Information
    }

    Controls.Dialog {
        id: confirmDialog
        anchors.centerIn: parent
        title: page.pendingAction === "remove" ? "Remover pacote?" : "Confirmar alteração"
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        modal: true
        onAccepted: {
            if (page.pendingAction === "updateAll") systemBackend.updateAll()
            else if (page.pendingAction === "remove") systemBackend.removePackage(page.pendingPackage.origin, page.pendingPackage.id)
            else if (page.pendingAction === "update") systemBackend.updatePackage(page.pendingPackage.origin, page.pendingPackage.id)
            else systemBackend.installPackage(page.pendingPackage.origin, page.pendingPackage.id)
        }
        Controls.Label { width: 380; wrapMode: Text.WordWrap; text: page.pendingAction === "updateAll" ? "Instalar todas as atualizações disponíveis?" : (page.pendingPackage ? (page.pendingAction === "remove" ? "Remover " : "Instalar ") + page.pendingPackage.name + "?" : "") }
    }

    component PackageList: Item {
        property var model
        property string emptyText
        property bool updateMode: false
        signal actionRequested(var item, string action)
        ListView {
            anchors.fill: parent; anchors.leftMargin: 28; anchors.rightMargin: 28
            clip: true; spacing: Kirigami.Units.smallSpacing; model: parent.model
            delegate: Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                required property var modelData
                width: ListView.view.width
                contentItem: RowLayout {
                    Kirigami.Icon { source: modelData.icon || "package-x-generic"; implicitWidth: 36; implicitHeight: 36 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Controls.Label { text: modelData.name || modelData.id; font.bold: true }
                        Controls.Label { Layout.fillWidth: true; text: modelData.description || modelData.repository || modelData.id; elide: Text.ElideRight; color: window.secondaryText }
                    }
                    Controls.Label { text: modelData.origin === "flathub" ? "Flatpak" : (modelData.repository || "Oficial"); color: window.secondaryText }
                    Controls.Button {
                        text: updateMode ? "Atualizar" : (modelData.installed ? "Remover" : "Instalar")
                        icon.name: updateMode ? "system-software-update" : (modelData.installed ? "edit-delete" : "list-add")
                        onClicked: actionRequested(modelData, updateMode ? "update" : (modelData.installed ? "remove" : "install"))
                    }
                }
            }
        }
        Kirigami.PlaceholderMessage { anchors.centerIn: parent; visible: model.length === 0; text: emptyText; icon.name: "system-software-install" }
    }
}
