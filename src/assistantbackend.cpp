#include "assistantbackend.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QUrlQuery>

namespace {
constexpr int RequestTimeoutMs = 30'000;
const auto SystemPrompt = "Você é o Assistente do Lyra Vega, um centro de controle Linux. "
                          "Responda no idioma do usuário, seja conciso e seguro. Não afirme "
                          "ter alterado o sistema e nunca solicite senhas ou chaves.";
}

AssistantBackend::AssistantBackend(QObject *parent)
    : AssistantBackend(nullptr, RequestTimeoutMs, parent)
{
}

AssistantBackend::AssistantBackend(QNetworkAccessManager *network, int timeoutMs, QObject *parent)
    : QObject(parent), m_network(network ? network : new QNetworkAccessManager(this)),
      m_requestTimeoutMs(timeoutMs)
{
    QSettings settings;
    m_provider = settings.value(QStringLiteral("assistant/provider"), QStringLiteral("openai")).toString();
    m_model = settings.value(QStringLiteral("assistant/model"), QStringLiteral("gpt-4.1-mini")).toString();
    QTimer::singleShot(0, this, &AssistantBackend::checkConfiguredAsync);
}

AssistantBackend::~AssistantBackend()
{
    if (m_reply) {
        disconnect(m_reply, nullptr, this, nullptr);
        if (m_requestTimer)
            m_requestTimer->stop();
        m_reply->abort();
        m_reply->deleteLater();
    }
}

void AssistantBackend::checkConfiguredAsync()
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
    if (executable.isEmpty())
        return;

    auto *process = new QProcess(this);
    connect(process, &QProcess::finished, this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        const bool configured = exitStatus == QProcess::NormalExit && exitCode == 0
            && !QString::fromUtf8(process->readAllStandardOutput()).trimmed().isEmpty();
        if (m_configured != configured) {
            m_configured = configured;
            emit configurationChanged();
        }
        process->deleteLater();
    });
    process->start(executable, {QStringLiteral("lookup"), QStringLiteral("application"),
                                QStringLiteral("vega-qt"), QStringLiteral("provider"), m_provider});
    QTimer::singleShot(3000, process, [process] {
        if (process->state() != QProcess::NotRunning)
            process->kill();
    });
}

QString AssistantBackend::loadApiKey(const QString &provider) const
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
    if (executable.isEmpty())
        return {};
    QProcess process;
    process.start(executable, {QStringLiteral("lookup"), QStringLiteral("application"),
                               QStringLiteral("vega-qt"), QStringLiteral("provider"), provider});
    if (!process.waitForFinished(3000) || process.exitCode() != 0)
        return {};
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

bool AssistantBackend::storeApiKey(const QString &provider, const QString &apiKey) const
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
    if (executable.isEmpty())
        return false;
    QProcess process;
    process.start(executable, {QStringLiteral("store"), QStringLiteral("--label=Vega Qt — Assistente de IA"),
                               QStringLiteral("application"), QStringLiteral("vega-qt"),
                               QStringLiteral("provider"), provider});
    if (!process.waitForStarted(3000))
        return false;
    process.write(apiKey.toUtf8());
    process.closeWriteChannel();
    return process.waitForFinished(5000) && process.exitCode() == 0;
}

void AssistantBackend::configure(const QString &provider, const QString &model, const QString &apiKey)
{
    if (m_busy)
        return;
    const QString normalizedProvider = provider.trimmed().toLower();
    const QString normalizedModel = model.trimmed();
    if (normalizedModel.isEmpty() || (apiKey.trimmed().isEmpty() && loadApiKey(normalizedProvider).isEmpty())) {
        m_status = tr("Informe o modelo e a chave de API.");
        emit messagesChanged();
        return;
    }
    if (!apiKey.trimmed().isEmpty() && !storeApiKey(normalizedProvider, apiKey.trimmed())) {
        m_status = tr("Não foi possível salvar a chave no Secret Service.");
        emit messagesChanged();
        return;
    }
    m_provider = normalizedProvider;
    m_model = normalizedModel;
    m_configured = true;
    QSettings settings;
    settings.setValue(QStringLiteral("assistant/provider"), m_provider);
    settings.setValue(QStringLiteral("assistant/model"), m_model);
    m_status.clear();
    emit configurationChanged();
    emit messagesChanged();
}

QJsonObject AssistantBackend::requestBody() const
{
    if (m_provider == QStringLiteral("anthropic")) {
        QJsonArray messages;
        for (const QVariant &entry : m_messages) {
            const QVariantMap item = entry.toMap();
            messages.append(QJsonObject{{"role", item.value("role").toString()},
                                        {"content", item.value("content").toString()}});
        }
        return {{"model", m_model}, {"max_tokens", 1200}, {"system", SystemPrompt}, {"messages", messages}};
    }
    if (m_provider == QStringLiteral("gemini")) {
        QJsonArray contents;
        contents.append(QJsonObject{{"role", "user"},
            {"parts", QJsonArray{QJsonObject{{"text", SystemPrompt}}}}});
        for (const QVariant &entry : m_messages) {
            const QVariantMap item = entry.toMap();
            contents.append(QJsonObject{{"role", item.value("role") == "assistant" ? "model" : "user"},
                {"parts", QJsonArray{QJsonObject{{"text", item.value("content").toString()}}}}});
        }
        return {{"contents", contents}, {"generationConfig", QJsonObject{{"maxOutputTokens", 1200}}}};
    }
    QJsonArray messages{QJsonObject{{"role", "system"}, {"content", SystemPrompt}}};
    for (const QVariant &entry : m_messages) {
        const QVariantMap item = entry.toMap();
        messages.append(QJsonObject{{"role", item.value("role").toString()},
                                    {"content", item.value("content").toString()}});
    }
    return {{"model", m_model}, {"messages", messages}, {"max_tokens", 1200}};
}

