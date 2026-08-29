import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"

ColumnLayout {
    id: root
    property string selectedConfig: ""
    property string selectedSnapshot: ""
    spacing: Kirigami.Units.largeSpacing

    function frequencyLabel(value) {
        return ({"daily": "Diário", "weekly": "Semanal", "on-connect": "Ao conectar", "manual": "Manual"})[value] || value
    }
    function sizeLabel(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + " MB"
        return (bytes / 1024).toFixed(1) + " KB"
    }

    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Kirigami.Heading { text: "Backup"; level: 1 }
            Controls.Label { text: "Proteja seus arquivos com cópias versionadas pelo Restic"; color: Kirigami.Theme.disabledTextColor }
        }
        Controls.Button { text: "Nova rotina"; icon.name: "list-add"; onClicked: createDialog.open() }
    }

    Rectangle {
        visible: systemBackend.backupStatus.length > 0
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        implicitHeight: statusRow.implicitHeight + 20; radius: 7
        color: Qt.alpha(window.lyraBlue, 0.10); border.color: Qt.alpha(window.lyraBlue, 0.35)
        RowLayout {
            id: statusRow; anchors.fill: parent; anchors.margins: 10
            Kirigami.Icon { source: "document-save"; implicitWidth: 20; implicitHeight: 20 }
            Controls.Label { Layout.fillWidth: true; text: systemBackend.backupStatus; wrapMode: Text.WordWrap }
            LyraProgressBar {
                visible: systemBackend.backupProgress > 0 && systemBackend.backupProgress < 100
                from: 0; to: 100; value: systemBackend.backupProgress; Layout.preferredWidth: 180
            }
        }
    }

    Controls.TabBar {
        id: tabs
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.TabButton { text: "Rotinas"; icon.name: "document-save" }
        Controls.TabButton { text: "Pontos de restauração"; icon.name: "edit-undo-history" }
    }

    Controls.ScrollView {
        visible: tabs.currentIndex === 0
        Layout.fillWidth: true; Layout.fillHeight: true
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width; spacing: Kirigami.Units.largeSpacing
            Controls.Label {
                visible: systemBackend.backupConfigs.length === 0
                Layout.fillWidth: true; Layout.margins: 28
                text: "Nenhuma rotina configurada. Crie uma para começar a proteger seus arquivos."
                horizontalAlignment: Text.AlignHCenter; color: Kirigami.Theme.disabledTextColor
            }
            Repeater {
                model: systemBackend.backupConfigs
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                    implicitHeight: configRow.implicitHeight + 32; radius: 8
                    color: window.cardColor; border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)
                    RowLayout {
                        id: configRow; anchors.fill: parent; anchors.margins: 16; spacing: 16
                        Kirigami.Icon { source: "document-save"; implicitWidth: 32; implicitHeight: 32 }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Controls.Label { text: modelData.id; font.bold: true; font.pixelSize: 16 }
                            Controls.Label { text: modelData.paths; color: Kirigami.Theme.disabledTextColor; elide: Text.ElideMiddle; Layout.fillWidth: true }
                            Controls.Label { text: "Destino: " + modelData.destination; color: Kirigami.Theme.disabledTextColor; elide: Text.ElideMiddle; Layout.fillWidth: true }
                        }
                        ColumnLayout {
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter; spacing: 5
                            Controls.Label { Layout.alignment: Qt.AlignRight; text: root.frequencyLabel(modelData.frequency); font.bold: true }
                            RowLayout {
                                Controls.Button {
                                    text: "Executar agora"; icon.name: "media-playback-start"
                                    onClicked: systemBackend.runBackup(modelData.id)
                                }
                                Controls.ToolButton {
                                    icon.name: "edit-delete"; text: "Excluir"; display: Controls.AbstractButton.IconOnly
                                    onClicked: { root.selectedConfig = modelData.id; deleteDialog.open() }
                                }
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent; acceptedButtons: Qt.LeftButton
                        onClicked: { root.selectedConfig = modelData.id; systemBackend.refreshBackup(modelData.id); tabs.currentIndex = 1 }
                        z: -1
                    }
                }
            }
        }
    }

    Controls.ScrollView {
        visible: tabs.currentIndex === 1
        Layout.fillWidth: true; Layout.fillHeight: true
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width; spacing: Kirigami.Units.largeSpacing
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                Controls.Label { Layout.fillWidth: true; text: root.selectedConfig.length ? "Rotina: " + root.selectedConfig : "Selecione uma rotina"; font.bold: true }
                Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshBackup(root.selectedConfig) }
            }
            Controls.Label {
                visible: systemBackend.backupSnapshots.length === 0
                Layout.fillWidth: true; Layout.margins: 28
                text: "Nenhum ponto de restauração encontrado."
                horizontalAlignment: Text.AlignHCenter; color: Kirigami.Theme.disabledTextColor
            }
            Repeater {
                model: systemBackend.backupSnapshots
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
                    implicitHeight: snapshotRow.implicitHeight + 28; radius: 8
                    color: window.cardColor; border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)
                    RowLayout {
                        id: snapshotRow; anchors.fill: parent; anchors.margins: 14
                        Kirigami.Icon { source: "edit-undo-history"; implicitWidth: 28; implicitHeight: 28 }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Controls.Label { text: modelData.date; font.bold: true }
                            Controls.Label { text: modelData.id; color: Kirigami.Theme.disabledTextColor }
                        }
                        ColumnLayout {
                            Controls.Label { Layout.alignment: Qt.AlignRight; text: modelData.files + " arquivos" }
                            Controls.Label { Layout.alignment: Qt.AlignRight; text: root.sizeLabel(modelData.bytes); color: Kirigami.Theme.disabledTextColor }
                        }
                        Controls.Button {
                            text: "Restaurar"; icon.name: "edit-undo"
                            onClicked: { root.selectedSnapshot = modelData.id; restoreDialog.open() }
                        }
                    }
                }
            }
        }
    }

    Controls.Dialog {
        id: createDialog
        anchors.centerIn: parent; width: 560
        title: "Nova rotina de backup"; modal: true
        standardButtons: Controls.Dialog.Cancel | Controls.Dialog.Save
        onAccepted: systemBackend.createBackupConfig(idField.text, pathsField.text, destinationField.text, uuidField.text, frequencyBox.currentValue)
        ColumnLayout {
            width: parent.width
            Controls.Label { text: "Nome"; font.bold: true }
            Controls.TextField { id: idField; Layout.fillWidth: true; placeholderText: "documentos" }
            Controls.Label { text: "Pastas (separadas por vírgula)"; font.bold: true }
            Controls.TextField { id: pathsField; Layout.fillWidth: true; placeholderText: "/home/usuario/Documentos, /home/usuario/Imagens" }
            Controls.Label { text: "Destino"; font.bold: true }
            Controls.TextField { id: destinationField; Layout.fillWidth: true; placeholderText: "/mnt/backup/Lyra" }
            Controls.Label { text: "UUID do dispositivo removível (opcional)"; font.bold: true }
            Controls.TextField { id: uuidField; Layout.fillWidth: true; placeholderText: "Deixe vazio para um destino permanente" }
            Controls.Label { text: "Frequência"; font.bold: true }
            Controls.ComboBox {
                id: frequencyBox; Layout.fillWidth: true; textRole: "text"; valueRole: "value"
                model: [{text: "Diário", value: "daily"}, {text: "Semanal", value: "weekly"},
                        {text: "Ao conectar o dispositivo", value: "on-connect"}, {text: "Manual", value: "manual"}]
            }
        }
    }

    Controls.Dialog {
        id: deleteDialog; anchors.centerIn: parent; modal: true
        title: "Excluir rotina?"; standardButtons: Controls.Dialog.Cancel | Controls.Dialog.Yes
        onAccepted: systemBackend.deleteBackupConfig(root.selectedConfig)
        Controls.Label { text: "A rotina será removida, mas o repositório de backup no destino será preservado."; wrapMode: Text.WordWrap; width: 420 }
    }

    Controls.Dialog {
        id: restoreDialog; anchors.centerIn: parent; width: 520; modal: true
        title: "Restaurar ponto"; standardButtons: Controls.Dialog.Cancel | Controls.Dialog.Ok
        onAccepted: systemBackend.restoreBackup(root.selectedSnapshot, restoreTarget.text)
        ColumnLayout {
            width: parent.width
            Controls.Label { text: "Pasta de destino"; font.bold: true }
            Controls.TextField { id: restoreTarget; Layout.fillWidth: true; placeholderText: "/home/usuario/Restaurados" }
            Controls.Label {
                Layout.fillWidth: true
                text: "Os arquivos serão restaurados em uma nova subpasta. Os dados existentes não serão sobrescritos."
                color: Kirigami.Theme.disabledTextColor; wrapMode: Text.WordWrap
            }
        }
    }
}
