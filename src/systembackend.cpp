#include "systembackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
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
#include <functional>

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

void watchCall(QObject *owner, const QDBusPendingCall &call,
               std::function<void(const QDBusMessage &)> finished)
{
    auto *watcher = new QDBusPendingCallWatcher(call, owner);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, owner,
                     [watcher, finished = std::move(finished)] {
        finished(watcher->reply());
        watcher->deleteLater();
    });
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
    const quint64 requestId = ++m_refreshRequestId;
    m_status = tr("Carregando informações do sistema…");
    emit changed();
    watchCall(this, system.asyncCall(QStringLiteral("Ping")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_refreshRequestId) return;
        m_connected = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            && reply.arguments().first().toBool();
        m_status = m_connected ? tr("Sistema conectado") : tr("Falha na comunicação");
        emit changed();
    });
    watchCall(this, system.asyncCall(QStringLiteral("Version")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_refreshRequestId) return;
        m_version = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            ? reply.arguments().first().toString() : tr("Desconhecida");
        emit changed();
    });
    watchCall(this, system.asyncCall(QStringLiteral("Distro")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_refreshRequestId) return;
        m_distro = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            ? reply.arguments().first().toString() : tr("Não detectada");
        emit changed();
    });
    watchCall(this, system.asyncCall(QStringLiteral("DiskUsage")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_refreshRequestId) return;
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().size() == 3) {
            m_disk = tr("%1 usados de %2").arg(reply.arguments().at(0).toString(), reply.arguments().at(1).toString());
            m_diskPercent = static_cast<int>(reply.arguments().at(2).toUInt());
        } else {
            m_disk = tr("Informação indisponível");
            m_diskPercent = 0;
        }
        emit changed();
    });
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
    const quint64 requestId = ++m_softwareRequestId;
    m_softwareStatus = tr("Consultando atualizações…");
    m_repositories.clear();
    emit softwareChanged();
    watchCall(this, software.asyncCall(QStringLiteral("PackageManagerName")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_softwareRequestId) return;
        m_packageManager = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            ? reply.arguments().first().toString() : tr("Zypper");
        emit softwareChanged();
    });
    watchCall(this, software.asyncCall(QStringLiteral("ListRepos")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_softwareRequestId) return;
        m_repositories.clear();
        if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
            const QDBusArgument array = qvariant_cast<QDBusArgument>(reply.arguments().first());
            array.beginArray();
            while (!array.atEnd()) {
                QString name; bool enabled = false;
                array.beginStructure(); array >> name >> enabled; array.endStructure();
                m_repositories.append(QVariantMap{{QStringLiteral("name"), name},
                                                   {QStringLiteral("enabled"), enabled}});
            }
            array.endArray();
        }
        emit softwareChanged();
    });
    watchCall(this, software.asyncCall(QStringLiteral("ListUpdates")), [this, requestId](const QDBusMessage &reply) {
        if (requestId != m_softwareRequestId) return;
        m_softwareUpdates = packageList(reply);
        if (!m_softwareUpdates.isEmpty()) {
            m_softwareStatus = tr("%1 atualização(ões) disponível(is)").arg(m_softwareUpdates.size());
            m_softwareBusy = false;
            emit softwareChanged();
            return;
        }
        QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software", QDBusConnection::systemBus());
        watchCall(this, software.asyncCall(QStringLiteral("ListNativeUpdates")), [this, requestId](const QDBusMessage &native) {
            if (requestId != m_softwareRequestId) return;
            m_softwareUpdates = packageList(native);
            m_softwareStatus = native.type() == QDBusMessage::ReplyMessage
                ? (m_softwareUpdates.isEmpty() ? tr("Sistema atualizado")
                   : tr("%1 atualização(ões) disponível(is)").arg(m_softwareUpdates.size()))
                : tr("Não foi possível consultar atualizações.");
            m_softwareBusy = false;
            emit softwareChanged();
        });
    });
}

void SystemBackend::searchSoftware(const QString &query)
{
    if (query.trimmed().size() < 2)
        return;
    const quint64 requestId = ++m_searchRequestId;
    m_softwareBusy = true;
    emit softwareChanged();
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    const QString term = query.trimmed();
    watchCall(this, software.asyncCall(QStringLiteral("Search"), term), [this, requestId, term](const QDBusMessage &reply) {
        if (requestId != m_searchRequestId) return;
        m_softwareResults = packageList(reply);
        if (!m_softwareResults.isEmpty()) {
            m_softwareBusy = false; emit softwareChanged(); return;
        }
        QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software", QDBusConnection::systemBus());
        watchCall(this, software.asyncCall(QStringLiteral("SearchNative"), term), [this, requestId](const QDBusMessage &native) {
            if (requestId != m_searchRequestId) return;
            m_softwareResults = packageList(native);
            m_softwareBusy = false;
            emit softwareChanged();
        });
    });
}

