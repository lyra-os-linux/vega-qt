import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ScrollView {
    id: root
    contentWidth: availableWidth

    function selectValue(combo, value) {
        const index = combo.find(value)
        if (index >= 0)
            combo.currentIndex = index
        else
            combo.editText = value || ""
    }

    Connections {
        target: systemBackend
        function onDateTimeChanged() {
            root.selectValue(timezoneBox, systemBackend.dateTimeStatus.timezone)
            root.selectValue(localeBox, systemBackend.dateTimeStatus.locale)
            root.selectValue(keymapBox, systemBackend.dateTimeStatus.keymap)
            ntpSwitch.checked = Boolean(systemBackend.dateTimeStatus.ntp)
        }
    }

    Component.onCompleted: {
        root.selectValue(timezoneBox, systemBackend.dateTimeStatus.timezone)
        root.selectValue(localeBox, systemBackend.dateTimeStatus.locale)
        root.selectValue(keymapBox, systemBackend.dateTimeStatus.keymap)
        ntpSwitch.checked = Boolean(systemBackend.dateTimeStatus.ntp)
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 28
            Layout.bottomMargin: 0
            spacing: 3
            Kirigami.Heading { text: qsTr("Date, Time and Language"); level: 1 }
            Controls.Label {
                text: qsTr("Adjust the system region, clock and keyboard layout")
                color: Kirigami.Theme.disabledTextColor
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 28
            implicitHeight: settings.implicitHeight + 36
            radius: 8
            color: window.cardColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)

            ColumnLayout {
                id: settings
                anchors.fill: parent
                anchors.margins: 18
                spacing: Kirigami.Units.largeSpacing

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Controls.Label { text: qsTr("Time zone"); font.bold: true }
                        Controls.Label { text: qsTr("Sets the local time shown by the system"); color: Kirigami.Theme.disabledTextColor }
                    }
                    Controls.ComboBox {
                        id: timezoneBox
                        Layout.preferredWidth: 300
                        model: systemBackend.timezones
                        editable: true
                    }
                }

                Kirigami.Separator { Layout.fillWidth: true }

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Controls.Label { text: qsTr("Automatic date and time"); font.bold: true }
                        Controls.Label { text: qsTr("Synchronizes the clock over the network (NTP)"); color: Kirigami.Theme.disabledTextColor }
                    }
                    Controls.Switch { id: ntpSwitch; text: checked ? qsTr("On") : qsTr("Off") }
                }

                Kirigami.Separator { Layout.fillWidth: true }

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Controls.Label { text: qsTr("Language and region"); font.bold: true }
                        Controls.Label { text: qsTr("Format used by applications and the system"); color: Kirigami.Theme.disabledTextColor }
                    }
                    Controls.ComboBox {
                        id: localeBox
                        Layout.preferredWidth: 300
                        model: systemBackend.locales
                        editable: true
                    }
                }

                Kirigami.Separator { Layout.fillWidth: true }

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Controls.Label { text: qsTr("Keyboard layout"); font.bold: true }
                        Controls.Label { text: qsTr("Keymap used on the console and in the session"); color: Kirigami.Theme.disabledTextColor }
                    }
                    Controls.ComboBox {
                        id: keymapBox
                        Layout.preferredWidth: 300
                        model: systemBackend.keymaps
                        editable: true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 28
            Layout.rightMargin: 28
            Item { Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Reset")
                icon.name: "view-refresh"
                onClicked: systemBackend.refreshDateTime()
            }
            Controls.Button {
                text: qsTr("Apply changes")
                icon.name: "dialog-ok-apply"
                highlighted: true
                onClicked: systemBackend.applyDateTime(
                    timezoneBox.editText || timezoneBox.currentText,
                    ntpSwitch.checked,
                    localeBox.editText || localeBox.currentText,
                    keymapBox.editText || keymapBox.currentText)
            }
        }
    }
}
