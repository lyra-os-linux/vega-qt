import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    contentWidth: availableWidth
    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing
        Kirigami.Heading { Layout.margins: 28; Layout.bottomMargin: 0; text: "Software"; level: 1 }
        Controls.Label {
            Layout.leftMargin: 28
            text: "Aplicativos, pacotes e atualizações"
            color: Kirigami.Theme.disabledTextColor
        }
        GridLayout {
            Layout.fillWidth: true; Layout.margins: 28; columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            Kirigami.AbstractCard {
                Layout.fillWidth: true; Layout.minimumHeight: 130
                contentItem: ColumnLayout {
                    Controls.Label { text: "Gerenciador de pacotes"; color: Kirigami.Theme.disabledTextColor }
                    Kirigami.Heading { text: systemBackend.packageManager; level: 2 }
                }
            }
            Kirigami.AbstractCard {
                Layout.fillWidth: true; Layout.minimumHeight: 130
                contentItem: ColumnLayout {
                    Controls.Label { text: "Estado do sistema"; color: Kirigami.Theme.disabledTextColor }
                    Kirigami.Heading { text: systemBackend.softwareStatus; level: 3; wrapMode: Text.WordWrap }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
            Controls.TextField { Layout.fillWidth: true; placeholderText: "Buscar pacote ou aplicativo…"; enabled: false }
            Controls.Button { text: "Verificar atualizações"; icon.name: "view-refresh"; onClicked: systemBackend.refreshSoftware() }
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true; Layout.margins: 28; visible: true
            text: "A consulta de atualizações já usa o vegad. Busca, instalação e remoção entram no próximo corte deste módulo."
        }
    }
}
