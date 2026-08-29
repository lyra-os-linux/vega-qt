#include "systembackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDateTime>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QVariantMap>

namespace {
constexpr auto Service = "org.lyraos.Vega1";
constexpr auto ObjectPath = "/org/lyraos/Vega1";
constexpr auto Interface = "org.lyraos.Vega1.System";

QVariantList packageList(const QDBusMessage &reply)
{
    QVariantList packages;
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return packages;
    const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
    array.beginArray();
    while (!array.atEnd()) {
        QString origin, id, name, description, icon, repository;
        bool installed = false;
        array.beginStructure();
        array >> origin >> id >> name >> description >> installed >> icon >> repository;
        array.endStructure();
        packages.append(QVariantMap{{QStringLiteral("origin"), origin},
                                    {QStringLiteral("id"), id},
                                    {QStringLiteral("name"), name},
                                    {QStringLiteral("description"), description},
                                    {QStringLiteral("installed"), installed},
                                    {QStringLiteral("icon"), icon},
                                    {QStringLiteral("repository"), repository}});
    }
    array.endArray();
    return packages;
}
}

SystemBackend::SystemBackend(QObject *parent) : QObject(parent)
{
    m_status = tr("Carregando informações do sistema…");
    m_version = tr("Aguardando vegad");
    m_distro = tr("Lyra OS");
    m_disk = tr("Carregando…");
    m_packageManager = tr("Carregar ao abrir Software");
    m_softwareStatus = tr("Abra esta seção para consultar atualizações.");
    auto bus = QDBusConnection::systemBus();
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Software", "TransactionProgress",
                this, SLOT(onTransactionProgress(uint,uint,QString)));
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Software", "TransactionFinished",
                this, SLOT(onTransactionFinished(uint,bool,QString)));
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Backup", "BackupProgress",
                this, SLOT(onBackupProgress(uint,uint,QString)));
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Backup", "BackupFinished",
                this, SLOT(onBackupFinished(uint,bool,QString)));
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Backup", "RestoreProgress",
                this, SLOT(onBackupProgress(uint,uint,QString)));
    bus.connect(Service, ObjectPath, "org.lyraos.Vega1.Backup", "RestoreFinished",
                this, SLOT(onBackupFinished(uint,bool,QString)));
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
    if (m_softwareBusy)
        return;
    m_softwareBusy = true;
    emit softwareChanged();
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    if (!software.isValid()) {
        m_packageManager = tr("Indisponível");
        m_softwareStatus = tr("O módulo Software do vegad não está disponível.");
        m_softwareBusy = false;
        emit softwareChanged();
        return;
    }
    const QDBusReply<QString> manager = software.call(QStringLiteral("PackageManagerName"));
    m_packageManager = manager.isValid() ? manager.value() : tr("Zypper");
    m_softwareUpdates = packageList(software.call(QStringLiteral("ListUpdates")));
    if (!m_softwareUpdates.isEmpty()) {
        m_softwareStatus = tr("%1 atualização(ões) disponível(is)").arg(m_softwareUpdates.size());
    } else {
        const QDBusMessage native = software.call(QStringLiteral("ListNativeUpdates"));
        if (native.type() == QDBusMessage::ReplyMessage) {
            m_softwareUpdates = packageList(native);
            m_softwareStatus = m_softwareUpdates.isEmpty() ? tr("Sistema atualizado")
                : tr("%1 atualização(ões) disponível(is)").arg(m_softwareUpdates.size());
        } else {
        m_softwareStatus = tr("Não foi possível consultar atualizações.");
        }
    }
    m_repositories.clear();
    const QDBusMessage repos = software.call(QStringLiteral("ListRepos"));
    if (repos.type() == QDBusMessage::ReplyMessage && !repos.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(repos.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString name;
            bool enabled = false;
            array.beginStructure();
            array >> name >> enabled;
            array.endStructure();
            m_repositories.append(QVariantMap{{QStringLiteral("name"), name},
                                               {QStringLiteral("enabled"), enabled}});
        }
        array.endArray();
    }
    m_softwareBusy = false;
    emit softwareChanged();
}