static void startSoftwareTransaction(QObject *owner, const QString &method,
                                     const QVariantList &arguments,
                                     std::function<void(uint)> finished)
{
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    watchCall(owner, software.asyncCallWithArgumentList(method, arguments),
              [finished = std::move(finished)](const QDBusMessage &reply) {
        finished(reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            ? reply.arguments().first().toUInt() : 0);
    });
}

void SystemBackend::installPackage(const QString &origin, const QString &id)
{
    m_transactionProgress = 0;
    m_transactionMessage = tr("Solicitando instalação…");
    emit transactionChanged();
    startSoftwareTransaction(this, QStringLiteral("Install"), {origin, id}, [this](uint id) {
        m_transactionId = id;
        m_transactionMessage = id ? tr("Instalação iniciada") : tr("Não foi possível iniciar a instalação");
        emit transactionChanged();
    });
}

void SystemBackend::removePackage(const QString &origin, const QString &id)
{
    m_transactionProgress = 0;
    m_transactionMessage = tr("Solicitando remoção…");
    emit transactionChanged();
    startSoftwareTransaction(this, QStringLiteral("Remove"), {origin, id}, [this](uint id) {
        m_transactionId = id;
        m_transactionMessage = id ? tr("Remoção iniciada") : tr("Não foi possível iniciar a remoção");
        emit transactionChanged();
    });
}

void SystemBackend::updateAll()
{
    m_transactionProgress = 0;
    m_transactionMessage = tr("Solicitando atualização…");
    emit transactionChanged();
    startSoftwareTransaction(this, QStringLiteral("UpdateAll"), {}, [this](uint id) {
        m_transactionId = id;
        m_transactionMessage = id ? tr("Atualização iniciada") : tr("Não foi possível iniciar a atualização");
        emit transactionChanged();
    });
}

void SystemBackend::updatePackage(const QString &origin, const QString &id)
{
    m_transactionProgress = 0;
    m_transactionMessage = tr("Solicitando atualização…");
    emit transactionChanged();
    startSoftwareTransaction(this, QStringLiteral("UpdatePackage"), {origin, id}, [this](uint id) {
        m_transactionId = id;
        m_transactionMessage = id ? tr("Atualização iniciada") : tr("Não foi possível iniciar a atualização");
        emit transactionChanged();
    });
}

void SystemBackend::setRepositoryEnabled(const QString &name, bool enabled)
{
    QDBusInterface software(Service, ObjectPath, "org.lyraos.Vega1.Software",
                            QDBusConnection::systemBus());
    watchCall(this, software.asyncCall(QStringLiteral("SetRepoEnabled"), name, enabled),
              [this](const QDBusMessage &) { refreshSoftware(); });
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
    emit servicesChanged();
    watchCall(this, servicesInterface.asyncCall(all ? QStringLiteral("ListAllServicesLocalized")
                                                    : QStringLiteral("ListServicesLocalized"),
                                                   QStringLiteral("pt_BR")), [this](const QDBusMessage &reply) {
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
    });
}

void SystemBackend::refreshHardware()
{
    m_hardware.clear();
    QDBusInterface hardware(Service, ObjectPath, "org.lyraos.Vega1.Hardware",
                            QDBusConnection::systemBus());
    emit hardwareChanged();
    watchCall(this, hardware.asyncCall(QStringLiteral("InventoryLocalized"),
                                       QStringLiteral("pt_BR")), [this](const QDBusMessage &inventory) {
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
      emit hardwareChanged();
    });
    QDBusInterface kernel(Service, ObjectPath, "org.lyraos.Vega1.Kernel",
                          QDBusConnection::systemBus());
    watchCall(this, kernel.asyncCall(QStringLiteral("ListInstalled")), [this](const QDBusMessage &reply) {
        m_kernels = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
            ? reply.arguments().first().toStringList() : QStringList{};
        emit hardwareChanged();
    });
}

void SystemBackend::refreshStorage()
{
    m_volumes.clear();
    QDBusInterface storage(Service, ObjectPath, "org.lyraos.Vega1.Storage",
                           QDBusConnection::systemBus());
    emit storageChanged();
    watchCall(this, storage.asyncCall(QStringLiteral("ListVolumes")), [this](const QDBusMessage &reply) {
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
    });
}

