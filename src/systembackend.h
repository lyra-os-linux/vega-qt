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
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshSoftware();
    Q_INVOKABLE void searchSoftware(const QString &query);
    Q_INVOKABLE void installPackage(const QString &origin, const QString &id);
    Q_INVOKABLE void removePackage(const QString &origin, const QString &id);
    Q_INVOKABLE void updateAll();
    Q_INVOKABLE void updatePackage(const QString &origin, const QString &id);
    Q_INVOKABLE void setRepositoryEnabled(const QString &name, bool enabled);
    Q_INVOKABLE void refreshServices();
    Q_INVOKABLE void refreshHardware();
    Q_INVOKABLE void refreshStorage();
    Q_INVOKABLE void refreshUsers();
    Q_INVOKABLE void refreshNetwork();
    Q_INVOKABLE void refreshBluetooth();
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

private slots:
    void onTransactionProgress(uint transactionId, uint percent, const QString &message);
    void onTransactionFinished(uint transactionId, bool success, const QString &message);
};
