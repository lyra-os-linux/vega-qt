#include "systembackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>

namespace {
constexpr auto Service = "org.lyraos.Vega1";
constexpr auto ObjectPath = "/org/lyraos/Vega1";
constexpr auto Interface = "org.lyraos.Vega1.System";
}

SystemBackend::SystemBackend(QObject *parent) : QObject(parent) { refresh(); }

void SystemBackend::refresh()
{
    QDBusInterface system(Service, ObjectPath, Interface, QDBusConnection::systemBus());
    if (!system.isValid()) {
        m_connected = false;
        m_status = tr("vegad indisponível");
        m_version = tr("Não conectado");
        m_distro = tr("Aguardando o serviço do sistema");
        m_disk.clear();
        m_diskPercent = 0;
        emit changed();
        return;
    }
    const QDBusReply<bool> ping = system.call(QStringLiteral("Ping"));
    const QDBusReply<QString> version = system.call(QStringLiteral("Version"));
    const QDBusReply<QString> distro = system.call(QStringLiteral("Distro"));
    const QDBusMessage disk = system.call(QStringLiteral("DiskUsage"));
    m_connected = ping.isValid() && ping.value();
    m_status = m_connected ? tr("Sistema conectado") : tr("Falha na comunicação");
    m_version = version.isValid() ? version.value() : tr("Desconhecida");
    m_distro = distro.isValid() ? distro.value() : tr("Não detectada");
    if (disk.type() == QDBusMessage::ReplyMessage && disk.arguments().size() == 3) {
        m_disk = tr("%1 usados de %2").arg(disk.arguments().at(0).toString(), disk.arguments().at(1).toString());
        m_diskPercent = static_cast<int>(disk.arguments().at(2).toUInt());
    } else {
        m_disk = tr("Informação indisponível");
        m_diskPercent = 0;
    }
    emit changed();
}
