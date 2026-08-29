#include "systembackend.h"
#include "assistantbackend.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStyleHints>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("LyraOS"));
    QGuiApplication::setApplicationName(QStringLiteral("Vega Qt"));
    QGuiApplication::setDesktopFileName(QStringLiteral("org.lyraos.Vega.Qt"));
    SystemBackend backend;
    const bool forceDark = app.arguments().contains(QStringLiteral("--dark"));
    const bool openSoftwareUpdates = app.arguments().contains(QStringLiteral("--software-updates"));
    const bool systemDark = app.styleHints()
        && app.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    const bool initialDark = forceDark || systemDark;
    backend.setDarkTheme(initialDark);
    AssistantBackend assistant;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("systemBackend"), &backend);
    engine.rootContext()->setContextProperty(QStringLiteral("assistantBackend"), &assistant);
    engine.rootContext()->setContextProperty(QStringLiteral("forceDarkTheme"), forceDark);
    engine.rootContext()->setContextProperty(QStringLiteral("initialDarkTheme"), initialDark);
    engine.rootContext()->setContextProperty(QStringLiteral("initialPage"), openSoftwareUpdates ? 1 : 0);
    engine.loadFromModule(QStringLiteral("org.lyraos.vega.qt"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return 1;
    QObject *root = engine.rootObjects().constFirst();
    if (!forceDark && app.styleHints()) {
        QObject::connect(app.styleHints(), &QStyleHints::colorSchemeChanged, root,
                         [root, &backend](Qt::ColorScheme scheme) {
            if (scheme == Qt::ColorScheme::Unknown)
                return;
            const bool dark = scheme == Qt::ColorScheme::Dark;
            root->setProperty("darkMode", dark);
            backend.setDarkTheme(dark);
        });
    }
    return app.exec();
}
