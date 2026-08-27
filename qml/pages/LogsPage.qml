import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        Kirigami.Heading { Layout.fillWidth: true; text: "Log do Sistema"; level: 1 }
        Controls.ComboBox { id: priority; visible: systemBackend.logsUnlocked; model: ["", "err", "warning", "info"]; Layout.preferredWidth: 130 }
        Controls.Button { visible: systemBackend.logsUnlocked; text: "Consultar"; icon.name: "view-refresh"; onClicked: systemBackend.queryLogs(unit.currentText === "Todos" ? "" : unit.currentText, priority.currentText, search.text) }
    }
    RowLayout {
        visible: systemBackend.logsUnlocked
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.ComboBox { id: unit; model: ["Todos"].concat(systemBackend.logUnits); Layout.preferredWidth: 260 }
        Controls.TextField { id: search; Layout.fillWidth: true; placeholderText: "Buscar no journal…"; onAccepted: systemBackend.queryLogs(unit.currentText === "Todos" ? "" : unit.currentText, priority.currentText, text) }
    }
    Rectangle {
        visible: systemBackend.logsUnlocked
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; Layout.topMargin: 0
        color: window.cardColor; border.color: Qt.alpha(Kirigami.Theme.textColor, 0.18); radius: 6
        Controls.ScrollView {
            anchors.fill: parent; anchors.margins: 10
            ListView {
                clip: true; model: systemBackend.logLines
                delegate: Controls.Label {
                    required property string modelData
                    width: ListView.view.width; text: modelData; font.family: "monospace"; font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft; wrapMode: Text.WrapAnywhere
                }
            }
        }
    }

    Item {
        visible: !systemBackend.logsUnlocked
        Layout.fillWidth: true
        Layout.fillHeight: true

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(460, parent.width - 56)
            spacing: Kirigami.Units.largeSpacing

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 72; implicitHeight: 72; radius: 36
                color: Qt.alpha(window.lyraBlue, 0.14)
                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "security-high"
                    implicitWidth: 36; implicitHeight: 36
                    color: window.lyraBlue
                }
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                text: "Acesso administrativo necessário"
                level: 2
                horizontalAlignment: Text.AlignHCenter
            }
            Controls.Label {
                Layout.fillWidth: true
                text: "Os registros do sistema podem conter informações sensíveis. Autentique-se como administrador para consultá-los."
                color: Kirigami.Theme.disabledTextColor
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
            Controls.Label {
                visible: systemBackend.logsStatus.length > 0
                Layout.fillWidth: true
                text: systemBackend.logsStatus
                color: Kirigami.Theme.negativeTextColor
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
            Controls.Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Autenticar e consultar logs"
                icon.name: "document-decrypt"
                highlighted: true
                onClicked: systemBackend.queryLogs("", "", "")
            }
        }
    }
}
