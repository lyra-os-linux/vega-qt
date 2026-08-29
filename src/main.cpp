#include "systembackend.h"
#include "assistantbackend.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
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
    QSettings settings;
    const bool systemDark = app.styleHints()
        && app.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    const bool initialDark = forceDark
        || (settings.contains(QStringLiteral("appearance/darkMode"))
                ? settings.value(QStringLiteral("appearance/darkMode")).toBool()
                : systemDark);
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
    return app.exec();
}
