import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root
    spacing: 0

    function send() {
        if (messageField.text.trim().length === 0 || assistantBackend.busy)
            return
        assistantBackend.sendMessage(messageField.text)
        messageField.clear()
    }

    RowLayout {
        Layout.fillWidth: true; Layout.margins: 28; Layout.bottomMargin: 14
        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Kirigami.Heading { text: qsTr("AI Assistant"); level: 1 }
            Controls.Label {
                text: assistantBackend.configured
                    ? providerName(assistantBackend.provider) + " · " + assistantBackend.model
                    : qsTr("Set up your own provider to get started")
                color: Kirigami.Theme.disabledTextColor
            }
        }
        Controls.Button { text: qsTr("Clear"); icon.name: "edit-clear-history"; enabled: assistantBackend.messages.length > 0; onClicked: assistantBackend.clearConversation() }
        Controls.Button { text: qsTr("Configure"); icon.name: "configure"; enabled: !assistantBackend.busy; onClicked: settingsDialog.open() }
    }

    function providerName(value) {
        return ({openai: "OpenAI", anthropic: "Anthropic", gemini: "Google Gemini"})[value] || value
    }
    function defaultModel(value) {
        return ({openai: "gpt-4.1-mini", anthropic: "claude-sonnet-4-20250514", gemini: "gemini-2.5-flash"})[value]
    }

    Rectangle {
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28
        implicitHeight: privacyRow.implicitHeight + 18; radius: 7
        color: Qt.alpha(window.lyraBlue, 0.09); border.color: Qt.alpha(window.lyraBlue, 0.30)
        RowLayout {
            id: privacyRow; anchors.fill: parent; anchors.margins: 9
            Kirigami.Icon { source: "security-high"; implicitWidth: 20; implicitHeight: 20; color: window.lyraBlue }
            Controls.Label {
                Layout.fillWidth: true
                text: qsTr("The conversation is sent directly to the chosen provider. The key stays protected in this session's keyring.")
                wrapMode: Text.WordWrap; color: Kirigami.Theme.disabledTextColor
            }
        }
    }

    Item {
        Layout.fillWidth: true; Layout.fillHeight: true

        ColumnLayout {
            visible: !assistantBackend.configured
            anchors.centerIn: parent; width: Math.min(500, parent.width - 56)
            spacing: Kirigami.Units.largeSpacing
            Rectangle {
                Layout.alignment: Qt.AlignHCenter; implicitWidth: 76; implicitHeight: 76; radius: 38
                color: Qt.alpha(window.lyraBlue, 0.14)
                Kirigami.Icon { anchors.centerIn: parent; source: "system-search"; implicitWidth: 38; implicitHeight: 38; color: window.lyraBlue }
            }
            Kirigami.Heading { Layout.fillWidth: true; text: qsTr("Your assistant, your key"); level: 2; horizontalAlignment: Text.AlignHCenter }
            Controls.Label {
                Layout.fillWidth: true
                text: qsTr("Choose OpenAI, Anthropic or Gemini. Vega talks directly to the API and uses no intermediary servers.")
                horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; color: Kirigami.Theme.disabledTextColor
            }
            Controls.Button { Layout.alignment: Qt.AlignHCenter; text: qsTr("Set up assistant"); icon.name: "configure"; highlighted: true; onClicked: settingsDialog.open() }
        }

        ListView {
            id: conversation
            visible: assistantBackend.configured
            anchors.fill: parent; anchors.leftMargin: 28; anchors.rightMargin: 28; anchors.topMargin: 14; anchors.bottomMargin: 10
            clip: true; spacing: 12; model: assistantBackend.messages
            delegate: Item {
                required property var modelData
                width: ListView.view.width
                height: bubble.implicitHeight
                Rectangle {
                    id: bubble
                    width: Math.min(parent.width * 0.78, messageLabel.implicitWidth + 32)
                    implicitHeight: messageLabel.implicitHeight + 24
                    anchors.right: modelData.role === "user" ? parent.right : undefined
                    anchors.left: modelData.role === "assistant" ? parent.left : undefined
                    radius: 10
                    color: modelData.role === "user" ? window.lyraBlue : window.alternateColor
                    border.color: modelData.role === "user" ? window.lyraBlue : Qt.alpha(Kirigami.Theme.textColor, 0.14)
                    Controls.Label {
                        id: messageLabel; anchors.fill: parent; anchors.margins: 12
                        text: modelData.content; color: modelData.role === "user" ? "white" : Kirigami.Theme.textColor
                        wrapMode: Text.Wrap; textFormat: Text.PlainText
                    }
                }
            }
            footer: Item {
                width: conversation.width; height: assistantBackend.busy ? 46 : 0
                RowLayout {
                        visible: assistantBackend.busy; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    Controls.BusyIndicator { running: true; implicitWidth: 24; implicitHeight: 24 }
                    Controls.Label { text: qsTr("Thinking…"); color: Kirigami.Theme.disabledTextColor }
                }
            }
            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: assistantBackend.messages.length === 0 && !assistantBackend.busy
                text: qsTr("How can I help?")
                explanation: qsTr("Ask about Linux, system configuration or troubleshooting.")
                icon.name: "system-search"
            }
            Connections {
                target: assistantBackend
                function onMessagesChanged() { Qt.callLater(function() { conversation.positionViewAtEnd() }) }
            }
        }
    }

    Controls.Label {
        visible: assistantBackend.status.length > 0 && !assistantBackend.busy
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.bottomMargin: 8
        text: assistantBackend.status; color: Kirigami.Theme.negativeTextColor; wrapMode: Text.WordWrap
    }

    RowLayout {
        visible: assistantBackend.configured
        Layout.fillWidth: true; Layout.leftMargin: 28; Layout.rightMargin: 28; Layout.bottomMargin: 22
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.min(110, Math.max(46, messageField.contentHeight + 20))
            color: window.cardColor
            border.color: messageField.activeFocus ? window.lyraBlue : Qt.alpha(window.primaryText, 0.24)
            radius: 5
            TextEdit {
                id: messageField
                anchors.fill: parent; anchors.margins: 10
                color: window.primaryText
                selectionColor: window.lyraBlue; selectedTextColor: "white"
                wrapMode: TextEdit.Wrap
                enabled: !assistantBackend.busy
                Accessible.name: qsTr("Write a message")
                Keys.onReturnPressed: function(event) {
                    if (!(event.modifiers & Qt.ShiftModifier)) { root.send(); event.accepted = true }
                }
            }
            Controls.Label {
                anchors.left: parent.left; anchors.leftMargin: 10; anchors.verticalCenter: parent.verticalCenter
                visible: messageField.text.length === 0 && !messageField.activeFocus
                text: qsTr("Write a message…"); color: window.secondaryText
            }
        }
        Controls.Button {
            text: qsTr("Cancel"); visible: assistantBackend.busy; enabled: assistantBackend.busy
            onClicked: assistantBackend.cancelRequest()
        }
        Controls.Button {
            text: qsTr("Send"); icon.name: "document-send"; highlighted: true
            enabled: !assistantBackend.busy && messageField.text.trim().length > 0
            onClicked: root.send()
        }
    }

    Controls.Dialog {
        id: settingsDialog; anchors.centerIn: parent; width: 540; modal: true
        title: qsTr("Configure AI Assistant")
        standardButtons: Controls.Dialog.Cancel | Controls.Dialog.Save
        onOpened: {
            const index = providerBox.indexOfValue(assistantBackend.provider)
            providerBox.currentIndex = index >= 0 ? index : 0
            modelField.text = assistantBackend.model || root.defaultModel(providerBox.currentValue)
            keyField.clear()
        }
        onAccepted: assistantBackend.configure(providerBox.currentValue, modelField.text, keyField.text)
        ColumnLayout {
            width: parent.width; spacing: Kirigami.Units.largeSpacing
            Controls.Label { text: qsTr("Provider"); font.bold: true }
            Controls.ComboBox {
                id: providerBox; Layout.fillWidth: true; textRole: "name"; valueRole: "id"
                model: [{name: "OpenAI", id: "openai", defaultModel: "gpt-4.1-mini"},
                        {name: "Anthropic", id: "anthropic", defaultModel: "claude-sonnet-4-20250514"},
                        {name: "Google Gemini", id: "gemini", defaultModel: "gemini-2.5-flash"}]
                onActivated: modelField.text = root.defaultModel(currentValue)
            }
            Controls.Label { text: qsTr("Model"); font.bold: true }
            Controls.TextField { id: modelField; Layout.fillWidth: true; placeholderText: qsTr("Model name in the API") }
            Controls.Label { text: qsTr("API key"); font.bold: true }
            Controls.TextField {
                id: keyField; Layout.fillWidth: true; echoMode: TextInput.Password
                placeholderText: assistantBackend.configured ? qsTr("Leave empty to keep the current key") : qsTr("Paste your API key")
            }
            RowLayout {
                Kirigami.Icon { source: "security-high"; implicitWidth: 18; implicitHeight: 18 }
                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("The key is stored by the Secret Service and is never saved in plain text by Vega.")
                    color: Kirigami.Theme.disabledTextColor; wrapMode: Text.WordWrap
                }
            }
        }
    }
}
