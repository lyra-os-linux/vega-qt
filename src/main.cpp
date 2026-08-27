#include "systembackend.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("LyraOS"));
    QGuiApplication::setApplicationName(QStringLiteral("Vega Qt"));
    QGuiApplication::setDesktopFileName(QStringLiteral("org.lyraos.Vega.Qt"));
    SystemBackend backend;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("systemBackend"), &backend);
    engine.loadFromModule(QStringLiteral("org.lyraos.vega.qt"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