void SystemBackend::searchSoftware(const QString &query)
{
    if (query.trimmed().size() < 2)
        return;
    m_softwareBusy = true;
    emit softwareChanged();
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    m_softwareResults = packageList(software.call(QStringLiteral("Search"), query.trimmed()));
    if (m_softwareResults.isEmpty())
        m_softwareResults = packageList(software.call(QStringLiteral("SearchNative"), query.trimmed()));
    m_softwareBusy = false;
    emit softwareChanged();
}

static uint startSoftwareTransaction(const QString &method, const QVariantList &arguments)
{
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    const QDBusMessage reply = software.callWithArgumentList(QDBus::Block, method, arguments);
    return reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
        ? reply.arguments().first().toUInt() : 0;
}

void SystemBackend::installPackage(const QString &origin, const QString &id)
{
    m_transactionId = startSoftwareTransaction(QStringLiteral("Install"), {origin, id});
    m_transactionProgress = 0;
    m_transactionMessage = m_transactionId ? tr("Instalação iniciada") : tr("Não foi possível iniciar a instalação");
    emit transactionChanged();
}

void SystemBackend::removePackage(const QString &origin, const QString &id)
{
    m_transactionId = startSoftwareTransaction(QStringLiteral("Remove"), {origin, id});
    m_transactionProgress = 0;
    m_transactionMessage = m_transactionId ? tr("Remoção iniciada") : tr("Não foi possível iniciar a remoção");
    emit transactionChanged();
}

void SystemBackend::updateAll()
{
    m_transactionId = startSoftwareTransaction(QStringLiteral("UpdateAll"), {});
    m_transactionProgress = 0;
    m_transactionMessage = m_transactionId ? tr("Atualização iniciada") : tr("Não foi possível iniciar a atualização");
    emit transactionChanged();
}

void SystemBackend::updatePackage(const QString &origin, const QString &id)
{
    m_transactionId = startSoftwareTransaction(QStringLiteral("UpdatePackage"), {origin, id});
    m_transactionProgress = 0;
    m_transactionMessage = m_transactionId ? tr("Atualização iniciada") : tr("Não foi possível iniciar a atualização");
    emit transactionChanged();
}

void SystemBackend::setRepositoryEnabled(const QString &name, bool enabled)
{
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    software.call(QStringLiteral("SetRepoEnabled"), name, enabled);
    refreshSoftware();
}

void SystemBackend::onTransactionProgress(uint transactionId, uint percent, const QString &message)
{
    if (transactionId != m_transactionId)
        return;
    m_transactionProgress = static_cast<int>(percent);
    m_transactionMessage = message;
    emit transactionChanged();
}

void SystemBackend::onTransactionFinished(uint transactionId, bool success, const QString &message)
{
    if (transactionId != m_transactionId)
        return;
    m_transactionProgress = success ? 100 : 0;
    m_transactionMessage = message;
    m_transactionId = 0;
    emit transactionChanged();
    refreshSoftware();
}

