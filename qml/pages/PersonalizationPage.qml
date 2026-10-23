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
                icon.name: "go-next"; text: qsTr("Configure"); display: Controls.AbstractButton.IconOnly
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
                Kirigami.Heading { text: qsTr("Personalization"); level: 1 }
                Controls.Label { text: qsTr("Adjust Plasma appearance and behavior"); color: window.secondaryText }
            }
            Controls.Button { text: qsTr("Refresh"); icon.name: "view-refresh"; onClicked: systemBackend.refreshAppearance() }
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
                title: qsTr("Global theme"); description: qsTr("Complete look of the environment, panels and desktop")
                value: systemBackend.appearance.style || qsTr("Loading…"); iconName: "preferences-desktop"; moduleName: "kcm_lookandfeel"
            }
            AppearanceCard {
                title: qsTr("Colors"); description: qsTr("Palette used by windows, text and interface elements")
                value: systemBackend.appearance.colors || qsTr("Loading…"); iconName: "preferences-desktop-color"; moduleName: "kcm_colors"
            }
            AppearanceCard {
                title: qsTr("Icons"); description: qsTr("Icon set for applications and the system")
                value: systemBackend.appearance.icons || qsTr("Loading…"); iconName: "preferences-desktop-icons"; moduleName: "kcm_icons"
            }
            AppearanceCard {
                title: qsTr("Fonts"); description: qsTr("Typeface, size and text rendering")
                value: systemBackend.appearance.font || qsTr("Loading…"); iconName: "preferences-desktop-font"; moduleName: "kcm_fonts"
            }
            AppearanceCard {
                title: qsTr("Cursors"); description: qsTr("Mouse pointer theme and size")
                value: qsTr("Configure in Plasma"); iconName: "input-mouse"; moduleName: "kcm_cursortheme"
            }
            AppearanceCard {
                title: qsTr("Window decorations"); description: qsTr("Window borders, buttons and style")
                value: qsTr("Configure in Plasma"); iconName: "preferences-system-windows"; moduleName: "kcm_kwindecoration"
            }
            AppearanceCard {
                title: qsTr("Wallpaper"); description: qsTr("Background image and behavior")
                value: qsTr("Configure in Plasma"); iconName: "preferences-desktop-wallpaper"; moduleName: "kcm_wallpaper"
            }
            AppearanceCard {
                title: qsTr("Lock screen"); description: qsTr("Lock screen look and options")
                value: qsTr("Configure in Plasma"); iconName: "system-lock-screen"; moduleName: "kcm_screenlocker"
            }
        }
        Item { implicitHeight: 12 }
    }
}