void SystemBackend::refreshUsers()
{
    m_users.clear();
    QDBusInterface usersInterface(Service, ObjectPath, "org.lyraos.Vega1.Users",
                                  QDBusConnection::systemBus());
    emit usersChanged();
    watchCall(this, usersInterface.asyncCall(QStringLiteral("ListUsers")), [this](const QDBusMessage &reply) {
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
    });
}

void SystemBackend::refreshNetwork()
{
    m_networkInterfaces.clear();
    m_wifiNetworks.clear();
    QDBusInterface network(Service, ObjectPath, "org.lyraos.Vega1.Network", QDBusConnection::systemBus());
    emit networkChanged();
    watchCall(this, network.asyncCall(QStringLiteral("ListInterfaces")), [this](const QDBusMessage &interfaces) {
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
      emit networkChanged();
    });
    watchCall(this, network.asyncCall(QStringLiteral("ListWifi")), [this](const QDBusMessage &wifi) {
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
      emit networkChanged();
    });
    QDBusInterface firewall(Service, ObjectPath, "org.lyraos.Vega1.Firewall", QDBusConnection::systemBus());
    watchCall(this, firewall.asyncCall(QStringLiteral("Status")), [this](const QDBusMessage &status) {
        if (status.type() == QDBusMessage::ReplyMessage && status.arguments().size() >= 2) {
            m_firewallEnabled = status.arguments().at(0).toBool();
            m_firewallZone = status.arguments().at(1).toString();
        }
        emit networkChanged();
    });
}

void SystemBackend::refreshBluetooth()
{
    m_bluetoothStatus.clear();
    m_bluetoothDevices.clear();
    QDBusInterface bluetooth(Service, ObjectPath, "org.lyraos.Vega1.Bluetooth", QDBusConnection::systemBus());
    emit bluetoothChanged();
    watchCall(this, bluetooth.asyncCall(QStringLiteral("Status")), [this](const QDBusMessage &status) {
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
      emit bluetoothChanged();
    });
    watchCall(this, bluetooth.asyncCall(QStringLiteral("ListDevices")), [this](const QDBusMessage &devices) {
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
    });
}

void SystemBackend::queryLogs(const QString &unit, const QString &priority, const QString &search)
{
    QDBusInterface logs(Service, ObjectPath, "org.lyraos.Vega1.Logs", QDBusConnection::systemBus());
    m_logsStatus = tr("Consultando o log…");
    emit logsChanged();
    watchCall(this, logs.asyncCall(QStringLiteral("ListUnits")),
              [this, unit, priority, search](const QDBusMessage &unitsReply) {
        if (unitsReply.type() != QDBusMessage::ReplyMessage || unitsReply.arguments().isEmpty()) {
            m_logsUnlocked = false;
            m_logsStatus = tr("A autenticação administrativa foi cancelada ou recusada.");
            m_logUnits.clear(); m_logLines.clear(); emit logsChanged(); return;
        }
        m_logUnits = unitsReply.arguments().first().toStringList();
        QDBusInterface logs(Service, ObjectPath, "org.lyraos.Vega1.Logs", QDBusConnection::systemBus());
        watchCall(this, logs.asyncCall(QStringLiteral("Query"), unit, priority,
                                       QStringLiteral("today"), search, 300u),
                  [this](const QDBusMessage &linesReply) {
            if (linesReply.type() != QDBusMessage::ReplyMessage || linesReply.arguments().isEmpty()) {
                m_logsUnlocked = false;
                m_logsStatus = tr("Não foi possível consultar o log administrativo.");
                m_logLines.clear(); emit logsChanged(); return;
            }
            m_logLines = linesReply.arguments().first().toStringList();
            m_logsUnlocked = true;
            m_logsStatus = m_logLines.isEmpty() ? tr("Nenhuma entrada encontrada para os filtros atuais.") : QString{};
            emit logsChanged();
        });
    });
}

void SystemBackend::refreshMonitor()
{
    m_metrics.clear();
    m_processes.clear();
    QDBusInterface monitor(Service, ObjectPath, "org.lyraos.Vega1.Monitor", QDBusConnection::systemBus());
    emit monitorChanged();
    watchCall(this, monitor.asyncCall(QStringLiteral("Metrics")), [this](const QDBusMessage &metricsReply) {
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
      emit monitorChanged();
    });
    watchCall(this, monitor.asyncCall(QStringLiteral("ListProcesses")), [this](const QDBusMessage &processReply) {
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
    });
}