void SystemBackend::refreshServices(bool all)
{
    QDBusInterface servicesInterface(Service, ObjectPath, "org.lyraos.Vega1.Services",
                                     QDBusConnection::systemBus());
    m_showingAllServices = all;
    m_services.clear();
    const QDBusMessage reply = servicesInterface.call(all ? QStringLiteral("ListAllServicesLocalized")
                                                          : QStringLiteral("ListServicesLocalized"),
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

void SystemBackend::refreshNetwork()
{
    m_networkInterfaces.clear();
    m_wifiNetworks.clear();
    QDBusInterface network(Service, ObjectPath, "org.lyraos.Vega1.Network", QDBusConnection::systemBus());
    const QDBusMessage interfaces = network.call(QStringLiteral("ListInterfaces"));
    if (interfaces.type() == QDBusMessage::ReplyMessage && !interfaces.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(interfaces.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString name, type, state, ipv4, ipv6, gateway, dns, mac, speed, ssid, device;
            uint signal = 0; bool autoconf = false;
            array.beginStructure();
            array >> name >> type >> state >> ipv4 >> ipv6 >> gateway >> dns >> mac >> speed >> ssid >> signal >> device >> autoconf;
            array.endStructure();
            m_networkInterfaces.append(QVariantMap{{"name", name}, {"type", type}, {"state", state},
                {"ipv4", ipv4}, {"speed", speed}, {"ssid", ssid}, {"device", device}});
        }
        array.endArray();
    }
    const QDBusMessage wifi = network.call(QStringLiteral("ListWifi"));
    if (wifi.type() == QDBusMessage::ReplyMessage && !wifi.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(wifi.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString ssid, security, device; uint signal = 0; bool active = false;
            array.beginStructure(); array >> ssid >> security >> signal >> active >> device; array.endStructure();
            m_wifiNetworks.append(QVariantMap{{"ssid", ssid}, {"security", security}, {"signal", signal}, {"active", active}, {"device", device}});
        }
        array.endArray();
    }
    QDBusInterface firewall(Service, ObjectPath, "org.lyraos.Vega1.Firewall", QDBusConnection::systemBus());
    const QDBusMessage status = firewall.call(QStringLiteral("Status"));
    if (status.type() == QDBusMessage::ReplyMessage && status.arguments().size() >= 2) {
        m_firewallEnabled = status.arguments().at(0).toBool();
        m_firewallZone = status.arguments().at(1).toString();
    }
    emit networkChanged();
}

void SystemBackend::refreshBluetooth()
{
    m_bluetoothStatus.clear();
    m_bluetoothDevices.clear();
    QDBusInterface bluetooth(Service, ObjectPath, "org.lyraos.Vega1.Bluetooth", QDBusConnection::systemBus());
    const QDBusMessage status = bluetooth.call(QStringLiteral("Status"));
    if (status.type() == QDBusMessage::ReplyMessage && !status.arguments().isEmpty()) {
        const QDBusArgument data = qvariant_cast<QDBusArgument>(status.arguments().first());
        bool available, powered, discoverable, pairable, scanning, transferAvailable, receiverActive;
        QString controller, controllerName, receivePath;
        data.beginStructure();
        data >> available >> powered >> discoverable >> pairable >> scanning >> controller
             >> controllerName >> transferAvailable >> receiverActive >> receivePath;
        data.endStructure();
        m_bluetoothStatus = {{"available", available}, {"powered", powered}, {"discoverable", discoverable},
            {"scanning", scanning}, {"controller", controllerName.isEmpty() ? controller : controllerName}};
    }
    const QDBusMessage devices = bluetooth.call(QStringLiteral("ListDevices"));
    if (devices.type() == QDBusMessage::ReplyMessage && !devices.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(devices.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString address, name, alias, icon; bool paired, trusted, connected, blocked; int rssi;
            array.beginStructure(); array >> address >> name >> alias >> icon >> paired >> trusted >> connected >> blocked >> rssi; array.endStructure();
            m_bluetoothDevices.append(QVariantMap{{"address", address}, {"name", alias.isEmpty() ? name : alias},
                {"icon", icon}, {"paired", paired}, {"trusted", trusted}, {"connected", connected}, {"rssi", rssi}});
        }
        array.endArray();
    }
    emit bluetoothChanged();
}

void SystemBackend::queryLogs(const QString &unit, const QString &priority, const QString &search)
{
    QDBusInterface logs(Service, ObjectPath, "org.lyraos.Vega1.Logs", QDBusConnection::systemBus());
    const QDBusReply<QStringList> units = logs.call(QStringLiteral("ListUnits"));
    if (!units.isValid()) {
        m_logsUnlocked = false;
        m_logsStatus = tr("A autenticação administrativa foi cancelada ou recusada.");
        m_logUnits.clear();
        m_logLines.clear();
        emit logsChanged();
        return;
    }
    m_logUnits = units.value();
    const QDBusReply<QStringList> lines = logs.call(QStringLiteral("Query"), unit, priority,
                                                    QStringLiteral("today"), search, 300u);
    if (!lines.isValid()) {
        m_logsUnlocked = false;
        m_logsStatus = tr("Não foi possível consultar o log administrativo.");
        m_logLines.clear();
        emit logsChanged();
        return;
    }
    m_logsUnlocked = true;
    m_logsStatus = lines.value().isEmpty() ? tr("Nenhuma entrada encontrada para os filtros atuais.") : QString{};
    m_logLines = lines.value();
    emit logsChanged();
}

void SystemBackend::refreshMonitor()
{
    m_metrics.clear();
    m_processes.clear();
    QDBusInterface monitor(Service, ObjectPath, "org.lyraos.Vega1.Monitor", QDBusConnection::systemBus());
    const QDBusMessage metricsReply = monitor.call(QStringLiteral("Metrics"));
    if (metricsReply.type() == QDBusMessage::ReplyMessage && !metricsReply.arguments().isEmpty()) {
        const QDBusArgument data = qvariant_cast<QDBusArgument>(metricsReply.arguments().first());
        double cpu = 0, gpu = -1; qulonglong memUsed, memTotal, swapUsed, swapTotal, diskRead, diskWrite, netRx, netTx;
        QList<double> cores, gpus;
        data.beginStructure();
        data >> cpu >> memUsed >> memTotal >> swapUsed >> swapTotal >> diskRead >> diskWrite
             >> netRx >> netTx >> cores >> gpu >> gpus;
        data.endStructure();
        const double memPercent = memTotal ? (100.0 * memUsed / memTotal) : 0;
        m_metrics = {{"cpu", cpu}, {"memory", memPercent}, {"memUsed", QVariant::fromValue(memUsed)},
                     {"memTotal", QVariant::fromValue(memTotal)}, {"gpu", gpu},
                     {"netRx", QVariant::fromValue(netRx)}, {"netTx", QVariant::fromValue(netTx)}};
    }
    const QDBusMessage processReply = monitor.call(QStringLiteral("ListProcesses"));
    if (processReply.type() == QDBusMessage::ReplyMessage && !processReply.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(processReply.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            uint pid, ppid; QString name, user, state; double cpu; qulonglong memory;
            array.beginStructure(); array >> pid >> ppid >> name >> user >> cpu >> memory >> state; array.endStructure();
            m_processes.append(QVariantMap{{"pid", pid}, {"name", name}, {"user", user},
                {"cpu", cpu}, {"memory", QVariant::fromValue(memory)}, {"state", state}});
        }
        array.endArray();
    }
    emit monitorChanged();
}

void SystemBackend::refreshDateTime()
{
    QDBusInterface dateTime(Service, ObjectPath, "org.lyraos.Vega1.DateTime",
                            QDBusConnection::systemBus());
    m_dateTimeStatus.clear();
    const QDBusMessage status = dateTime.call(QStringLiteral("Status"));
    if (status.type() == QDBusMessage::ReplyMessage && !status.arguments().isEmpty()) {
        const QDBusArgument data = qvariant_cast<QDBusArgument>(status.arguments().first());
        QString timezone, locale, keymap;
        bool ntp = false;
        data.beginStructure();
        data >> timezone >> ntp >> locale >> keymap;
        data.endStructure();
        m_dateTimeStatus = {{QStringLiteral("timezone"), timezone},
                            {QStringLiteral("ntp"), ntp},
                            {QStringLiteral("locale"), locale},
                            {QStringLiteral("keymap"), keymap}};
    }
    const QDBusReply<QStringList> timezones = dateTime.call(QStringLiteral("ListTimezones"));
    const QDBusReply<QStringList> locales = dateTime.call(QStringLiteral("ListLocales"));
    const QDBusReply<QStringList> keymaps = dateTime.call(QStringLiteral("ListKeymaps"));
    m_timezones = timezones.isValid() ? timezones.value() : QStringList{};
    m_locales = locales.isValid() ? locales.value() : QStringList{};
    m_keymaps = keymaps.isValid() ? keymaps.value() : QStringList{};
    emit dateTimeChanged();
}

void SystemBackend::applyDateTime(const QString &timezone, bool ntp,
                                  const QString &locale, const QString &keymap)
{
    QDBusInterface dateTime(Service, ObjectPath, "org.lyraos.Vega1.DateTime",
                            QDBusConnection::systemBus());
    dateTime.call(QStringLiteral("Apply"), timezone, ntp, locale, keymap);
    refreshDateTime();
}

void SystemBackend::refreshBackup(const QString &configId)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    m_backupConfigs.clear();
    const QDBusMessage configs = backup.call(QStringLiteral("ListConfigs"));
    if (configs.type() == QDBusMessage::ReplyMessage && !configs.arguments().isEmpty()) {
        const QDBusArgument array = qvariant_cast<QDBusArgument>(configs.arguments().first());
        array.beginArray();
        while (!array.atEnd()) {
            QString id, destination, uuid, frequency;
            QStringList paths;
            array.beginStructure(); array >> id >> paths >> destination >> uuid >> frequency; array.endStructure();
            m_backupConfigs.append(QVariantMap{{"id", id}, {"paths", paths.join(", ")},
                {"destination", destination}, {"uuid", uuid}, {"frequency", frequency}});
        }
        array.endArray();
    }
    if (!configId.isEmpty())
        m_backupConfigId = configId;
    if (m_backupConfigId.isEmpty() && !m_backupConfigs.isEmpty())
        m_backupConfigId = m_backupConfigs.first().toMap().value("id").toString();
    m_backupSnapshots.clear();
    if (!m_backupConfigId.isEmpty()) {
        const QDBusMessage snapshots = backup.call(QStringLiteral("ListSnapshots"), m_backupConfigId);
        if (snapshots.type() == QDBusMessage::ReplyMessage && !snapshots.arguments().isEmpty()) {
            const QDBusArgument array = qvariant_cast<QDBusArgument>(snapshots.arguments().first());
            array.beginArray();
            while (!array.atEnd()) {
                QString id; qlonglong timestamp = 0; qulonglong files = 0, bytes = 0;
                array.beginStructure(); array >> id >> timestamp >> files >> bytes; array.endStructure();
                m_backupSnapshots.append(QVariantMap{{"id", id},
                    {"date", QDateTime::fromSecsSinceEpoch(timestamp).toLocalTime().toString("dd/MM/yyyy HH:mm")},
                    {"files", files}, {"bytes", bytes}});
            }
            array.endArray();
        }
    }
    emit backupChanged();
}

void SystemBackend::createBackupConfig(const QString &id, const QString &paths,
                                       const QString &destination, const QString &destinationUuid,
                                       const QString &frequency)
{
    QStringList pathList;
    for (const QString &path : paths.split(',', Qt::SkipEmptyParts))
        pathList.append(path.trimmed());
    QDBusArgument config;
    config.beginStructure();
    config << id.trimmed() << pathList << destination.trimmed() << destinationUuid.trimmed() << frequency;
    config.endStructure();
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    const QDBusMessage reply = backup.call(QStringLiteral("CreateConfig"), QVariant::fromValue(config));
    m_backupStatus = reply.type() == QDBusMessage::ReplyMessage
        ? tr("Configuração de backup criada.") : tr("Não foi possível criar a configuração.");
    refreshBackup();
}

void SystemBackend::deleteBackupConfig(const QString &id)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    const QDBusMessage reply = backup.call(QStringLiteral("DeleteConfig"), id);
    if (reply.type() == QDBusMessage::ReplyMessage) {
        m_backupConfigId.clear();
        m_backupStatus = tr("Configuração removida.");
    } else {
        m_backupStatus = tr("Não foi possível remover a configuração.");
    }
    refreshBackup();
}

