#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QProcess>
#include <QSettings>
#include <QTimer>

class UpdateNotifier final : public QObject
{
    Q_OBJECT
public:
    explicit UpdateNotifier(QObject *parent = nullptr) : QObject(parent)
    {
        auto bus = QDBusConnection::sessionBus();
        bus.connect(QStringLiteral("org.freedesktop.Notifications"),
                    QStringLiteral("/org/freedesktop/Notifications"),
                    QStringLiteral("org.freedesktop.Notifications"),
                    QStringLiteral("ActionInvoked"), this,
                    SLOT(actionInvoked(uint,QString)));
        QDBusConnection::systemBus().connect(QStringLiteral("org.lyraos.Vega1"),
                    QStringLiteral("/org/lyraos/Vega1"),
                    QStringLiteral("org.lyraos.Vega1.Software"),
                    QStringLiteral("UpdatesAvailable"), this,
                    SLOT(updatesAvailable(uint)));
        QTimer::singleShot(15000, this, &UpdateNotifier::check);
        auto *timer = new QTimer(this);
        timer->setInterval(60 * 60 * 1000);
        connect(timer, &QTimer::timeout, this, &UpdateNotifier::check);
        timer->start();
    }

private slots:
    void check()
    {
        QDBusInterface software(QStringLiteral("org.lyraos.Vega1"),
            QStringLiteral("/org/lyraos/Vega1"),
            QStringLiteral("org.lyraos.Vega1.Software"), QDBusConnection::systemBus());
        const QDBusMessage reply = software.call(QStringLiteral("GetUpdateStatus"));
        if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
            return;
        const QDBusArgument status = qvariant_cast<QDBusArgument>(reply.arguments().first());
        QString checkedAt, profile, nativeError, flatpakError, error;
        uint nativeCount = 0, flatpakCount = 0, totalCount = 0, generation = 0;
        bool checking = false;
        status.beginStructure();
        status >> checkedAt >> profile >> nativeCount >> flatpakCount >> totalCount
               >> generation >> checking >> error;
        status.endStructure();
        updatesAvailable(totalCount);
    }

    void updatesAvailable(uint count)
    {
        QSettings settings(QStringLiteral("LyraOS"), QStringLiteral("VegaUpdateNotifier"));
        if (count == 0) {
            settings.setValue(QStringLiteral("lastCount"), 0);
            return;
        }
        if (settings.value(QStringLiteral("lastCount"), 0).toUInt() == count)
            return;
        settings.setValue(QStringLiteral("lastCount"), count);

        QDBusInterface notifications(QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("/org/freedesktop/Notifications"),
            QStringLiteral("org.freedesktop.Notifications"), QDBusConnection::sessionBus());
        QVariantMap hints{{QStringLiteral("desktop-entry"), QStringLiteral("org.lyraos.Vega.Qt")}};
        const QDBusMessage reply = notifications.call(QStringLiteral("Notify"),
            QStringLiteral("Vega"), m_notificationId,
            QStringLiteral("system-software-update"),
            tr("Atualizações disponíveis"),
            tr("%1 pacote(s) aguardando atualização").arg(count),
            QStringList{QStringLiteral("default"), QString(),
                        QStringLiteral("open"), tr("Abrir Software e Atualizações")},
            hints, -1);
        if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
            m_notificationId = reply.arguments().first().toUInt();
    }

    void actionInvoked(uint id, const QString &action)
    {
        if (id == m_notificationId && (action == QStringLiteral("open") || action == QStringLiteral("default")))
            QProcess::startDetached(QStringLiteral("/usr/bin/vega-qt"), {QStringLiteral("--software-updates")});
    }

private:
    uint m_notificationId = 0;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Vega Update Notifier"));
    UpdateNotifier notifier;
    return app.exec();
}

#include "update_notifier.moc"