void SystemBackend::refreshDateTime()
{
    QDBusInterface dateTime(Service, ObjectPath, "org.lyraos.Vega1.DateTime",
                            QDBusConnection::systemBus());
    m_dateTimeStatus.clear();
    emit dateTimeChanged();
    watchCall(this, dateTime.asyncCall(QStringLiteral("Status")), [this](const QDBusMessage &status) {
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
      emit dateTimeChanged();
    });
    const auto updateList = [this](QStringList SystemBackend::*member) {
        return [this, member](const QDBusMessage &reply) {
            this->*member = reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()
                ? reply.arguments().first().toStringList() : QStringList{};
            emit dateTimeChanged();
        };
    };
    watchCall(this, dateTime.asyncCall(QStringLiteral("ListTimezones")), updateList(&SystemBackend::m_timezones));
    watchCall(this, dateTime.asyncCall(QStringLiteral("ListLocales")), updateList(&SystemBackend::m_locales));
    watchCall(this, dateTime.asyncCall(QStringLiteral("ListKeymaps")), updateList(&SystemBackend::m_keymaps));
}

void SystemBackend::applyDateTime(const QString &timezone, bool ntp,
                                  const QString &locale, const QString &keymap)
{
    QDBusInterface dateTime(Service, ObjectPath, "org.lyraos.Vega1.DateTime",
                            QDBusConnection::systemBus());
    watchCall(this, dateTime.asyncCall(QStringLiteral("Apply"), timezone, ntp, locale, keymap),
              [this](const QDBusMessage &) { refreshDateTime(); });
}

void SystemBackend::refreshBackup(const QString &configId)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    m_backupConfigs.clear();
    m_backupSnapshots.clear();
    if (!configId.isEmpty())
        m_backupConfigId = configId;
    emit backupChanged();
    watchCall(this, backup.asyncCall(QStringLiteral("ListConfigs")), [this](const QDBusMessage &configs) {
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
      if (m_backupConfigId.isEmpty() && !m_backupConfigs.isEmpty())
          m_backupConfigId = m_backupConfigs.first().toMap().value("id").toString();
      emit backupChanged();
      if (m_backupConfigId.isEmpty()) return;
      QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
      watchCall(this, backup.asyncCall(QStringLiteral("ListSnapshots"), m_backupConfigId),
                [this](const QDBusMessage &snapshots) {
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
        emit backupChanged();
      });
    });
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
    m_backupStatus = tr("Criando configuração de backup…");
    emit backupChanged();
    watchCall(this, backup.asyncCall(QStringLiteral("CreateConfig"), QVariant::fromValue(config)),
              [this](const QDBusMessage &reply) {
        m_backupStatus = reply.type() == QDBusMessage::ReplyMessage
            ? tr("Configuração de backup criada.") : tr("Não foi possível criar a configuração.");
        refreshBackup();
    });
}

void SystemBackend::deleteBackupConfig(const QString &id)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    m_backupStatus = tr("Removendo configuração…");
    emit backupChanged();
    watchCall(this, backup.asyncCall(QStringLiteral("DeleteConfig"), id),
              [this](const QDBusMessage &reply) {
        if (reply.type() == QDBusMessage::ReplyMessage) {
            m_backupConfigId.clear();
            m_backupStatus = tr("Configuração removida.");
        } else {
            m_backupStatus = tr("Não foi possível remover a configuração.");
        }
        refreshBackup();
    });
}

void SystemBackend::runBackup(const QString &id)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    m_backupProgress = 0;
    m_backupStatus = tr("Iniciando backup…");
    emit backupChanged();
    watchCall(this, backup.asyncCall(QStringLiteral("RunBackupNow"), id),
              [this](const QDBusMessage &reply) {
        m_backupStatus = reply.type() == QDBusMessage::ReplyMessage
            ? tr("Backup iniciado.") : tr("Não foi possível iniciar o backup.");
        emit backupChanged();
    });
}

void SystemBackend::restoreBackup(const QString &snapshotId, const QString &targetPath)
{
    QDBusInterface backup(Service, ObjectPath, "org.lyraos.Vega1.Backup", QDBusConnection::systemBus());
    m_backupProgress = 0;
    m_backupStatus = tr("Iniciando restauração…");
    emit backupChanged();
    watchCall(this, backup.asyncCall(QStringLiteral("RestoreSnapshot"), snapshotId,
                                     targetPath.trimmed(), QStringLiteral("separate-folder")),
              [this](const QDBusMessage &reply) {
        m_backupStatus = reply.type() == QDBusMessage::ReplyMessage
            ? tr("Restauração iniciada em uma pasta separada.") : tr("Não foi possível iniciar a restauração.");
        emit backupChanged();
    });
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
