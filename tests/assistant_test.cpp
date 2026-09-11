#include "assistantbackend.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QSettings>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

// Real HTTP transport; only the remote address changes. No provider or real keyring is contacted.
class LocalNetwork final : public QNetworkAccessManager {
public:
    QUrl endpoint;
    QPointer<QNetworkReply> lastReply;
    int requests = 0;
protected:
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request,
                                 QIODevice *body) override
    {
        QNetworkRequest local(request);
        local.setUrl(endpoint);
        ++requests;
        lastReply = QNetworkAccessManager::createRequest(operation, local, body);
        return lastReply;
    }
};

class HttpPeer final : public QTcpServer {
public:
    QByteArray body = R"({"choices":[{"message":{"content":"resposta"}}]})";
    enum Mode { Complete, Silent, Partial, Trickle, HttpError, Drop } mode = Complete;
    int chunks = 0;
    QList<QPointer<QTcpSocket>> sockets;
    HttpPeer()
    {
        connect(this, &QTcpServer::newConnection, this, [this] {
            while (hasPendingConnections()) {
                auto *socket = nextPendingConnection();
                sockets.append(socket);
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
                connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
                    if (socket->property("answered").toBool()) {
                        socket->readAll();
                        return;
                    }
                    QByteArray request = socket->property("request").toByteArray() + socket->readAll();
                    socket->setProperty("request", request);
                    if (!request.contains("\r\n\r\n"))
                        return;
                    socket->setProperty("answered", true);
                    if (mode == Silent)
                        return;
                    if (mode == Drop) {
                        socket->abort();
                        return;
                    }
                    if (mode == Partial || mode == Trickle) {
                        socket->write("HTTP/1.1 200 OK\r\nContent-Length: 100000\r\n\r\n{");
                        ++chunks;
                        if (mode == Trickle) {
                            auto *timer = new QTimer(socket);
                            connect(timer, &QTimer::timeout, socket, [this, socket] {
                                socket->write(" ");
                                ++chunks;
                            });
                            timer->start(20);
                        }
                        return;
                    }
                    const QByteArray status = mode == HttpError ? "503 Unavailable" : "200 OK";
                    socket->write("HTTP/1.1 " + status + "\r\nContent-Type: application/json\r\nContent-Length: "
                                  + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
                    socket->disconnectFromHost();
                });
            }
        });
    }
};

class AssistantBackendTest final : public QObject {
    Q_OBJECT
    QTemporaryDir isolated;
    QByteArray previousPath;

    static void attach(LocalNetwork &network, HttpPeer &peer)
    {
        QVERIFY(peer.listen(QHostAddress::LocalHost));
        network.setProxy(QNetworkProxy::NoProxy);
        network.endpoint = QUrl(QStringLiteral("http://127.0.0.1:%1/request").arg(peer.serverPort()));
    }

private slots:
    void initTestCase()
    {
        QVERIFY(isolated.isValid());
        QCoreApplication::setOrganizationName("LyraAssistantTests");
        QCoreApplication::setApplicationName("Assistant");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, isolated.path());
        previousPath = qgetenv("PATH");
        QFile tool(isolated.filePath("secret-tool"));
        QVERIFY(tool.open(QIODevice::WriteOnly));
        tool.write("#!/bin/sh\nprintf 'fake-test-key\\n'\n");
        tool.close();
        QVERIFY(tool.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        qputenv("PATH", isolated.path().toUtf8());
    }
    void cleanupTestCase() { qputenv("PATH", previousPath); }
    void init() { QSettings().clear(); }

    void response_data()
    {
        QTest::addColumn<QString>("provider");
        QTest::addColumn<QByteArray>("body");
        QTest::newRow("openai") << QString("openai") << QByteArray(R"({"choices":[{"message":{"content":"resposta"}}]})");
        QTest::newRow("anthropic") << QString("anthropic") << QByteArray(R"({"content":[{"text":"resposta"}]})");
        QTest::newRow("gemini") << QString("gemini") << QByteArray(R"({"candidates":[{"content":{"parts":[{"text":"resposta"}]}}]})");
    }
    void response()
    {
        QFETCH(QString, provider);
        QFETCH(QByteArray, body);
        QSettings().setValue("assistant/provider", provider);
        HttpPeer peer;
        peer.body = body;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        backend.sendMessage("olá");
        QVERIFY(backend.busy());
        QPointer<QNetworkReply> reply = network.lastReply;
        QPointer<QTimer> timer = backend.m_requestTimer;
        QTRY_VERIFY(!backend.busy());
        QCOMPARE(backend.messages().size(), 2);
        QCOMPARE(backend.messages().last().toMap().value("content").toString(), "resposta");
        QVERIFY(backend.status().isEmpty());
        QTRY_VERIFY(reply.isNull());
        QVERIFY(timer.isNull());
    }

