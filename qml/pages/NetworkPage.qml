import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 0
        Kirigami.Heading { Layout.fillWidth: true; text: qsTr("Network and Firewall"); level: 1 }
        Controls.Button { text: qsTr("Refresh"); icon.name: "view-refresh"; onClicked: systemBackend.refreshNetwork() }
    }
    Controls.TabBar {
        id: tabs; Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        Controls.TabButton { text: qsTr("Interfaces") }
        Controls.TabButton { text: qsTr("Wi‑Fi") }
        Controls.TabButton { text: qsTr("Firewall") }
    }
    StackLayout {
        Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: tabs.currentIndex
        NetworkList { model: systemBackend.networkInterfaces; wifi: false }
        NetworkList { model: systemBackend.wifiNetworks; wifi: true }
        ColumnLayout {
            Layout.margins: 28
            Kirigami.AbstractCard {
                background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
                Layout.fillWidth: true
                contentItem: RowLayout {
                    Kirigami.Icon { source: systemBackend.firewallEnabled ? "security-high" : "security-low"; implicitWidth: 40; implicitHeight: 40 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: qsTr("Firewall")
                            level: 2
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: qsTr("Protection for network connections")
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 180
                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            text: systemBackend.firewallEnabled ? qsTr("Active") : qsTr("Inactive")
                            font.bold: true
                            color: systemBackend.firewallEnabled ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            text: qsTr("Zone: %1").arg(systemBackend.firewallZone || qsTr("not detected"))
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
    component NetworkList: ListView {
        property bool wifi
        Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28; clip: true; spacing: 6
        delegate: Kirigami.AbstractCard {
            background: Rectangle { color: window.cardColor; radius: 7; border.color: Qt.alpha(window.primaryText, 0.14) }
            required property var modelData
            width: ListView.view.width
            contentItem: RowLayout {
                Kirigami.Icon { source: wifi ? "network-wireless" : "network-wired"; implicitWidth: 32; implicitHeight: 32 }
                ColumnLayout {
                    Layout.fillWidth: true
                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: wifi ? modelData.ssid : (modelData.name || modelData.device)
                        font.bold: true
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: wifi ? modelData.security : (modelData.ipv4 || qsTr("No IPv4 address"))
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
                ColumnLayout {
                    Layout.preferredWidth: 160
                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: wifi ? modelData.signal + "%" : (modelData.speed || modelData.type)
                        font.bold: true
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: wifi ? (modelData.active ? qsTr("Connected") : qsTr("Available")) : connectionLabel(modelData.state)
                        color: (wifi ? modelData.active : modelData.state === "connected")
                               ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                    }
                }
            }
        }
    }

    function connectionLabel(state) {
        if (state === "connected") return qsTr("Connected")
        if (state === "connecting") return qsTr("Connecting")
        if (state === "disconnected") return qsTr("Disconnected")
        return state || qsTr("Unavailable")
    }
}
