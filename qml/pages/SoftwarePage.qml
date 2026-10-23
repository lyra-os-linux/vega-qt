import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"

ColumnLayout {
    id: page
    property var pendingPackage: null
    property string pendingAction: ""
    spacing: Kirigami.Units.largeSpacing

    RowLayout {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.topMargin: 20
        ColumnLayout {
            Layout.fillWidth: true; spacing: 0
            Kirigami.Heading { text: qsTr("Software"); level: 1 }
            Controls.Label { text: systemBackend.softwareStatus; color: window.secondaryText }
        }
        Controls.Button { text: qsTr("Check"); icon.name: "view-refresh"; onClicked: systemBackend.refreshSoftware() }
    }

    Controls.TabBar {
        id: tabs
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.TabButton { text: qsTr("Explore"); icon.name: "system-search" }
        Controls.TabButton { text: qsTr("Updates (%1)").arg(systemBackend.softwareUpdates.length); icon.name: "system-software-update" }
        Controls.TabButton { text: qsTr("Repositories"); icon.name: "network-server" }
    }

    StackLayout {
        Layout.fillWidth: true; Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                Controls.TextField {
                    id: search; Layout.fillWidth: true; placeholderText: qsTr("Search for a package or application…")
                    onAccepted: systemBackend.searchSoftware(text)
                }
                Controls.Button { text: qsTr("Search"); icon.name: "search"; enabled: search.text.trim().length >= 2 && !systemBackend.softwareBusy; onClicked: systemBackend.searchSoftware(search.text) }
            }
            Controls.BusyIndicator { Layout.alignment: Qt.AlignHCenter; running: systemBackend.softwareBusy; visible: running }
            PackageList {
                Layout.fillWidth: true; Layout.fillHeight: true
                model: systemBackend.softwareResults
                emptyText: search.text.length < 2 ? qsTr("Type at least two characters") : qsTr("No packages found")
                onActionRequested: (item, action) => { page.pendingPackage = item; page.pendingAction = action; confirmDialog.open() }
            }
        }

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                Controls.Label { Layout.fillWidth: true; text: systemBackend.softwareUpdates.length === 0 ? qsTr("The system is up to date") : qsTr("Updates available") }
                Controls.Button { text: qsTr("Update all"); icon.name: "system-software-update"; enabled: systemBackend.softwareUpdates.length > 0; onClicked: { page.pendingAction = "updateAll"; confirmDialog.open() } }
            }
            PackageList {
                Layout.fillWidth: true; Layout.fillHeight: true
                model: systemBackend.softwareUpdates; emptyText: qsTr("No pending updates"); updateMode: true
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
                        Controls.Switch { text: modelData.enabled ? qsTr("Active") : qsTr("Inactive"); checked: modelData.enabled; onToggled: systemBackend.setRepositoryEnabled(modelData.name, checked) }
                    }
                }
                Kirigami.PlaceholderMessage { anchors.centerIn: parent; visible: systemBackend.repositories.length === 0; text: qsTr("No repositories returned"); icon.name: "network-server" }
            }
        }
    }

    LyraProgressBar {
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
        title: page.pendingAction === "remove" ? qsTr("Remove package?") : qsTr("Confirm change")
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        modal: true
        onAccepted: {
            if (page.pendingAction === "updateAll") systemBackend.updateAll()
            else if (page.pendingAction === "remove") systemBackend.removePackage(page.pendingPackage.origin, page.pendingPackage.id)
            else if (page.pendingAction === "update") systemBackend.updatePackage(page.pendingPackage.origin, page.pendingPackage.id)
            else systemBackend.installPackage(page.pendingPackage.origin, page.pendingPackage.id)
        }
        Controls.Label { width: 380; wrapMode: Text.WordWrap; text: page.pendingAction === "updateAll" ? qsTr("Install all available updates?") : (page.pendingPackage ? (page.pendingAction === "remove" ? qsTr("Remove %1?") : qsTr("Install %1?")).arg(page.pendingPackage.name) : "") }
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
                    Controls.Label { text: modelData.origin === "flathub" ? "Flatpak" : (modelData.repository || qsTr("Official")); color: window.secondaryText }
                    Controls.Button {
                        text: updateMode ? qsTr("Update") : (modelData.installed ? qsTr("Remove") : qsTr("Install"))
                        icon.name: updateMode ? "system-software-update" : (modelData.installed ? "edit-delete" : "list-add")
                        onClicked: actionRequested(modelData, updateMode ? "update" : (modelData.installed ? "remove" : "install"))
                    }
                }
            }
        }
        Kirigami.PlaceholderMessage { anchors.centerIn: parent; visible: model.length === 0; text: emptyText; icon.name: "system-software-install" }
    }
}