QString AssistantBackend::responseText(const QJsonObject &root, const QString &provider) const
{
    if (provider == QStringLiteral("anthropic"))
        return root.value("content").toArray().at(0).toObject().value("text").toString();
    if (provider == QStringLiteral("gemini"))
        return root.value("candidates").toArray().at(0).toObject().value("content").toObject()
            .value("parts").toArray().at(0).toObject().value("text").toString();
    return root.value("choices").toArray().at(0).toObject().value("message").toObject().value("content").toString();
}

void AssistantBackend::sendMessage(const QString &text)
{
    const QString prompt = text.trimmed();
    if (m_busy || prompt.isEmpty())
        return;
    const QString apiKey = loadApiKey(m_provider);
    if (apiKey.isEmpty()) {
        m_configured = false;
        m_status = tr("Configure uma chave de API para continuar.");
        emit configurationChanged(); emit messagesChanged();
        return;
    }
    m_messages.append(QVariantMap{{"role", "user"}, {"content", prompt}});
    m_busy = true; m_status = tr("Pensando…");

    QUrl url;
    QNetworkRequest request;
    if (m_provider == QStringLiteral("anthropic")) {
        url = QUrl(QStringLiteral("https://api.anthropic.com/v1/messages"));
        request.setRawHeader("x-api-key", apiKey.toUtf8());
        request.setRawHeader("anthropic-version", "2023-06-01");
    } else if (m_provider == QStringLiteral("gemini")) {
        url = QUrl(QStringLiteral("https://generativelanguage.googleapis.com/v1beta/models/%1:generateContent").arg(m_model));
        QUrlQuery query; query.addQueryItem(QStringLiteral("key"), apiKey); url.setQuery(query);
    } else {
        url = QUrl(QStringLiteral("https://api.openai.com/v1/chat/completions"));
        request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    }
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    // The independent precise timer is an absolute budget, including connection,
    // upload and the complete response. Incoming trickles never reset it.
    QNetworkReply *reply = m_network->post(request, QJsonDocument(requestBody()).toJson(QJsonDocument::Compact));
    m_reply = reply;
    auto *timer = new QTimer(reply);
    timer->setSingleShot(true);
    timer->setTimerType(Qt::PreciseTimer);
    m_requestTimer = timer;
    connect(timer, &QTimer::timeout, this, [this, reply] {
        completeRequest(reply, Completion::Timeout);
    });
    const QString provider = m_provider;
    connect(reply, &QNetworkReply::finished, this, [this, reply, provider] {
        completeRequest(reply, Completion::Response, provider);
    });
    timer->start(m_requestTimeoutMs);
    // Publish busy only once cancellation is fully wired, including direct signal handlers.
    emit messagesChanged();
}

void AssistantBackend::completeRequest(QNetworkReply *reply, Completion completion, const QString &provider)
{
    if (m_reply != reply)
        return;
    // Detach the old request before abort(), which may synchronously emit finished.
    // All state is finalized before messagesChanged permits a new request.
    disconnect(reply, nullptr, this, nullptr);
    if (m_requestTimer)
        m_requestTimer->stop();
    m_requestTimer = nullptr;
    m_reply = nullptr;
    m_busy = false;

    switch (completion) {
    case Completion::Timeout:
        m_status = tr("O provedor não concluiu a resposta no prazo. Tente novamente.");
        break;
    case Completion::Cancelled:
        m_status = tr("Requisição cancelada.");
        break;
    case Completion::Cleared:
        m_messages.clear();
        m_status.clear();
        break;
    case Completion::Response: {
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        if (reply->error() != QNetworkReply::NoError) {
            const QString apiMessage = root.value("error").toObject().value("message").toString();
            m_status = tr("Falha no provedor: %1").arg(apiMessage.isEmpty() ? reply->errorString() : apiMessage);
        } else {
            const QString answer = responseText(root, provider).trimmed();
            if (answer.isEmpty())
                m_status = tr("Falha no provedor: %1").arg(tr("O provedor retornou uma resposta vazia."));
            else {
                m_messages.append(QVariantMap{{"role", "assistant"}, {"content", answer}});
                m_status.clear();
            }
        }
        break;
    }
    }
    if (!reply->isFinished())
        reply->abort();
    reply->deleteLater();
    emit messagesChanged();
}

void AssistantBackend::cancelRequest()
{
    if (m_reply)
        completeRequest(m_reply, Completion::Cancelled);
}

void AssistantBackend::clearConversation()
{
    if (m_reply) {
        completeRequest(m_reply, Completion::Cleared);
        return;
    }
    m_messages.clear();
    m_status.clear();
    emit messagesChanged();
}
