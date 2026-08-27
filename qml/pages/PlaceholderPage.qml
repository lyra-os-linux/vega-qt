import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    property string title
    property string iconName
    property string description
    signal returnRequested()
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - 80, 620)
        spacing: Kirigami.Units.largeSpacing
        Kirigami.Icon { Layout.alignment: Qt.AlignHCenter; source: iconName; implicitWidth: 64; implicitHeight: 64 }
        Kirigami.Heading { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: title; level: 1 }
        Controls.Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; text: description; color: window.secondaryText }
        Kirigami.InlineMessage { Layout.fillWidth: true; visible: true; text: "Módulo preparado para a próxima etapa da migração QtDBus." }
        Controls.Button { Layout.alignment: Qt.AlignHCenter; text: "Voltar ao Painel"; icon.name: "go-home"; onClicked: returnRequested() }
    }
}
