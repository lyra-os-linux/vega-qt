#include "systembackend.h"
#include "assistantbackend.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("LyraOS"));
    QGuiApplication::setApplicationName(QStringLiteral("Vega Qt"));
    QGuiApplication::setDesktopFileName(QStringLiteral("org.lyraos.Vega.Qt"));
    SystemBackend backend;
    const bool forceDark = app.arguments().contains(QStringLiteral("--dark"));
    const bool savedDark = QSettings().value(QStringLiteral("appearance/darkMode"), false).toBool();
    backend.setDarkTheme(forceDark || savedDark);
    AssistantBackend assistant;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("systemBackend"), &backend);
    engine.rootContext()->setContextProperty(QStringLiteral("assistantBackend"), &assistant);
    engine.rootContext()->setContextProperty(QStringLiteral("forceDarkTheme"), forceDark);
    engine.loadFromModule(QStringLiteral("org.lyraos.vega.qt"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
