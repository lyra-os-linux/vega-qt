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
            Kirigami.Heading { text: "Data, Hora e Idioma"; level: 1 }
            Controls.Label {
                text: "Ajuste a região, o relógio e o layout de teclado do sistema"
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
                        Controls.Label { text: "Fuso horário"; font.bold: true }
                        Controls.Label { text: "Define a hora local exibida pelo sistema"; color: Kirigami.Theme.disabledTextColor }
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
                        Controls.Label { text: "Data e hora automáticas"; font.bold: true }
                        Controls.Label { text: "Sincroniza o relógio pela rede (NTP)"; color: Kirigami.Theme.disabledTextColor }
                    }
                    Controls.Switch { id: ntpSwitch; text: checked ? "Ativado" : "Desativado" }
                }

                Kirigami.Separator { Layout.fillWidth: true }

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Controls.Label { text: "Idioma e região"; font.bold: true }
                        Controls.Label { text: "Formato usado por aplicativos e pelo sistema"; color: Kirigami.Theme.disabledTextColor }
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
                        Controls.Label { text: "Layout do teclado"; font.bold: true }
                        Controls.Label { text: "Mapa de teclas usado no console e na sessão"; color: Kirigami.Theme.disabledTextColor }
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
                text: "Restaurar"
                icon.name: "view-refresh"
                onClicked: systemBackend.refreshDateTime()
            }
            Controls.Button {
                text: "Aplicar alterações"
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
