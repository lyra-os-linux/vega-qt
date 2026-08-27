#include "systembackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QVariantMap>

namespace {
constexpr auto Service = "org.lyraos.Vega1";
constexpr auto ObjectPath = "/org/lyraos/Vega1";
constexpr auto Interface = "org.lyraos.Vega1.System";
}

SystemBackend::SystemBackend(QObject *parent) : QObject(parent)
{
    refresh();
    refreshSoftware();
    refreshServices();
    refreshHardware();
    refreshStorage();
    refreshUsers();
}

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

void SystemBackend::refreshSoftware()
{
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    if (!software.isValid()) {
        m_packageManager = tr("Indisponível");
        m_softwareStatus = tr("O módulo Software do vegad não está disponível.");
        emit softwareChanged();
        return;
    }
    const QDBusReply<QString> manager = software.call(QStringLiteral("PackageManagerName"));
    m_packageManager = manager.isValid() ? manager.value() : tr("Zypper");
    const QDBusMessage reply = software.call(QStringLiteral("ListNativeUpdates"));
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
        int count = 0;
        array.beginArray();
        while (!array.atEnd()) {
            QString origin, id, name, description, icon, repository;
            bool installed = false;
            array.beginStructure();
            array >> origin >> id >> name >> description >> installed >> icon >> repository;
            array.endStructure();
            ++count;
        }
        array.endArray();
        m_softwareStatus = count == 0 ? tr("Sistema atualizado")
                                      : tr("%1 atualização(ões) disponível(is)").arg(count);
    } else {
        m_softwareStatus = tr("Não foi possível consultar atualizações.");
    }
    emit softwareChanged();
}

void SystemBackend::refreshServices()
{
    QDBusInterface servicesInterface(Service, ObjectPath, "org.lyraos.Vega1.Services",
                                     QDBusConnection::systemBus());
    m_services.clear();
    const QDBusMessage reply = servicesInterface.call(QStringLiteral("ListServicesLocalized"),
                                                       QStringLiteral("pt_BR"));
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString name, label, description;
            bool enabled = false, active = false, available = false;
            array.beginStructure();
            array >> name >> label >> description >> enabled >> active >> available;
            array.endStructure();
            QVariantMap item;
            item.insert(QStringLiteral("name"), name);
            item.insert(QStringLiteral("label"), label.isEmpty() ? name : label);
            item.insert(QStringLiteral("description"), description);
            item.insert(QStringLiteral("enabled"), enabled);
            item.insert(QStringLiteral("active"), active);
            item.insert(QStringLiteral("available"), available);
            m_services.append(item);
        }
        array.endArray();
    }
    emit servicesChanged();
}

void SystemBackend::refreshHardware()
{
    m_hardware.clear();
    QDBusInterface hardware(Service, ObjectPath, "org.lyraos.Vega1.Hardware",
                            QDBusConnection::systemBus());
    const QDBusMessage inventory = hardware.call(QStringLiteral("InventoryLocalized"),
                                                  QStringLiteral("pt_BR"));
    if (inventory.type() == QDBusMessage::ReplyMessage && !inventory.arguments().isEmpty()) {
        const QDBusArgument data = qvariant_cast<QDBusArgument>(inventory.arguments().first());
        QString cpu, gpu, ram;
        data.beginStructure();
        data >> cpu >> gpu >> ram;
        data.endStructure();
        m_hardware.insert(QStringLiteral("cpu"), cpu);
        m_hardware.insert(QStringLiteral("gpu"), gpu);
        m_hardware.insert(QStringLiteral("ram"), ram);
    }
    QDBusInterface kernel(Service, ObjectPath, "org.lyraos.Vega1.Kernel",
                          QDBusConnection::systemBus());
    const QDBusReply<QStringList> installed = kernel.call(QStringLiteral("ListInstalled"));
    m_kernels = installed.isValid() ? installed.value() : QStringList{};
    emit hardwareChanged();
}

void SystemBackend::refreshStorage()
{
    m_volumes.clear();
    QDBusInterface storage(Service, ObjectPath, "org.lyraos.Vega1.Storage",
                           QDBusConnection::systemBus());
    const QDBusMessage reply = storage.call(QStringLiteral("ListVolumes"));
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString name, path, type, fsType, size, used, avail, mountpoint, model;
            uint percent = 0;
            bool removable = false, canMount = false, canUnmount = false;
            array.beginStructure();
            array >> name >> path >> type >> fsType >> size >> used >> avail >> percent
                  >> mountpoint >> model >> removable >> canMount >> canUnmount;
            array.endStructure();
            QVariantMap item{{QStringLiteral("name"), name}, {QStringLiteral("path"), path},
                             {QStringLiteral("fsType"), fsType}, {QStringLiteral("size"), size},
                             {QStringLiteral("used"), used}, {QStringLiteral("percent"), percent},
                             {QStringLiteral("mountpoint"), mountpoint}, {QStringLiteral("model"), model},
                             {QStringLiteral("removable"), removable}};
            m_volumes.append(item);
        }
        array.endArray();
    }
    emit storageChanged();
}

void SystemBackend::refreshUsers()
{
    m_users.clear();
    QDBusInterface usersInterface(Service, ObjectPath, "org.lyraos.Vega1.Users",
                                  QDBusConnection::systemBus());
    const QDBusMessage reply = usersInterface.call(QStringLiteral("ListUsers"));
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString username, fullName;
            QStringList groups;
            bool admin = false;
            array.beginStructure();
            array >> username >> fullName >> groups >> admin;
            array.endStructure();
            QVariantMap item{{QStringLiteral("username"), username},
                             {QStringLiteral("fullName"), fullName},
                             {QStringLiteral("groups"), groups.join(QStringLiteral(", "))},
                             {QStringLiteral("admin"), admin}};
            m_users.append(item);
        }
        array.endArray();
    }
    emit usersChanged();
}
