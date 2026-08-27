#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QVariantList>

class AssistantBackend final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString provider READ provider NOTIFY configurationChanged)
    Q_PROPERTY(QString model READ model NOTIFY configurationChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY configurationChanged)
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY messagesChanged)
    Q_PROPERTY(QString status READ status NOTIFY messagesChanged)

public:
    explicit AssistantBackend(QObject *parent = nullptr);
    QString provider() const { return m_provider; }
    QString model() const { return m_model; }
    bool configured() const { return m_configured; }
    QVariantList messages() const { return m_messages; }
    bool busy() const { return m_busy; }
    QString status() const { return m_status; }

    Q_INVOKABLE void configure(const QString &provider, const QString &model, const QString &apiKey);
    Q_INVOKABLE void sendMessage(const QString &text);
    Q_INVOKABLE void clearConversation();

signals:
    void configurationChanged();
    void messagesChanged();

private:
    QString loadApiKey(const QString &provider) const;
    bool storeApiKey(const QString &provider, const QString &apiKey) const;
    QJsonObject requestBody() const;
    QString responseText(const QJsonObject &root) const;
    void finishWithError(const QString &message);

    QNetworkAccessManager m_network;
    QString m_provider;
    QString m_model;
    bool m_configured = false;
    QVariantList m_messages;
    bool m_busy = false;
    QString m_status;
};
