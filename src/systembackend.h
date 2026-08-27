#pragma once
#include <QObject>
#include <QVariant>

class SystemBackend final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString version READ version NOTIFY changed)
    Q_PROPERTY(QString distro READ distro NOTIFY changed)
    Q_PROPERTY(QString disk READ disk NOTIFY changed)
    Q_PROPERTY(int diskPercent READ diskPercent NOTIFY changed)
    Q_PROPERTY(QString packageManager READ packageManager NOTIFY softwareChanged)
    Q_PROPERTY(QString softwareStatus READ softwareStatus NOTIFY softwareChanged)
    Q_PROPERTY(QVariantList softwareResults READ softwareResults NOTIFY softwareChanged)
    Q_PROPERTY(QVariantList softwareUpdates READ softwareUpdates NOTIFY softwareChanged)
    Q_PROPERTY(QVariantList repositories READ repositories NOTIFY softwareChanged)
    Q_PROPERTY(bool softwareBusy READ softwareBusy NOTIFY softwareChanged)
    Q_PROPERTY(int transactionProgress READ transactionProgress NOTIFY transactionChanged)
    Q_PROPERTY(QString transactionMessage READ transactionMessage NOTIFY transactionChanged)
    Q_PROPERTY(QVariantList services READ services NOTIFY servicesChanged)
    Q_PROPERTY(bool showingAllServices READ showingAllServices NOTIFY servicesChanged)
    Q_PROPERTY(QVariantMap hardware READ hardware NOTIFY hardwareChanged)
    Q_PROPERTY(QStringList kernels READ kernels NOTIFY hardwareChanged)
    Q_PROPERTY(QVariantList volumes READ volumes NOTIFY storageChanged)
    Q_PROPERTY(QVariantList users READ users NOTIFY usersChanged)
    Q_PROPERTY(QVariantList networkInterfaces READ networkInterfaces NOTIFY networkChanged)
    Q_PROPERTY(QVariantList wifiNetworks READ wifiNetworks NOTIFY networkChanged)
    Q_PROPERTY(bool firewallEnabled READ firewallEnabled NOTIFY networkChanged)
    Q_PROPERTY(QString firewallZone READ firewallZone NOTIFY networkChanged)
    Q_PROPERTY(QVariantMap bluetoothStatus READ bluetoothStatus NOTIFY bluetoothChanged)
    Q_PROPERTY(QVariantList bluetoothDevices READ bluetoothDevices NOTIFY bluetoothChanged)
    Q_PROPERTY(QStringList logUnits READ logUnits NOTIFY logsChanged)
    Q_PROPERTY(QStringList logLines READ logLines NOTIFY logsChanged)
    Q_PROPERTY(bool logsUnlocked READ logsUnlocked NOTIFY logsChanged)
    Q_PROPERTY(QString logsStatus READ logsStatus NOTIFY logsChanged)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY monitorChanged)
    Q_PROPERTY(QVariantList processes READ processes NOTIFY monitorChanged)
    Q_PROPERTY(QVariantMap dateTimeStatus READ dateTimeStatus NOTIFY dateTimeChanged)
    Q_PROPERTY(QStringList timezones READ timezones NOTIFY dateTimeChanged)
    Q_PROPERTY(QStringList locales READ locales NOTIFY dateTimeChanged)
    Q_PROPERTY(QStringList keymaps READ keymaps NOTIFY dateTimeChanged)
    Q_PROPERTY(QVariantList backupConfigs READ backupConfigs NOTIFY backupChanged)
    Q_PROPERTY(QVariantList backupSnapshots READ backupSnapshots NOTIFY backupChanged)
    Q_PROPERTY(QString backupStatus READ backupStatus NOTIFY backupChanged)
    Q_PROPERTY(int backupProgress READ backupProgress NOTIFY backupChanged)
    Q_PROPERTY(QVariantMap appearance READ appearance NOTIFY appearanceChanged)
    Q_PROPERTY(QString appearanceStatus READ appearanceStatus NOTIFY appearanceChanged)
