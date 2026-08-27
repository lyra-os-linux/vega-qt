import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtCore as Core
import org.kde.kirigami as Kirigami
import "pages"

Kirigami.ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 900
    minimumHeight: 620
    visible: true
    title: "Vega — KDE"

    readonly property color lyraBlue: "#2777c7"
    readonly property bool darkMode: forceDarkTheme || appSettings.darkMode
    readonly property color pageColor: darkMode ? "#191c20" : "#f6f7f9"
    readonly property color cardColor: darkMode ? "#23272d" : "#ffffff"
    readonly property color alternateColor: darkMode ? "#20242a" : "#eef1f5"
    readonly property color primaryText: darkMode ? "#f1f3f5" : "#20242a"
    readonly property color secondaryText: darkMode ? "#aeb6c0" : "#66717e"
    property int currentPage: 0
    property string searchText: ""

    color: pageColor
    palette.window: pageColor
    palette.windowText: primaryText
    palette.base: cardColor
    palette.alternateBase: alternateColor
    palette.text: primaryText
    palette.button: cardColor
    palette.buttonText: primaryText
    palette.highlight: lyraBlue
    palette.highlightedText: "#ffffff"
    palette.placeholderText: secondaryText
    Kirigami.Theme.inherit: false
    Kirigami.Theme.backgroundColor: pageColor
    Kirigami.Theme.alternateBackgroundColor: alternateColor
    Kirigami.Theme.textColor: primaryText
    Kirigami.Theme.disabledTextColor: secondaryText
    Kirigami.Theme.highlightColor: lyraBlue
    Kirigami.Theme.highlightedTextColor: "#ffffff"
    Kirigami.Theme.positiveTextColor: darkMode ? "#5bd68a" : "#168447"
    Kirigami.Theme.neutralTextColor: darkMode ? "#f3bd59" : "#9a6500"
    Kirigami.Theme.negativeTextColor: darkMode ? "#ff7a85" : "#c52c3a"

    Core.Settings {
        id: appSettings
        category: "appearance"
        property bool darkMode: false
    }

    Component.onCompleted: {
        if (forceDarkTheme)
            appSettings.darkMode = true
        systemBackend.setDarkTheme(window.darkMode)
    }

    ListModel {
        id: navigationModel
        ListElement { label: "Painel"; section: "Principal"; iconName: "preferences-system" }
        ListElement { label: "Software"; section: "Principal"; iconName: "system-software-install" }
        ListElement { label: "Backup"; section: "Principal"; iconName: "document-save" }
        ListElement { label: "Assistente de IA"; section: "Principal"; iconName: "system-search" }
        ListElement { label: "Hardware e Kernel"; section: "Sistema"; iconName: "computer" }
        ListElement { label: "Data, Hora e Idioma"; section: "Sistema"; iconName: "preferences-system-time" }
        ListElement { label: "Personalização"; section: "Sistema"; iconName: "preferences-desktop" }
        ListElement { label: "Monitor do Sistema"; section: "Sistema"; iconName: "utilities-system-monitor" }
        ListElement { label: "Armazenamento"; section: "Sistema"; iconName: "drive-harddisk" }
        ListElement { label: "Rede e Firewall"; section: "Sistema"; iconName: "network-wireless" }
        ListElement { label: "Bluetooth"; section: "Sistema"; iconName: "bluetooth" }
        ListElement { label: "Serviços"; section: "Sistema"; iconName: "system-run" }
        ListElement { label: "Usuários"; section: "Sistema"; iconName: "system-users" }
        ListElement { label: "Log do Sistema"; section: "Sistema"; iconName: "logviewer" }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 242
            Layout.fillHeight: true
            color: window.alternateColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.13)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    Image {
                        source: "qrc:/vega.svg"
                        Layout.preferredWidth: 34; Layout.preferredHeight: 34
                        sourceSize.width: 68; sourceSize.height: 68
                        fillMode: Image.PreserveAspectFit
                    }
                    ColumnLayout {
                        spacing: 0
                        Controls.Label { text: "Vega"; font.bold: true; font.pixelSize: 18; color: window.primaryText }
                        Controls.Label { text: "LYRA KDE"; font.pixelSize: 10; color: window.secondaryText }
                    }
                    Item { Layout.fillWidth: true }
                }

                Controls.TextField {
                    Layout.fillWidth: true
                    placeholderText: "Buscar configuração…"
                    color: window.primaryText
                    placeholderTextColor: window.secondaryText
                    leftPadding: 34
                    onTextChanged: window.searchText = text.toLowerCase()
                    Kirigami.Icon {
                        anchors.left: parent.left; anchors.leftMargin: 10; anchors.verticalCenter: parent.verticalCenter
                        source: "search"; implicitWidth: 16; implicitHeight: 16
                    }
                }

                Controls.ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff
                    ListView {
                        id: navigation
                        clip: true
                        spacing: 2
                        model: navigationModel
                        delegate: ColumnLayout {
                            required property int index
                            required property string label
                            required property string section
                            required property string iconName
                            width: navigation.width
                            spacing: 2
                            visible: window.searchText.length === 0 || label.toLowerCase().includes(window.searchText)
                            height: visible ? implicitHeight : 0

                            Controls.Label {
                                visible: index === 0 || navigationModel.get(index - 1).section !== section
                                text: section.toUpperCase()
                                font.pixelSize: 10; font.bold: true
                                color: window.secondaryText
                                Layout.leftMargin: 12; Layout.topMargin: 12; Layout.bottomMargin: 3
                            }
                            Controls.ItemDelegate {
                                Layout.fillWidth: true
                                implicitHeight: 38
                                highlighted: window.currentPage === index
                                onClicked: window.currentPage = index
                                contentItem: RowLayout {
                                    Kirigami.Icon {
                                        source: iconName; implicitWidth: 19; implicitHeight: 19
                                        color: window.currentPage === index ? "white" : window.primaryText
                                    }
                                    Controls.Label {
                                        Layout.fillWidth: true; text: label; elide: Text.ElideRight
                                        color: window.currentPage === index ? "white" : window.primaryText
                                        font.bold: window.currentPage === index
                                    }
                                }
                                background: Rectangle {
                                    radius: 5
                                    color: window.currentPage === index ? window.lyraBlue
                                          : parent.hovered ? Qt.alpha(Kirigami.Theme.textColor, 0.08) : "transparent"
                                }
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Controls.ToolBar {
                Layout.fillWidth: true
                background: Rectangle {
                    color: window.cardColor
                    border.color: Qt.alpha(window.primaryText, 0.12)
                }
                contentItem: RowLayout {
                    Kirigami.Heading {
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        text: navigationModel.get(window.currentPage).label
                        level: 3
                    }
                    Item { Layout.fillWidth: true }
                    Controls.ToolButton {
                        icon.name: "view-refresh"; text: "Atualizar"; display: Controls.AbstractButton.IconOnly
                        onClicked: systemBackend.refresh()
                    }
                    Controls.ToolButton {
                        icon.name: window.darkMode ? "weather-clear" : "weather-clear-night"
                        text: window.darkMode ? "Usar tema claro" : "Usar tema escuro"
                        display: Controls.AbstractButton.IconOnly
                        onClicked: {
                            appSettings.darkMode = !window.darkMode
                            systemBackend.setDarkTheme(appSettings.darkMode)
                        }
                        Controls.ToolTip.visible: hovered
                        Controls.ToolTip.text: text
                    }
                }
            }

            Controls.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 0
                contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: Kirigami.Units.largeSpacing
                    Kirigami.Heading { Layout.margins: 28; Layout.bottomMargin: 0; text: "Central de Controle"; level: 1 }
                    Controls.Label {
                        Layout.leftMargin: 28
                        text: "Administração do LyraOS integrada ao Plasma"
                        color: window.secondaryText
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
                        implicitHeight: statusRow.implicitHeight + Kirigami.Units.largeSpacing * 2; radius: 8
                        color: systemBackend.connected ? Qt.alpha(window.lyraBlue, 0.14)
                                                       : Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                        border.color: systemBackend.connected ? window.lyraBlue : Kirigami.Theme.negativeTextColor
                        RowLayout {
                            id: statusRow; anchors.fill: parent; anchors.margins: Kirigami.Units.largeSpacing
                            Kirigami.Icon { source: systemBackend.connected ? "dialog-ok" : "dialog-warning"; implicitWidth: 28; implicitHeight: 28 }
                            ColumnLayout {
                                Controls.Label { text: systemBackend.status; font.bold: true }
                                Controls.Label {
                                    text: systemBackend.connected ? "vegad " + systemBackend.version : "Instale ou inicie o serviço vegad"
                                    color: window.secondaryText
                                }
                            }
                        }
                    }
                    GridLayout {
                        Layout.fillWidth: true; Layout.margins: 28; Layout.topMargin: 0
                        columns: width > 760 ? 3 : 2
                        columnSpacing: Kirigami.Units.largeSpacing; rowSpacing: Kirigami.Units.largeSpacing
                        DashboardCard { title: "Sistema"; value: systemBackend.distro; iconName: "computer" }
                        DashboardCard { title: "Armazenamento"; value: systemBackend.disk; detail: systemBackend.diskPercent > 0 ? systemBackend.diskPercent + "% em uso" : ""; iconName: "drive-harddisk" }
                        DashboardCard { title: "Atualizações"; value: "Pronto para verificar"; detail: "Zypper e Flatpak"; iconName: "system-software-update" }
                        DashboardCard { title: "Serviços"; value: "Monitoramento"; detail: "Systemd"; iconName: "system-run" }
                        DashboardCard { title: "Backup"; value: "Não configurado"; detail: "Restic"; iconName: "document-save" }
                        DashboardCard { title: "Segurança"; value: "Firewall"; detail: "Políticas do sistema"; iconName: "security-high" }
                    }
                }
            }

            SoftwarePage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 1
            }

            ServicesPage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 11
            }

            HardwarePage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 4
            }

            StoragePage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 8
            }

            NetworkPage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 9
            }

            BluetoothPage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 10
            }

            UsersPage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: window.currentPage === 12
            }
            MonitorPage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 7 }
            LogsPage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 13 }
            DateTimePage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 5 }
            BackupPage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 2 }
            PersonalizationPage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 6 }
            AssistantPage { Layout.fillWidth: true; Layout.fillHeight: true; visible: window.currentPage === 3 }

            PlaceholderPage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: ![0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13].includes(window.currentPage)
                title: navigationModel.get(window.currentPage).label
                iconName: navigationModel.get(window.currentPage).iconName
                description: moduleDescription(window.currentPage)
                onReturnRequested: window.currentPage = 0
            }
        }
    }

    function moduleDescription(page) {
        const descriptions = [
            "Visão geral do sistema",
            "Aplicativos, pacotes, repositórios e atualizações do sistema.",
            "Backups e pontos de restauração protegidos pelo vegad.",
            "Assistência contextual para administração do LyraOS.",
            "Inventário de hardware, drivers, firmware e kernels instalados.",
            "Fuso horário, sincronização, idioma e formatos regionais.",
            "Aparência, temas, ícones, wallpapers e comportamento do Plasma.",
            "Uso de CPU, memória, processos e saúde do sistema.",
            "Discos, partições, volumes e sistemas de arquivos.",
            "Conexões, Wi-Fi, VPN, proxy e regras de firewall.",
            "Adaptadores e dispositivos Bluetooth.",
            "Serviços systemd e seu estado operacional.",
            "Contas locais, grupos e permissões.",
            "Eventos e diagnóstico do journal do sistema."
        ]
        return descriptions[page] || "Configuração do sistema"
    }

    component DashboardCard: Kirigami.AbstractCard {
        background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
        required property string title
        required property string value
        property string detail: ""
        required property string iconName
        Layout.fillWidth: true; Layout.minimumHeight: 135
        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Kirigami.Icon { source: iconName; implicitWidth: 38; implicitHeight: 38 }
            ColumnLayout {
                Layout.fillWidth: true
                Controls.Label { text: title; color: window.secondaryText }
                Controls.Label { Layout.fillWidth: true; text: value; font.pixelSize: 17; font.bold: true; elide: Text.ElideRight }
                Controls.Label { visible: detail.length > 0; text: detail; color: window.secondaryText }
            }
        }
    }
}