void SystemBackend::runBackup(const QString &id)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    const QDBusMessage reply = backup.call(QStringLiteral("RunBackupNow"), id);
    m_backupProgress = 0;
    m_backupStatus = reply.type() == QDBusMessage::ReplyMessage ? tr("Backup iniciado.") : tr("Não foi possível iniciar o backup.");
    emit backupChanged();
}

void SystemBackend::restoreBackup(const QString &snapshotId, const QString &targetPath)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    const QDBusMessage reply = backup.call(QStringLiteral("RestoreSnapshot"), snapshotId,
                                           targetPath.trimmed(), QStringLiteral("separate-folder"));
    m_backupProgress = 0;
    m_backupStatus = reply.type() == QDBusMessage::ReplyMessage
        ? tr("Restauração iniciada em uma pasta separada.") : tr("Não foi possível iniciar a restauração.");
    emit backupChanged();
}

void SystemBackend::onBackupProgress(uint, uint percent, const QString &message)
{
    m_backupProgress = static_cast<int>(percent);
    m_backupStatus = message;
    emit backupChanged();
}

void SystemBackend::onBackupFinished(uint, bool success, const QString &message)
{
    m_backupProgress = success ? 100 : 0;
    m_backupStatus = message;
    emit backupChanged();
    refreshBackup(m_backupConfigId);
}