public:
    explicit SystemBackend(QObject *parent = nullptr);
    bool connected() const { return m_connected; }
    QString status() const { return m_status; }
    QString version() const { return m_version; }
    QString distro() const { return m_distro; }
    QString disk() const { return m_disk; }
    int diskPercent() const { return m_diskPercent; }
    QString packageManager() const { return m_packageManager; }
    QString softwareStatus() const { return m_softwareStatus; }
    QVariantList softwareResults() const { return m_softwareResults; }
    QVariantList softwareUpdates() const { return m_softwareUpdates; }
    QVariantList repositories() const { return m_repositories; }
    bool softwareBusy() const { return m_softwareBusy; }
    int transactionProgress() const { return m_transactionProgress; }
    QString transactionMessage() const { return m_transactionMessage; }
    QVariantList services() const { return m_services; }
    bool showingAllServices() const { return m_showingAllServices; }
    QVariantMap hardware() const { return m_hardware; }
    QStringList kernels() const { return m_kernels; }
    QVariantList volumes() const { return m_volumes; }
    QVariantList users() const { return m_users; }
    QVariantList networkInterfaces() const { return m_networkInterfaces; }
    QVariantList wifiNetworks() const { return m_wifiNetworks; }
    bool firewallEnabled() const { return m_firewallEnabled; }
    QString firewallZone() const { return m_firewallZone; }
    QVariantMap bluetoothStatus() const { return m_bluetoothStatus; }
    QVariantList bluetoothDevices() const { return m_bluetoothDevices; }
    QStringList logUnits() const { return m_logUnits; }
    QStringList logLines() const { return m_logLines; }
    bool logsUnlocked() const { return m_logsUnlocked; }
    QString logsStatus() const { return m_logsStatus; }
    QVariantMap metrics() const { return m_metrics; }
    QVariantList processes() const { return m_processes; }
    QVariantMap dateTimeStatus() const { return m_dateTimeStatus; }
    QStringList timezones() const { return m_timezones; }
    QStringList locales() const { return m_locales; }
    QStringList keymaps() const { return m_keymaps; }
    QVariantList backupConfigs() const { return m_backupConfigs; }
    QVariantList backupSnapshots() const { return m_backupSnapshots; }
    QString backupStatus() const { return m_backupStatus; }
    int backupProgress() const { return m_backupProgress; }
    QVariantMap appearance() const { return m_appearance; }
    QString appearanceStatus() const { return m_appearanceStatus; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshSoftware();
    Q_INVOKABLE void searchSoftware(const QString &query);
    Q_INVOKABLE void installPackage(const QString &origin, const QString &id);
    Q_INVOKABLE void removePackage(const QString &origin, const QString &id);
    Q_INVOKABLE void updateAll();
    Q_INVOKABLE void updatePackage(const QString &origin, const QString &id);
    Q_INVOKABLE void setRepositoryEnabled(const QString &name, bool enabled);
    Q_INVOKABLE void refreshServices(bool all = false);
    Q_INVOKABLE void refreshHardware();
    Q_INVOKABLE void refreshStorage();
    Q_INVOKABLE void refreshUsers();
    Q_INVOKABLE void refreshNetwork();
    Q_INVOKABLE void refreshBluetooth();
    Q_INVOKABLE void queryLogs(const QString &unit, const QString &priority, const QString &search);
    Q_INVOKABLE void refreshMonitor();
    Q_INVOKABLE void refreshDateTime();
    Q_INVOKABLE void applyDateTime(const QString &timezone, bool ntp,
                                   const QString &locale, const QString &keymap);
    Q_INVOKABLE void refreshBackup(const QString &configId = {});
    Q_INVOKABLE void createBackupConfig(const QString &id, const QString &paths,
                                        const QString &destination, const QString &destinationUuid,
                                        const QString &frequency);
    Q_INVOKABLE void deleteBackupConfig(const QString &id);
    Q_INVOKABLE void runBackup(const QString &id);
    Q_INVOKABLE void restoreBackup(const QString &snapshotId, const QString &targetPath);
    Q_INVOKABLE void refreshAppearance();
    Q_INVOKABLE void openAppearanceModule(const QString &module);
    Q_INVOKABLE void setDarkTheme(bool enabled);
signals:
    void changed();
    void softwareChanged();
    void transactionChanged();
    void servicesChanged();
    void hardwareChanged();
    void storageChanged();
    void usersChanged();
    void networkChanged();
    void bluetoothChanged();
    void logsChanged();
    void monitorChanged();
    void dateTimeChanged();
    void backupChanged();
    void appearanceChanged();
private:
    bool m_connected = false;
    QString m_status;
    QString m_version;
    QString m_distro;
    QString m_disk;
    int m_diskPercent = 0;
    QString m_packageManager;
    QString m_softwareStatus;
    QVariantList m_softwareResults;
    QVariantList m_softwareUpdates;
    QVariantList m_repositories;
    bool m_softwareBusy = false;
    uint m_transactionId = 0;
    int m_transactionProgress = 0;
    QString m_transactionMessage;
    QVariantList m_services;
    bool m_showingAllServices = false;
    QVariantMap m_hardware;
    QStringList m_kernels;
    QVariantList m_volumes;
    QVariantList m_users;
    QVariantList m_networkInterfaces;
    QVariantList m_wifiNetworks;
    bool m_firewallEnabled = false;
    QString m_firewallZone;
    QVariantMap m_bluetoothStatus;
    QVariantList m_bluetoothDevices;
    QStringList m_logUnits;
    QStringList m_logLines;
    bool m_logsUnlocked = false;
    QString m_logsStatus;
    QVariantMap m_metrics;
    QVariantList m_processes;
    QVariantMap m_dateTimeStatus;
    QStringList m_timezones;
    QStringList m_locales;
    QStringList m_keymaps;
    QVariantList m_backupConfigs;
    QVariantList m_backupSnapshots;
    QString m_backupConfigId;
    QString m_backupStatus;
    int m_backupProgress = 0;
    QVariantMap m_appearance;
    QString m_appearanceStatus;

private slots:
    void onTransactionProgress(uint transactionId, uint percent, const QString &message);
    void onTransactionFinished(uint transactionId, bool success, const QString &message);
    void onBackupProgress(uint transactionId, uint percent, const QString &message);
    void onBackupFinished(uint transactionId, bool success, const QString &message);
};
