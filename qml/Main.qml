import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 900
    minimumHeight: 620
    visible: true
    title: "Vega — KDE"

    readonly property color lyraBlue: "#2777c7"
    property int currentPage: 0
    property string searchText: ""

    ListModel {
        id: navigationModel
        ListElement { label: "Painel"; section: "Principal"; iconName: "view-dashboard" }
        ListElement { label: "Software"; section: "Principal"; iconName: "system-software-install" }
        ListElement { label: "Backup"; section: "Principal"; iconName: "document-save" }
        ListElement { label: "Assistente de IA"; section: "Principal"; iconName: "system-search" }
        ListElement { label: "Hardware e Kernel"; section: "Sistema"; iconName: "computer" }
        ListElement { label: "Data, Hora e Idioma"; section: "Sistema"; iconName: "preferences-system-time" }
        ListElement { label: "Personalização"; section: "Sistema"; iconName: "preferences-desktop-theme" }
        ListElement { label: "Monitor do Sistema"; section: "Sistema"; iconName: "utilities-system-monitor" }
        ListElement { label: "Armazenamento"; section: "Sistema"; iconName: "drive-harddisk" }
        ListElement { label: "Rede e Firewall"; section: "Sistema"; iconName: "network-wireless" }
        ListElement { label: "Bluetooth"; section: "Sistema"; iconName: "preferences-system-bluetooth" }
        ListElement { label: "Serviços"; section: "Sistema"; iconName: "system-run" }
        ListElement { label: "Usuários"; section: "Sistema"; iconName: "system-users" }
        ListElement { label: "Log do Sistema"; section: "Sistema"; iconName: "view-list-text" }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 242
            Layout.fillHeight: true
            color: Kirigami.Theme.alternateBackgroundColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.13)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    Rectangle {
                        implicitWidth: 32; implicitHeight: 32; radius: 5
                        color: window.lyraBlue
                        Controls.Label {
                            anchors.centerIn: parent
                            text: "V"; color: "white"; font.bold: true; font.pixelSize: 17
                        }
                    }
                    ColumnLayout {
                        spacing: 0
                        Controls.Label { text: "Vega"; font.bold: true; font.pixelSize: 18 }
                        Controls.Label { text: "LYRA KDE"; font.pixelSize: 10; color: Kirigami.Theme.disabledTextColor }
                    }
                    Item { Layout.fillWidth: true }
                }

                Controls.TextField {
                    Layout.fillWidth: true
                    placeholderText: "Buscar configuração…"
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
                                color: Kirigami.Theme.disabledTextColor
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
                                        color: window.currentPage === index ? "white" : Kirigami.Theme.textColor
                                    }
                                    Controls.Label {
                                        Layout.fillWidth: true; text: label; elide: Text.ElideRight
                                        color: window.currentPage === index ? "white" : Kirigami.Theme.textColor
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
                    Controls.ToolButton { icon.name: "application-menu"; display: Controls.AbstractButton.IconOnly }
                }
            }

            Controls.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: Kirigami.Units.largeSpacing
                    Kirigami.Heading { Layout.margins: 28; Layout.bottomMargin: 0; text: "Central de Controle"; level: 1 }
                    Controls.Label {
                        Layout.leftMargin: 28
                        text: "Administração do LyraOS integrada ao Plasma"
                        color: Kirigami.Theme.disabledTextColor
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
                                    color: Kirigami.Theme.disabledTextColor
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
        }
    }

    component DashboardCard: Kirigami.AbstractCard {
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
                Controls.Label { text: title; color: Kirigami.Theme.disabledTextColor }
                Controls.Label { Layout.fillWidth: true; text: value; font.pixelSize: 17; font.bold: true; elide: Text.ElideRight }
                Controls.Label { visible: detail.length > 0; text: detail; color: Kirigami.Theme.disabledTextColor }
            }
        }
    }
}