void SystemBackend::refreshAppearance()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/kdeglobals");
    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("KDE"));
    const QString widgetStyle = settings.value(QStringLiteral("widgetStyle"), tr("Padrão do Plasma")).toString();
    settings.endGroup();
    settings.beginGroup(QStringLiteral("General"));
    const QString colorScheme = settings.value(QStringLiteral("ColorScheme"), tr("Padrão do Plasma")).toString();
    const QString font = settings.value(QStringLiteral("font"), tr("Fonte do sistema")).toString().section(',', 0, 0);
    settings.endGroup();
    settings.beginGroup(QStringLiteral("Icons"));
    const QString icons = settings.value(QStringLiteral("Theme"), tr("Padrão do Plasma")).toString();
    settings.endGroup();
    m_appearance = {{QStringLiteral("style"), widgetStyle},
                    {QStringLiteral("colors"), colorScheme},
                    {QStringLiteral("icons"), icons},
                    {QStringLiteral("font"), font}};
    emit appearanceChanged();
}

void SystemBackend::openAppearanceModule(const QString &module)
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("kcmshell6"));
    if (executable.isEmpty()) {
        m_appearanceStatus = tr("As Configurações do Sistema do Plasma não estão instaladas nesta sessão.");
        emit appearanceChanged();
        return;
    }
    const bool started = QProcess::startDetached(executable, {module});
    m_appearanceStatus = started ? QString{} : tr("Não foi possível abrir este módulo do Plasma.");
    emit appearanceChanged();
}

void SystemBackend::setDarkTheme(bool enabled)
{
    QPalette palette;
    const QColor window = enabled ? QColor("#191c20") : QColor("#f6f7f9");
    const QColor base = enabled ? QColor("#23272d") : QColor("#ffffff");
    const QColor alternate = enabled ? QColor("#20242a") : QColor("#eef1f5");
    const QColor text = enabled ? QColor("#f1f3f5") : QColor("#20242a");
    const QColor disabled = enabled ? QColor("#7f8995") : QColor("#87919d");
    const QColor border = enabled ? QColor("#3b424b") : QColor("#cdd3da");

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, alternate);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, base);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Highlight, QColor("#2777c7"));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, disabled);
    palette.setColor(QPalette::Light, enabled ? QColor("#454d58") : Qt::white);
    palette.setColor(QPalette::Midlight, border);
    palette.setColor(QPalette::Mid, border);
    palette.setColor(QPalette::Dark, enabled ? QColor("#111316") : QColor("#aeb6c0"));
    palette.setColor(QPalette::Shadow, enabled ? Qt::black : QColor("#68717c"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    QGuiApplication::setPalette(palette);
    if (QGuiApplication::styleHints())
        QGuiApplication::styleHints()->setColorScheme(enabled ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
}