    void deadline_data()
    {
        QTest::addColumn<int>("mode");
        QTest::newRow("no-response") << int(HttpPeer::Silent);
        QTest::newRow("partial-response") << int(HttpPeer::Partial);
        QTest::newRow("continuous-trickle") << int(HttpPeer::Trickle);
    }
    void deadline()
    {
        QFETCH(int, mode);
        HttpPeer peer;
        peer.mode = HttpPeer::Mode(mode);
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 300, nullptr);
        QElapsedTimer elapsed;
        elapsed.start();
        backend.sendMessage("olá");
        QPointer<QNetworkReply> reply = network.lastReply;
        QPointer<QTimer> timer = backend.m_requestTimer;
        QTRY_VERIFY_WITH_TIMEOUT(!backend.busy(), 2000);
        QVERIFY(elapsed.elapsed() >= 300);
        QVERIFY(elapsed.elapsed() < 2000);
        QVERIFY(backend.status().contains("prazo"));
        QVERIFY(!backend.status().contains("fake-test-key"));
        QCOMPARE(backend.messages().size(), 1);
        if (mode != HttpPeer::Silent)
            QVERIFY(peer.chunks > 0);
        if (mode == HttpPeer::Trickle)
            QVERIFY(peer.chunks > 2);
        QTRY_VERIFY(reply.isNull());
        QVERIFY(timer.isNull());
        // A late old reply/timer cannot erase a subsequent successful answer.
        peer.mode = HttpPeer::Complete;
        backend.sendMessage("novamente");
        QTRY_VERIFY(!backend.busy());
        QCOMPARE(backend.messages().size(), 3);
        QVERIFY(backend.status().isEmpty());
    }

    void cancelAndRestart()
    {
        HttpPeer peer;
        peer.mode = HttpPeer::Partial;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        backend.sendMessage("olá");
        QTRY_VERIFY(peer.chunks > 0);
        QPointer<QNetworkReply> oldReply = network.lastReply;
        QPointer<QTimer> oldTimer = backend.m_requestTimer;
        QSignalSpy changes(&backend, &AssistantBackend::messagesChanged);
        backend.cancelRequest();
        QVERIFY(!backend.busy());
        QVERIFY(backend.status().contains("cancelada"));
        QCOMPARE(changes.count(), 1);
        QVERIFY(!oldTimer->isActive());
        peer.mode = HttpPeer::Complete;
        backend.sendMessage("nova");
        QVERIFY(backend.busy());
        QVERIFY(QMetaObject::invokeMethod(oldReply, "finished", Qt::DirectConnection));
        QVERIFY(backend.busy());
        QTRY_VERIFY(!backend.busy());
        QVERIFY(backend.status().isEmpty());
        QTRY_VERIFY(oldReply.isNull());
        QVERIFY(oldTimer.isNull());
    }

    void cancellationFromBusyNotification()
    {
        HttpPeer peer;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        connect(&backend, &AssistantBackend::messagesChanged, &backend, [&backend] {
            if (backend.busy())
                backend.cancelRequest();
        });
        backend.sendMessage("olá");
        QVERIFY(!backend.busy());
        QVERIFY(backend.status().contains("cancelada"));
    }

    void clearWhileBusy()
    {
        HttpPeer peer;
        peer.mode = HttpPeer::Partial;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        backend.sendMessage("olá");
        QTRY_VERIFY(peer.chunks > 0);
        QPointer<QNetworkReply> reply = network.lastReply;
        backend.clearConversation();
        QVERIFY(!backend.busy());
        QVERIFY(backend.messages().isEmpty());
        QVERIFY(backend.status().isEmpty());
        backend.cancelRequest();
        QTRY_VERIFY(reply.isNull());
        QVERIFY(backend.status().isEmpty());
    }

    void failure_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<QByteArray>("body");
        QTest::newRow("http-error") << int(HttpPeer::HttpError) << QByteArray(R"({"error":{"message":"indisponível"}})");
        QTest::newRow("invalid-json") << int(HttpPeer::Complete) << QByteArray("invalid");
        QTest::newRow("empty-answer") << int(HttpPeer::Complete) << QByteArray("{}");
        QTest::newRow("connection-lost") << int(HttpPeer::Drop) << QByteArray();
    }
    void failure()
    {
        QFETCH(int, mode);
        QFETCH(QByteArray, body);
        HttpPeer peer;
        peer.mode = HttpPeer::Mode(mode);
        peer.body = body;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        backend.sendMessage("olá");
        QPointer<QNetworkReply> reply = network.lastReply;
        QTRY_VERIFY(!backend.busy());
        QVERIFY(backend.status().startsWith("Falha no provedor:"));
        QCOMPARE(backend.messages().size(), 1);
        QTRY_VERIFY(reply.isNull());
    }

    void configurationCannotChangeInFlight()
    {
        HttpPeer peer;
        peer.mode = HttpPeer::Partial;
        LocalNetwork network;
        attach(network, peer);
        AssistantBackend backend(&network, 1000, nullptr);
        backend.sendMessage("olá");
        backend.configure("gemini", "different", "fake-key");
        QCOMPARE(backend.provider(), "openai");
        QCOMPARE(backend.model(), "gpt-4.1-mini");
        backend.sendMessage("must not start another request");
        QCOMPARE(network.requests, 1);
        backend.cancelRequest();
    }

    void destructionAbortsPendingReply()
    {
        HttpPeer peer;
        peer.mode = HttpPeer::Partial;
        LocalNetwork network;
        attach(network, peer);
        auto *backend = new AssistantBackend(&network, 1000, nullptr);
        backend->sendMessage("olá");
        QTRY_VERIFY(peer.chunks > 0);
        QPointer<QNetworkReply> reply = network.lastReply;
        QSignalSpy finished(reply, &QNetworkReply::finished);
        delete backend;
        QCOMPARE(finished.count(), 1);
        QTRY_VERIFY(reply.isNull());
    }
};

QTEST_GUILESS_MAIN(AssistantBackendTest)
#include "assistant_test.moc"
