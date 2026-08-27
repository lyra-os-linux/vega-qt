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
    Q_PROPERTY(QVariantList services READ services NOTIFY servicesChanged)
    Q_PROPERTY(QVariantMap hardware READ hardware NOTIFY hardwareChanged)
    Q_PROPERTY(QStringList kernels READ kernels NOTIFY hardwareChanged)
    Q_PROPERTY(QVariantList volumes READ volumes NOTIFY storageChanged)
    Q_PROPERTY(QVariantList users READ users NOTIFY usersChanged)
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
    QVariantList services() const { return m_services; }
    QVariantMap hardware() const { return m_hardware; }
    QStringList kernels() const { return m_kernels; }
    QVariantList volumes() const { return m_volumes; }
    QVariantList users() const { return m_users; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshSoftware();
    Q_INVOKABLE void refreshServices();
    Q_INVOKABLE void refreshHardware();
    Q_INVOKABLE void refreshStorage();
    Q_INVOKABLE void refreshUsers();
signals:
    void changed();
    void softwareChanged();
    void servicesChanged();
    void hardwareChanged();
    void storageChanged();
    void usersChanged();
private:
    bool m_connected = false;
    QString m_status;
    QString m_version;
    QString m_distro;
    QString m_disk;
    int m_diskPercent = 0;
    QString m_packageManager;
    QString m_softwareStatus;
    QVariantList m_services;
    QVariantMap m_hardware;
    QStringList m_kernels;
    QVariantList m_volumes;
    QVariantList m_users;
};
