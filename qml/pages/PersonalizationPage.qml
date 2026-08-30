import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: root
    contentWidth: availableWidth

    component AppearanceCard: Rectangle {
        required property string title
        required property string description
        required property string value
        required property string iconName
        required property string moduleName
        Layout.fillWidth: true
        implicitHeight: 138
        radius: 9
        color: window.cardColor
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)

        RowLayout {
            anchors.fill: parent; anchors.margins: 18; spacing: 15
            Rectangle {
                implicitWidth: 48; implicitHeight: 48; radius: 9
                color: Qt.alpha(window.lyraBlue, 0.13)
                Kirigami.Icon { anchors.centerIn: parent; source: iconName; implicitWidth: 27; implicitHeight: 27; color: window.lyraBlue }
            }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 3
                Controls.Label { text: title; font.bold: true; font.pixelSize: 16 }
                Controls.Label { Layout.fillWidth: true; text: description; color: window.secondaryText; wrapMode: Text.WordWrap }
                Controls.Label { Layout.fillWidth: true; text: value; color: window.lyraBlue; font.bold: true; elide: Text.ElideRight }
            }
            Controls.ToolButton {
                icon.name: "go-next"; text: "Configurar"; display: Controls.AbstractButton.IconOnly
                onClicked: systemBackend.openAppearanceModule(moduleName)
            }
        }
        MouseArea {
            anchors.fill: parent; cursorShape: Qt.PointingHandCursor; z: -1
            onClicked: systemBackend.openAppearanceModule(moduleName)
        }
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
            ColumnLayout {
                Layout.fillWidth: true; spacing: 2
                Kirigami.Heading { text: "Personalização"; level: 1 }
                Controls.Label { text: "Ajuste a aparência e o comportamento do Plasma"; color: window.secondaryText }
            }
            Controls.Button { text: "Atualizar"; icon.name: "view-refresh"; onClicked: systemBackend.refreshAppearance() }
        }

        Rectangle {
            visible: systemBackend.appearanceStatus.length > 0
            Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
            implicitHeight: warningRow.implicitHeight + 20; radius: 7
            color: Qt.alpha(Kirigami.Theme.neutralTextColor, 0.12)
            border.color: Qt.alpha(Kirigami.Theme.neutralTextColor, 0.5)
            RowLayout {
                id: warningRow; anchors.fill: parent; anchors.margins: 10
                Kirigami.Icon { source: "dialog-information"; implicitWidth: 20; implicitHeight: 20 }
                Controls.Label { Layout.fillWidth: true; text: systemBackend.appearanceStatus; wrapMode: Text.WordWrap }
            }
        }

        GridLayout {
            Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
            columns: width > 720 ? 2 : 1
            columnSpacing: Kirigami.Units.largeSpacing; rowSpacing: Kirigami.Units.largeSpacing

            AppearanceCard {
                title: "Tema global"; description: "Aparência completa do ambiente, painéis e área de trabalho"
                value: systemBackend.appearance.style || "Carregando…"; iconName: "preferences-desktop"; moduleName: "kcm_lookandfeel"
            }
            AppearanceCard {
                title: "Cores"; description: "Paleta usada por janelas, textos e elementos de interface"
                value: systemBackend.appearance.colors || "Carregando…"; iconName: "preferences-desktop-color"; moduleName: "kcm_colors"
            }
            AppearanceCard {
                title: "Ícones"; description: "Conjunto de ícones dos aplicativos e do sistema"
                value: systemBackend.appearance.icons || "Carregando…"; iconName: "preferences-desktop-icons"; moduleName: "kcm_icons"
            }
            AppearanceCard {
                title: "Fontes"; description: "Família, tamanho e renderização dos textos"
                value: systemBackend.appearance.font || "Carregando…"; iconName: "preferences-desktop-font"; moduleName: "kcm_fonts"
            }
            AppearanceCard {
                title: "Cursores"; description: "Tema e tamanho do ponteiro do mouse"
                value: "Configurar no Plasma"; iconName: "input-mouse"; moduleName: "kcm_cursortheme"
            }
            AppearanceCard {
                title: "Decoração das janelas"; description: "Bordas, botões e estilo das janelas"
                value: "Configurar no Plasma"; iconName: "preferences-system-windows"; moduleName: "kcm_kwindecoration"
            }
            AppearanceCard {
                title: "Papel de parede"; description: "Imagem e comportamento do plano de fundo"
                value: "Configurar no Plasma"; iconName: "preferences-desktop-wallpaper"; moduleName: "kcm_wallpaper"
            }
            AppearanceCard {
                title: "Tela de bloqueio"; description: "Visual e opções da tela de bloqueio"
                value: "Configurar no Plasma"; iconName: "system-lock-screen"; moduleName: "kcm_screenlocker"
            }
        }
        Item { implicitHeight: 12 }
    }
}
