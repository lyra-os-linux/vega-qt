#pragma once
#include <QObject>

class SystemBackend final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString version READ version NOTIFY changed)
    Q_PROPERTY(QString distro READ distro NOTIFY changed)
    Q_PROPERTY(QString disk READ disk NOTIFY changed)
    Q_PROPERTY(int diskPercent READ diskPercent NOTIFY changed)
public:
    explicit SystemBackend(QObject *parent = nullptr);
    bool connected() const { return m_connected; }
    QString status() const { return m_status; }
    QString version() const { return m_version; }
    QString distro() const { return m_distro; }
    QString disk() const { return m_disk; }
    int diskPercent() const { return m_diskPercent; }
    Q_INVOKABLE void refresh();
signals:
    void changed();
private:
    bool m_connected = false;
    QString m_status;
    QString m_version;
    QString m_distro;
    QString m_disk;
    int m_diskPercent = 0;
};
